/*
 * Copyright (C) 2014-2018 Jolla Ltd.
 * Contact: Slava Monich <slava.monich@jolla.com>
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
 *   3. Neither the name of Jolla Ltd nor the names of its contributors may
 *      be used to endorse or promote products derived from this software
 *      without specific prior written permission.
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

#include <gutil_inotify.h>
#include <gutil_log.h>

#include <glib-object.h>

#include <sys/inotify.h>
#include <unistd.h>
#include <errno.h>

typedef struct gutil_inotify {
    int ref_count;
    int fd;
    GHashTable* watches;
    GIOChannel* io_channel;
    guint io_watch_id;
    gchar buf[sizeof(struct inotify_event) + NAME_MAX + 1];
} GUtilInotify;

typedef GObjectClass GUtilInotifyWatchClass;
typedef GUtilInotifyWatch GUtilInotifyWatch;

struct gutil_inotify_watch {
    GObject object;
    GUtilInotify* inotify;
    GUtilInotifyWatch* parent;
    guint32 mask;
    char* path;
    int wd;
};

struct gutil_inotify_watch_callback {
    GUtilInotifyWatch* watch;
    gulong id;
};

enum gutil_inotify_watch_signal {
    SIGNAL_WATCH_EVENT,
    SIGNAL_COUNT
};

#define SIGNAL_WATCH_EVENT_NAME        "gutil-inotify-watch-event"

static guint gutil_inotify_watch_signals[SIGNAL_COUNT] = { 0 };

G_DEFINE_TYPE(GUtilInotifyWatch, gutil_inotify_watch, G_TYPE_OBJECT)
#define GUTIL_INOTIFY_WATCH_TYPE (gutil_inotify_watch_get_type())
#define GUTIL_INOTIFY_WATCH(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj),\
        GUTIL_INOTIFY_WATCH_TYPE, GUtilInotifyWatch))

/*==========================================================================*
 * GUtilInotify
 *==========================================================================*/

static GUtilInotify* gutil_inotify_instance = NULL;

static
GUtilInotify*
gutil_inotify_ref(
    GUtilInotify* self)
{
    GASSERT(self->ref_count > 0);
    g_atomic_int_inc(&self->ref_count);
    return self;
}

static
void
gutil_inotify_unref(
    GUtilInotify* self)
{
    GASSERT(self->ref_count > 0);
    if (g_atomic_int_dec_and_test(&self->ref_count)) {
        if (self->io_watch_id) {
            g_source_remove(self->io_watch_id);
        }
        GASSERT(!g_hash_table_size(self->watches));
        g_hash_table_destroy(self->watches);
        g_io_channel_shutdown(self->io_channel, FALSE, NULL);
        g_io_channel_unref(self->io_channel);
        close(self->fd);
        g_free(self);
        if (gutil_inotify_instance == self) {
            gutil_inotify_instance = NULL;
        }
    }
}

static
gboolean
gutil_inotify_read(
    GUtilInotify* self)
{
    gsize nbytes = 0;
    GError* error = NULL;
    g_io_channel_read_chars(self->io_channel, self->buf, sizeof(self->buf),
        &nbytes, &error);
    if (error) {
        GERR("Inotify read failed: %s", error->message);
        g_error_free(error);
        return FALSE;
    } else {
        const char* next = self->buf;
        while (nbytes > 0) {
            const struct inotify_event* event = (void*)next;
            const size_t len = sizeof(struct inotify_event) + event->len;
            const char* path = event->len ? event->name : NULL;
            GDEBUG("Inotify event 0x%04x for %s", event->mask, path);
            GASSERT(len <= nbytes);
            if (len > nbytes) {
                break;
            } else  {
                GUtilInotifyWatch* watch = g_hash_table_lookup(self->watches,
                    GINT_TO_POINTER(event->wd));
                if (watch) {
                    gutil_inotify_watch_ref(watch);
                    g_signal_emit(watch,
                        gutil_inotify_watch_signals[SIGNAL_WATCH_EVENT], 0,
                            event->mask, event->cookie, path);
                    gutil_inotify_watch_unref(watch);
                }
                next += len;
                nbytes -= len;
            }
        }
        return TRUE;
    }
}

