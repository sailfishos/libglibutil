/*
 * Copyright (C) 2017 Jolla Ltd.
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

#ifndef GUTIL_HISTORY_H
#define GUTIL_HISTORY_H

#include "gutil_types.h"

G_BEGIN_DECLS

/*
 * A history of values. Keeps track of no more than last X values submitted
 * within the last Y microseconds. Microsecond precision may sound like a bit
 * of an overkill but that's what glib functions like g_get_monotonic_time
 * return.
 *
 * Currently it's only useful for calculating the median but the API could
 * be extended to provide access to the individual samples, if anyone ever
 * needs that.
 *
 * Also, it's calculating median rather than the average taking into account
 * that samples are not necessarily added to the history at uniform intervals.
 * If someone needs the straight average, it's easy enough to add API for that
 * too.
 *
 * When you call gutil_int_history_add but the time hasn't changed since the
 * last gutil_int_history_add call, the last value is replaced (as opposed to
 * adding a new entry).
 */

#define GUTIL_HISTORY_SEC ((gint64)(G_USEC_PER_SEC))
typedef gint64 (*GUtilHistoryTimeFunc)(void);

GUtilIntHistory*
gutil_int_history_new(
    int max_size,
    gint64 max_interval);

GUtilIntHistory*
gutil_int_history_new_full(
    int max_size,
    gint64 max_interval,
    GUtilHistoryTimeFunc time_fn);

GUtilIntHistory*
gutil_int_history_ref(
    GUtilIntHistory* history);

void
gutil_int_history_unref(
    GUtilIntHistory* history);

guint
gutil_int_history_size(
    GUtilIntHistory* history);

gint64
gutil_int_history_interval(
    GUtilIntHistory* history);

void
gutil_int_history_clear(
    GUtilIntHistory* history);

int
gutil_int_history_add(
    GUtilIntHistory* history,
    int value);

int
gutil_int_history_median(
    GUtilIntHistory* history,
    int default_value);

G_END_DECLS

#endif /* GUTIL_HISTORY_H */

/*
 * Local Variables:
 * mode: C
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
