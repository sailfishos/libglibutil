/*
 * Copyright (C) 2016-2023 Slava Monich <slava@monich.com>
 * Copyright (C) 2016-2019 Jolla Ltd.
 *
 * You may use this file under the terms of BSD license as follows:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   1. Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *   3. Neither the names of the copyright holders nor the names of its
 *      contributors may be used to endorse or promote products derived
 *      from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "gutil_timenotify.h"
#include "gutil_log.h"

#include <glib-object.h>

#include <time.h>
#include <errno.h>
#include <string.h>
#include <sys/timerfd.h>

#ifndef TFD_TIMER_CANCEL_ON_SET
#  define TFD_TIMER_CANCEL_ON_SET (1 << 1)
#endif

#define TIME_T_MAX (time_t)((1UL << ((sizeof(time_t)*8)-1)) - 1)

struct gutil_time_notify {
    GObject object;
    GIOChannel* io_channel;
    guint io_watch_id;
};

enum gutil_time_notify_signal {
    SIGNAL_TIME_CHANGED,
    SIGNAL_COUNT
};

#define SIGNAL_TIME_CHANGED_NAME   "time-changed"

static guint gutil_time_notify_signals[SIGNAL_COUNT] = { 0 };

#define PARENT_CLASS gutil_time_notify_parent_class
#define THIS_TYPE gutil_time_notify_get_type()
#define THIS(obj) G_TYPE_CHECK_INSTANCE_CAST(obj, THIS_TYPE, GUtilTimeNotify)

GType THIS_TYPE G_GNUC_INTERNAL;
typedef GObjectClass GUtilTimeNotifyClass;
G_DEFINE_TYPE(GUtilTimeNotify, gutil_time_notify, G_TYPE_OBJECT)

GUtilTimeNotify*
gutil_time_notify_new()
{
    /* There's no need to have more than one instance. */
    static GUtilTimeNotify* gutil_time_notify_instance = NULL;

    if (gutil_time_notify_instance) {
        gutil_time_notify_ref(gutil_time_notify_instance);
    } else {
        gutil_time_notify_instance = g_object_new(THIS_TYPE, 0);
        g_object_add_weak_pointer(G_OBJECT(gutil_time_notify_instance),
            (gpointer*)(&gutil_time_notify_instance));
    }
    return gutil_time_notify_instance;
}

GUtilTimeNotify*
gutil_time_notify_ref(
    GUtilTimeNotify* self)
{
    if (G_LIKELY(self)) {
        g_object_ref(THIS(self));
    }
    return self;
}

void
gutil_time_notify_unref(
    GUtilTimeNotify* self)
{
    if (G_LIKELY(self)) {
        g_object_unref(THIS(self));
    }
}

gulong
gutil_time_notify_add_handler(
    GUtilTimeNotify* self,
    GUtilTimeNotifyFunc fn,
    void* arg)
{
    return (G_LIKELY(self) && G_LIKELY(fn)) ? g_signal_connect(self,
        SIGNAL_TIME_CHANGED_NAME, G_CALLBACK(fn), arg) : 0;
}

void
gutil_time_notify_remove_handler(
    GUtilTimeNotify* self,
    gulong id)
{
    if (G_LIKELY(self) && G_LIKELY(id)) {
        g_signal_handler_disconnect(self, id);
    }
}

static
gboolean
gutil_time_notify_callback(
    GIOChannel* channel,
    GIOCondition condition,
    gpointer user_data)
{
    GUtilTimeNotify* self = THIS(user_data);

    if (condition & (G_IO_NVAL | G_IO_ERR | G_IO_HUP)) {
        self->io_watch_id = 0;
        return G_SOURCE_REMOVE;
    } else {
        gsize bytes_read = 0;
        GError* error = NULL;
        guint64 exp;

        gutil_time_notify_ref(self);
        g_io_channel_read_chars(self->io_channel, (void*)&exp, sizeof(exp),
            &bytes_read, &error);
        if (error) {
            /* ECANCELED is expected */
            GDEBUG("%s", error->message);
            g_error_free(error);
        }
        g_signal_emit(self, gutil_time_notify_signals[SIGNAL_TIME_CHANGED], 0);
        gutil_time_notify_unref(self);
        return G_SOURCE_CONTINUE;
    }
}

static
void
gutil_time_notify_init(
    GUtilTimeNotify* self)
{
    const int fd = timerfd_create(CLOCK_REALTIME, 0);

    if (fd >= 0) {
        struct itimerspec timer;

        self->io_channel = g_io_channel_unix_new(fd);
        g_io_channel_set_close_on_unref(self->io_channel, TRUE);
        g_io_channel_set_encoding(self->io_channel, NULL, NULL);
        g_io_channel_set_buffered(self->io_channel, FALSE);
        self->io_watch_id = g_io_add_watch(self->io_channel,
            G_IO_IN | G_IO_HUP | G_IO_NVAL | G_IO_ERR,
            gutil_time_notify_callback, self);

        /* Set timeout to the latest possible value */
        memset(&timer, 0, sizeof(timer));
        timer.it_value.tv_sec = TIME_T_MAX;
        timer.it_interval.tv_sec = TIME_T_MAX;
        if (timerfd_settime(fd, TFD_TIMER_ABSTIME | TFD_TIMER_CANCEL_ON_SET,
            &timer, NULL) < 0) {
            GERR("timerfd settime: %s", strerror(errno));
        }
    } else {
        GERR("timerfd open: %s", strerror(errno));
    }
}

static
void
gutil_time_notify_finalize(
    GObject* object)
{
    GUtilTimeNotify* self = THIS(object);

    if (self->io_channel) {
        if (self->io_watch_id) {
            g_source_remove(self->io_watch_id);
        }
        g_io_channel_shutdown(self->io_channel, FALSE, NULL);
        g_io_channel_unref(self->io_channel);
    }
    G_OBJECT_CLASS(PARENT_CLASS)->finalize(object);
}

static
void
gutil_time_notify_class_init(
    GUtilTimeNotifyClass* klass)
{
    G_OBJECT_CLASS(klass)->finalize = gutil_time_notify_finalize;
    gutil_time_notify_signals[SIGNAL_TIME_CHANGED] =
        g_signal_new(SIGNAL_TIME_CHANGED_NAME, G_OBJECT_CLASS_TYPE(klass),
            G_SIGNAL_RUN_FIRST, 0, NULL, NULL, NULL, G_TYPE_NONE, 0);
}

/*
 * Local Variables:
 * mode: C
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