static
gboolean
gutil_inotify_callback(
    GIOChannel* channel,
    GIOCondition condition,
    gpointer user_data)
{
    GUtilInotify* self = user_data;
    if (condition & (G_IO_NVAL | G_IO_ERR | G_IO_HUP)) {
        self->io_watch_id = 0;
        return G_SOURCE_REMOVE;
    } else {
        gboolean ok;
        gutil_inotify_ref(self);
        ok = (condition & G_IO_IN) && gutil_inotify_read(self);
        if (!ok) self->io_watch_id = 0;
        gutil_inotify_unref(self);
        return ok;
    }
}

static
GUtilInotify*
gutil_inotify_create(
    int fd)
{
    GIOChannel* channel = g_io_channel_unix_new(fd);
    GASSERT(channel);
    if (channel) {
        GUtilInotify* inotify = g_new(GUtilInotify, 1);
        inotify->ref_count = 1;
        inotify->fd = fd;
        inotify->io_channel = channel;
        inotify->watches = g_hash_table_new_full(g_direct_hash,
            g_direct_equal, NULL, NULL);
        g_io_channel_set_encoding(channel, NULL, NULL);
        g_io_channel_set_buffered(channel, FALSE);
        inotify->io_watch_id = g_io_add_watch(channel,
            G_IO_IN | G_IO_HUP | G_IO_NVAL | G_IO_ERR,
            gutil_inotify_callback, inotify);
        return inotify;
    }
    return NULL;
}

static
GUtilInotify*
gutil_inotify_new()
{
    if (gutil_inotify_instance) {
        gutil_inotify_ref(gutil_inotify_instance);
    } else {
        int fd = inotify_init();
        if (fd >= 0) {
            /* GUtilInotify takes ownership of the file descriptor */
            gutil_inotify_instance = gutil_inotify_create(fd);
            if (!gutil_inotify_instance) {
                close(fd);
            }
        } else {
            GERR("Failed to init inotify: %s", strerror(errno));
        }
    }
    return gutil_inotify_instance;
}

static
void
gutil_inotify_add_watch(
    GUtilInotify* self,
    GUtilInotifyWatch* watch)
{
    GASSERT(watch->wd >= 0);
    g_hash_table_insert(self->watches, GINT_TO_POINTER(watch->wd), watch);
}

static
void
gutil_inotify_remove_watch(
    GUtilInotify* self,
    GUtilInotifyWatch* watch)
{
    GASSERT(watch->wd >= 0);
    GVERIFY(g_hash_table_remove(self->watches, GINT_TO_POINTER(watch->wd)));
}

/*==========================================================================*
 * GUtilInotifyWatch
 *==========================================================================*/

GUtilInotifyWatch*
gutil_inotify_watch_new(
    const char* path,
    guint32 mask)
{
    if (G_LIKELY(path)) {
        GUtilInotify* inotify = gutil_inotify_new();
        if (G_LIKELY(inotify)) {
            int wd = inotify_add_watch(inotify->fd, path, mask);
            if (wd >= 0) {
                GUtilInotifyWatch* self =
                    g_object_new(GUTIL_INOTIFY_WATCH_TYPE,0);
                self->inotify = inotify;
                self->wd = wd;
                self->mask = mask;
                self->path = g_strdup(path);
                gutil_inotify_add_watch(inotify, self);
                return self;
            } else if (errno == ENOENT) {
                GDEBUG("%s doesn't exist", path);
            } else {
                GERR("Failed to add inotify watch %s mask 0x%04x: %s",
                    path, mask, strerror(errno));
            }
            gutil_inotify_unref(inotify);
        }
    }
    return NULL;
}

