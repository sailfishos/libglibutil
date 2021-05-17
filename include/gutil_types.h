/*
 * Copyright (C) 2014-2021 Jolla Ltd.
 * Copyright (C) 2014-2021 Slava Monich <slava.monich@jolla.com>
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

#ifndef GUTIL_TYPES_H
#define GUTIL_TYPES_H

#include <glib.h>
#include <string.h>
#include <stdio.h>

G_BEGIN_DECLS

typedef char* GStrV;
typedef struct gutil_idle_pool GUtilIdlePool;
typedef struct gutil_idle_queue GUtilIdleQueue;
typedef struct gutil_ints GUtilInts;
typedef struct gutil_int_array GUtilIntArray;
typedef struct gutil_int_history GUtilIntHistory;
typedef struct gutil_inotify_watch GUtilInotifyWatch;
typedef struct gutil_ring GUtilRing;
typedef struct gutil_time_notify GUtilTimeNotify;

typedef struct gutil_data {
    const guint8* bytes;
    gsize size;
} GUtilData;

typedef struct gutil_range {
    const guint8* ptr;
    const guint8* end;
} GUtilRange; /* Since 1.0.54 */

#define GLOG_MODULE_DECL(m) extern GLogModule m;
typedef struct glog_module GLogModule;

G_END_DECLS

#endif /* GUTIL_TYPES_H */

/*
 * Local Variables:
 * mode: C
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
