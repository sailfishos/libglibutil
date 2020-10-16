/*
 * Copyright (C) 2016-2020 Jolla Ltd.
 * Copyright (C) 2016-2020 Slava Monich <slava.monich@jolla.com>
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

#include "gutil_ring.h"
#include "gutil_macros.h"
#include "gutil_log.h"

struct gutil_ring {
    gint ref_count;
    gint alloc;
    gint maxsiz;
    gint start;
    gint end;
    gpointer* data;
    GDestroyNotify free_func;
};

GUtilRing*
gutil_ring_new()
{
    return gutil_ring_sized_new(0, GUTIL_RING_UNLIMITED_SIZE);
}

GUtilRing*
gutil_ring_sized_new(
    gint reserved_size,
    gint max_size)
{
    return gutil_ring_new_full(reserved_size, max_size, NULL);
}

GUtilRing*
gutil_ring_new_full(
    gint reserved_size,
    gint max_size,
    GDestroyNotify free_func)
{
    GUtilRing* r = g_slice_new0(GUtilRing);
    r->ref_count = 1;
    r->start = r->end = -1;
    r->maxsiz = (max_size < 0) ? GUTIL_RING_UNLIMITED_SIZE : max_size;
    r->free_func = free_func;
    if (reserved_size) {
        r->data = g_new(gpointer, reserved_size);
        r->alloc = reserved_size;
    }
    return r;
}

GUtilRing*
gutil_ring_ref(
    GUtilRing* r)
{
    if (G_LIKELY(r)) {
        GASSERT(r->ref_count > 0);
        g_atomic_int_inc(&r->ref_count);
    }
    return r;
}

void
gutil_ring_unref(
    GUtilRing* r)
{
    if (G_LIKELY(r)) {
        GASSERT(r->ref_count > 0);
        if (g_atomic_int_dec_and_test(&r->ref_count)) {
            if (r->free_func) {
                const gint n = gutil_ring_size(r);
                gint i;
                for (i=0; i<n; i++) {
                    r->free_func(r->data[(r->start + i) % r->alloc]);
                }
            }
            g_free(r->data);
            gutil_slice_free(r);
        }
    }
}

void
gutil_ring_set_free_func(
    GUtilRing* r,
    GDestroyNotify free_func)
{
    if (G_LIKELY(r)) {
        r->free_func = free_func;
    }    
}

gint
gutil_ring_max_size(
    GUtilRing* r)
{
    return G_LIKELY(r) ? r->maxsiz : 0;
}

void
gutil_ring_set_max_size(
    GUtilRing* r,
    gint max_size)
{
    /* Normalize the value */
    if (max_size < 0) max_size = GUTIL_RING_UNLIMITED_SIZE;
    if (G_LIKELY(r) && r->maxsiz != max_size) {
        const gint size = gutil_ring_size(r);
        if (max_size >= 0 && size > max_size) {
            gutil_ring_drop(r, size - max_size);
        }
        r->maxsiz = max_size;
    }
}

gint
gutil_ring_size(
    GUtilRing* r)
{
    if (G_LIKELY(r) && r->start >= 0) {
        if (r->start > r->end) {
            return (r->alloc + r->end - r->start);
        } else if (r->end > r->start) {
            return (r->end - r->start);
        } else {
            GASSERT(r->start >= 0 && r->start < r->alloc);
            GASSERT(r->end >= 0 && r->end < r->alloc);
            return r->alloc;
        }
    }
    return 0;
}

void
gutil_ring_clear(
    GUtilRing* r)
{
    if (G_LIKELY(r)) {
        gint n = gutil_ring_size(r);
        if (n > 0) {
            GDestroyNotify free_func = r->free_func;
            if (free_func) {
                do {
                    free_func(gutil_ring_get(r));
                    n--;
                } while (n > 0 && gutil_ring_size(r) > 0);
            } else {
                r->start = r->end = -1;
            }
        }
    }
}

void
gutil_ring_compact(
    GUtilRing* r)
{
    if (G_LIKELY(r)) {
        int n = gutil_ring_size(r);
        if (r->alloc > n) {
            if (n > 0) {
                gpointer* buf = g_new(gpointer,n);
                if (r->start < r->end) {
                    memcpy(buf, r->data + r->start, sizeof(gpointer) * n);
                } else {
                    const int tail = r->alloc - r->start;
                    memcpy(buf, r->data + r->start, sizeof(gpointer) * tail);
                    memcpy(buf + tail, r->data, sizeof(gpointer) * r->end);
                }
                g_free(r->data);
                r->data = buf;
                r->alloc = n;
                r->start = 0;
                r->end = 0;
            } else {
                GASSERT(r->data);
                g_free(r->data);
                r->data = NULL;
                r->alloc = 0;
            }
        }
    }
}

