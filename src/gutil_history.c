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

#include "gutil_history.h"
#include "gutil_log.h"

#define GUTIL_HISTORY_DEFAULT_TIME_FUNC g_get_monotonic_time

typedef struct gutil_int_history_entry {
    gint64 time;
    int value;
} GUtilIntHistoryEntry;

struct gutil_int_history {
    gint ref_count;
    GUtilHistoryTimeFunc time;
    gint64 max_interval;
    int first;                          /* Oldest position (inclusive) */
    int last;                           /* Latest position (inclusive) */
    int max_size;                       /* Number of entries */
    GUtilIntHistoryEntry entry[1];
};

GUtilIntHistory*
gutil_int_history_new(
   int max_size,
   gint64 max_interval)
{
    return gutil_int_history_new_full(max_size, max_interval, NULL);
}

GUtilIntHistory*
gutil_int_history_new_full(
    int max_size,
    gint64 max_interval,
    GUtilHistoryTimeFunc fn)
{
    if (max_size > 0 && max_interval > 0) {
        /* 
         * We don't allow to dynamically change the maximum history size
         * so we can allocate the whole thing from a single memory block.
         */
        GUtilIntHistory* h = g_malloc0(sizeof(GUtilIntHistory) +
            (max_size-1)*sizeof(GUtilIntHistoryEntry));
        h->ref_count = 1;
        h->max_size = max_size;
        h->max_interval = max_interval;
        h->first = h->last = -1;
        h->time = fn ? fn : GUTIL_HISTORY_DEFAULT_TIME_FUNC;
        return h;
    }
    return NULL;
}

GUtilIntHistory*
gutil_int_history_ref(
    GUtilIntHistory* h)
{
    if (G_LIKELY(h)) {
        GASSERT(h->ref_count > 0);
        g_atomic_int_inc(&h->ref_count);
    }
    return h;
}

void
gutil_int_history_unref(
    GUtilIntHistory* h)
{
    if (G_LIKELY(h)) {
        GASSERT(h->ref_count > 0);
        if (g_atomic_int_dec_and_test(&h->ref_count)) {
            g_free(h);
        }
    }
}

static
gboolean
gutil_int_history_flush(
    GUtilIntHistory* h,
    const gint64 now)
{
    const gint64 cutoff = now - h->max_interval;
    if (h->entry[h->last].time >= cutoff) {
        /* At least the last entry is valid */
        while (h->entry[h->first].time < cutoff) {
            h->first = (h->first + 1) % h->max_size;
        }
        return TRUE;
    } else {
        /* The last entry has expired */
        h->last = h->first = -1;
        return FALSE;
    }
}

guint
gutil_int_history_size(
    GUtilIntHistory* h)
{
    if (G_LIKELY(h) && h->last >= 0) {
        if (gutil_int_history_flush(h, h->time())) {
            if (h->first > h->last) {
                return (h->max_size + h->last - h->first + 1);
            } else {
                GASSERT(h->last > h->first);
                return (h->last - h->first + 1);
            }
        }
    }
    return 0;
}

gint64
gutil_int_history_interval(
    GUtilIntHistory* h)
{
    if (G_LIKELY(h) && h->last >= 0) {
	const gint64 now = h->time();
        if (gutil_int_history_flush(h, now)) {
            return now - h->entry[h->first].time;
        }
    }
    return 0;
}

void
gutil_int_history_clear(
    GUtilIntHistory* h)
{
    if (G_LIKELY(h)) {
        h->last = h->first = -1;
    }
}


static
int
gutil_int_history_median_at(
    GUtilIntHistory* h,
    const gint64 now)
{
    /* The caller has already checked that the history is not empty */
    if (h->first == h->last) {
        return h->entry[h->last].value;
    } else {
        int pos = h->first;
        int v = h->entry[pos].value;
        gint64 t = h->entry[pos].time;
        gint64 dt = 0;
        gint64 area = 0;
        while (pos != h->last) {
            gint64 t1;
            int v1;
            pos = (pos + 1) % h->max_size;
            t1 = h->entry[pos].time;
            v1 = h->entry[pos].value;
            dt += t1 - t;
            area += (t1 - t)*(v + v1)/2;
            t = t1;
            v = v1;
        }
        /* Integral area divided by time */
        return (int)(area/dt);
    }
}

int
gutil_int_history_add(
    GUtilIntHistory* h,
    int value)
{
    if (G_LIKELY(h)) {
	gint64 now = h->time();
        if (h->last < 0 || !gutil_int_history_flush(h, now)) {
            h->last = h->first = 0;
        } else {
            const gint64 last_time = h->entry[h->last].time;
            if (now > last_time) {
                /* Need a new entry */
                h->last = (h->last + 1) % h->max_size;
                if (h->last == h->first) {
                    h->first = (h->first + 1) % h->max_size;
                }
            } else if (now < last_time) {
                /* Time goes back? */
                now = last_time;
            }
        }
        h->entry[h->last].time = now;
        h->entry[h->last].value = value;
        return gutil_int_history_median_at(h, now);
    }
    return 0;
}

int
gutil_int_history_median(
    GUtilIntHistory* h,
    int default_value)
{
    if (G_LIKELY(h) && h->last >= 0) {
	const gint64 now = h->time();
        if (gutil_int_history_flush(h, now)) {
            return gutil_int_history_median_at(h, now);
        }
    }
    return default_value;
}

/*
 * Local Variables:
 * mode: C
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
