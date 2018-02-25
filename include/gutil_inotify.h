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

#ifndef GUTIL_INOTIFY_H
#define GUTIL_INOTIFY_H

#include "gutil_types.h"

G_BEGIN_DECLS

typedef struct gutil_inotify_watch_callback GUtilInotifyWatchCallback;

typedef void
(*GUtilInotifyWatchFunc)(
    GUtilInotifyWatch* watch,
    guint mask,
    guint cookie,
    const char* name,
    void* arg);

GUtilInotifyWatch*
gutil_inotify_watch_new(
    const char* path,
    guint32 mask);

GUtilInotifyWatch*
gutil_inotify_watch_ref(
    GUtilInotifyWatch* watch);

void
gutil_inotify_watch_unref(
    GUtilInotifyWatch* watch);

void
gutil_inotify_watch_destroy(
    GUtilInotifyWatch* watch);

gulong
gutil_inotify_watch_add_handler(
    GUtilInotifyWatch* watch,
    GUtilInotifyWatchFunc fn,
    void* arg);

void
gutil_inotify_watch_remove_handler(
    GUtilInotifyWatch* watch,
    gulong id);

GUtilInotifyWatchCallback*
gutil_inotify_watch_callback_new(
    const char* path,
    guint32 mask,
    GUtilInotifyWatchFunc fn,
    void* arg);

void
gutil_inotify_watch_callback_free(
    GUtilInotifyWatchCallback* cb);

G_END_DECLS

#endif /* GUTIL_INOTIFY_H */

/*
 * Local Variables:
 * mode: C
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