static
void
gutil_inotify_watch_stop(
    GUtilInotifyWatch* self)
{
    if (self->wd >= 0) {
        gutil_inotify_remove_watch(self->inotify, self);
        inotify_rm_watch(self->inotify->fd, self->wd);
        self->wd = -1;
    }
}

void
gutil_inotify_watch_destroy(
    GUtilInotifyWatch* self)
{
    if (G_LIKELY(self)) {
        gutil_inotify_watch_stop(self);
        gutil_inotify_watch_unref(self);
    }
}

GUtilInotifyWatch*
gutil_inotify_watch_ref(
    GUtilInotifyWatch* self)
{
    if (G_LIKELY(self)) {
        g_object_ref(GUTIL_INOTIFY_WATCH(self));
        return self;
    } else {
        return NULL;
    }
}

void
gutil_inotify_watch_unref(
    GUtilInotifyWatch* self)
{
    if (G_LIKELY(self)) {
        g_object_unref(GUTIL_INOTIFY_WATCH(self));
    }
}

gulong
gutil_inotify_watch_add_handler(
    GUtilInotifyWatch* self,
    GUtilInotifyWatchFunc cb,
    void* arg)
{
    return (G_LIKELY(self) && G_LIKELY(cb)) ? g_signal_connect(self,
        SIGNAL_WATCH_EVENT_NAME, G_CALLBACK(cb), arg) : 0;
}

void
gutil_inotify_watch_remove_handler(
    GUtilInotifyWatch* self,
    gulong id)
{
    if (G_LIKELY(self) && G_LIKELY(id)) {
        g_signal_handler_disconnect(self, id);
    }
}

static
void
gutil_inotify_watch_init(
    GUtilInotifyWatch* self)
{
}

static
void
gutil_inotify_watch_dispose(
    GObject* object)
{
    GUtilInotifyWatch* self = GUTIL_INOTIFY_WATCH(object);
    gutil_inotify_watch_stop(self);
    G_OBJECT_CLASS(gutil_inotify_watch_parent_class)->dispose(object);
}

static
void
gutil_inotify_watch_finalize(
    GObject* object)
{
    GUtilInotifyWatch* self = GUTIL_INOTIFY_WATCH(object);
    gutil_inotify_unref(self->inotify);
    g_free(self->path);
    G_OBJECT_CLASS(gutil_inotify_watch_parent_class)->finalize(object);
}

static
void gutil_inotify_watch_class_init(
    GUtilInotifyWatchClass* klass)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);
    object_class->dispose = gutil_inotify_watch_dispose;
    object_class->finalize = gutil_inotify_watch_finalize;
    gutil_inotify_watch_signals[SIGNAL_WATCH_EVENT] =
        g_signal_new(SIGNAL_WATCH_EVENT_NAME, G_OBJECT_CLASS_TYPE(klass),
            G_SIGNAL_RUN_FIRST, 0, NULL, NULL, NULL,
            G_TYPE_NONE, 3, G_TYPE_UINT, G_TYPE_UINT, G_TYPE_POINTER);
}

/*==========================================================================*
 * GUtilInotifyWatchCallback
 *==========================================================================*/

GUtilInotifyWatchCallback*
gutil_inotify_watch_callback_new(
    const char* path,
    guint32 mask,
    GUtilInotifyWatchFunc fn,
    void* arg)
{
    GUtilInotifyWatch* watch = gutil_inotify_watch_new(path, mask);
    if (watch) {
        GUtilInotifyWatchCallback* cb = g_new(GUtilInotifyWatchCallback,1);
        cb->watch = watch;
        cb->id = gutil_inotify_watch_add_handler(watch, fn, arg);
        return cb;
    }
    return NULL;
}

void
gutil_inotify_watch_callback_free(
    GUtilInotifyWatchCallback* cb)
{
    if (G_LIKELY(cb)) {
        gutil_inotify_watch_remove_handler(cb->watch, cb->id);
        gutil_inotify_watch_unref(cb->watch);
        g_free(cb);
    }
}

/*
 * Local Variables:
 * mode: C
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