gboolean
gutil_ring_reserve(
    GUtilRing* r,
    gint minsize)
{
    if (G_LIKELY(r)) {
        if (minsize <= r->alloc) {
            /* The buffer is already large enough */
            return TRUE;
        } else if (r->maxsiz >= 0 && r->alloc >= r->maxsiz) {
            /* Can't grow the buffer anymore */
            return FALSE;
        } else {
            gpointer* buf;
            /* At least double the allocation size */
            minsize = MAX(minsize, r->alloc*2);
            if (r->maxsiz > 0 && minsize > r->maxsiz) {
                /* Do not exceed the maximum size though */
                minsize = r->maxsiz;
            }
            buf = g_new(gpointer, minsize);
            if (r->start < r->end) {
                const int n = r->end - r->start;
                memcpy(buf, r->data + r->start, sizeof(gpointer) * n);
                r->start = 0;
                r->end = n;
            } else if (r->start >= 0) {
                const int tail = r->alloc - r->start;
                memcpy(buf, r->data + r->start, sizeof(gpointer) * tail);
                memcpy(buf + tail, r->data, sizeof(gpointer) * r->end);
                r->start = 0;
                r->end += tail;
                GASSERT(r->end < minsize);
            }
            g_free(r->data);
            r->data = buf;
            r->alloc = minsize;
            return TRUE;
        }
    }
    return FALSE;
}

gboolean
gutil_ring_can_put(
    GUtilRing* r,
    gint n)
{
    if (G_LIKELY(r)) {
        return r->maxsiz < 0 || (gutil_ring_size(r) + n) <= r->maxsiz;
    }
    return FALSE;
}

gboolean
gutil_ring_put(
    GUtilRing* r,
    gpointer data)
{
    if (gutil_ring_reserve(r, gutil_ring_size(r) + 1)) {
        if (r->start < 0) {
            r->start = r->end = 0;
        }
        GASSERT(r->end < r->alloc);
        r->data[r->end++] = data;
        r->end %= r->alloc;
        return TRUE;
    }
    return FALSE;
}

gboolean
gutil_ring_put_front(
    GUtilRing* r,
    gpointer data)
{
    if (gutil_ring_reserve(r, gutil_ring_size(r) + 1)) {
        if (r->start >= 0) {
            r->start = (r->start + r->alloc - 1) % r->alloc;
        } else {
            r->start = 0;
            r->end = 1;
        }
        r->data[r->start] = data;
        return TRUE;
    }
    return FALSE;
}

gpointer
gutil_ring_get(
    GUtilRing* r)
{
    if (G_LIKELY(r) && r->start >= 0) {
        gpointer data = r->data[r->start++];
        if (r->start == r->end) {
            r->start = r->end = -1;
        } else {
            r->start %= r->alloc;
            if (r->start == r->end) {
                r->start = r->end = -1;
            }
        }
        return data;
    }
    return NULL;
}

gpointer
gutil_ring_get_last(
    GUtilRing* r)
{
    if (G_LIKELY(r) && r->start >= 0) {
        gpointer data;
        r->end = (r->end + r->alloc - 1) % r->alloc;
        data = r->data[r->end];
        if (r->start == r->end) {
            r->start = r->end = -1;
        }
        return data;
    }
    return NULL;
}

gint
gutil_ring_drop(
    GUtilRing* r,
    gint n)
{
    int size, dropped = 0;
    if (n > 0 && (size = gutil_ring_size(r)) > 0) {
        if (n >= size) {
            dropped = size;
            gutil_ring_clear(r);
        } else {
            dropped = n;
            if (r->free_func) {
                while ((n--) > 0) {
                    r->free_func(r->data[r->start]);
                    r->start = (r->start + 1) % r->alloc;
                    GASSERT(r->start != r->end);
                }
            } else {
                r->start = (r->start + n) % r->alloc;
                GASSERT(r->start != r->end);
            }
        }
    }
    return dropped;
}

gint
gutil_ring_drop_last(
    GUtilRing* r,
    gint n)
{
    int size, dropped = 0;
    if (n > 0 && (size = gutil_ring_size(r)) > 0) {
        if (n >= size) {
            dropped = size;
            gutil_ring_clear(r);
        } else {
            dropped = n;
            if (r->free_func) {
                while ((n--) > 0) {
                    r->end = (r->end + r->alloc - 1) % r->alloc;
                    r->free_func(r->data[r->end]);
                    GASSERT(r->start != r->end);
                }
            } else {
                r->end = (r->end + r->alloc - n) % r->alloc;
                GASSERT(r->start != r->end);
            }
        }
    }
    return dropped;
}

gpointer
gutil_ring_data_at(
    GUtilRing* r,
    gint pos)
{
    if (pos >= 0 && pos < gutil_ring_size(r)) {
        return r->data[(r->start + pos) % r->alloc];
    }
    return NULL;

}

gpointer*
gutil_ring_flatten(
    GUtilRing* r,
    gint* size)
{
    gpointer* data = NULL;
    gint n = gutil_ring_size(r);
    if (G_LIKELY(r) && n > 0) {
        if (r->start > 0 && r->start >= r->end) {
            gpointer* buf = g_new(gpointer, n); 
            const gint n1 = r->alloc - r->start;
            memcpy(buf, r->data + r->start, sizeof(gpointer) * n1);
            memcpy(buf + n1, r->data, sizeof(gpointer) * r->end);
            g_free(r->data);
            r->data = buf;
            r->start = 0;
            r->end = (n % r->alloc);
        }
        data = r->data + r->start;
    }
    if (size) *size = n;
    return data;
}

/*
 * Local Variables:
 * mode: C
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
