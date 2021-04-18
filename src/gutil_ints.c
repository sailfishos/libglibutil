/*
 * Copyright (C) 2017-2021 Jolla Ltd.
 * Copyright (C) 2017-2021 Slava Monich <slava.monich@jolla.com>
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

#include "gutil_intarray.h"
#include "gutil_ints.h"
#include "gutil_misc.h"
#include "gutil_macros.h"

struct gutil_ints {
    const int* data;
    guint count;
    gint ref_count;
    GDestroyNotify free_func;
    gpointer user_data;
};

GUtilInts*
gutil_ints_new(
    const int* data,
    guint n)
{
    if (data && n) {
        return gutil_ints_new_take(gutil_memdup(data, n * sizeof(int)), n);
    } else {
        return NULL;
    }
}

GUtilInts*
gutil_ints_new_take(
    int* data,
    guint count)
{
    return gutil_ints_new_with_free_func(data, count, g_free, data);
}

GUtilInts*
gutil_ints_new_static(
    const int* data,
    guint count)
{
    return gutil_ints_new_with_free_func(data, count, NULL, NULL);
}

GUtilInts*
gutil_ints_new_with_free_func(
    const int* data,
    guint count,
    GDestroyNotify free_func,
    gpointer user_data)
{
    if (data && count) {
        GUtilInts* ints = g_slice_new(GUtilInts);
        ints->data = data;
        ints->count = count;
        ints->free_func = free_func;
        ints->user_data = user_data;
        ints->ref_count = 1;
        return ints;
    } else {
        return NULL;
    }
}

static
void
gutil_ints_unref1(
    gpointer ints)
{
    gutil_ints_unref(ints);
}

GUtilInts*
gutil_ints_new_from_ints(
    GUtilInts* ints,
    guint offset,
    guint count)
{
    if (ints && offset < ints->count) {
        guint end = offset + count;
        if (end > ints->count) {
            end = ints->count;
        }
        return gutil_ints_new_with_free_func(ints->data + offset, end - offset,
            gutil_ints_unref1, gutil_ints_ref(ints));
    }
    return NULL;
}

GUtilInts*
gutil_ints_ref(
    GUtilInts* ints)
{
    if (ints) {
        g_atomic_int_inc(&ints->ref_count);
        return ints;
    }
    return NULL;
}

void
gutil_ints_unref(
    GUtilInts* ints)
{
    if (ints) {
        if (g_atomic_int_dec_and_test(&ints->ref_count)) {
            if (ints->free_func) {
                ints->free_func(ints->user_data);
            }
            gutil_slice_free(ints);
        }
    }
}

int*
gutil_ints_unref_to_data(
    GUtilInts* ints,
    guint* count)
{
    if (ints) {
        int* result;
        if (count) {
            *count = ints->count;
        }
        if (g_atomic_int_dec_and_test(&ints->ref_count)) {
            if (ints->free_func == g_free) {
                /* We can allow the caller to free the data */
                result = (int*)ints->data;
            } else {
                result = gutil_memdup(ints->data, ints->count * sizeof(int));
                ints->free_func(ints->user_data);
            }
            gutil_slice_free(ints);
        } else {
            result = gutil_memdup(ints->data, ints->count * sizeof(int));
        }
        return result;
    } else {
        if (count) {
            *count = 0;
        }
        return NULL;
    }
}

const int*
gutil_ints_get_data(
    GUtilInts* ints,
    guint* count)
{
    if (ints) {
        if (count) {
            *count = ints->count;
        }
        return ints->data;
    } else {
        if (count) {
            *count = 0;
        }
        return NULL;
    }
}

guint
gutil_ints_get_count(
    GUtilInts* ints)
{
    return ints ? ints->count : 0;
}

int
gutil_ints_find(
    GUtilInts* ints,
    int value)
{
    if (ints) {
        guint i;
        for (i=0; i<ints->count; i++) {
            if (ints->data[i] == value) {
                return i;
            }
        }
    }
    return -1;
}

gboolean
gutil_ints_contains(
    GUtilInts* ints,
    int value)
{
    /*
     * Could use gutil_ints_find() here but it would return negative value
     * if the array contains more than MAX_INT entries. Let's be a bit more
     * reliable.
     */
    if (ints) {
        guint i;
        for (i=0; i<ints->count; i++) {
            if (ints->data[i] == value) {
                return TRUE;
            }
        }
    }
    return FALSE;
}

guint
gutil_ints_hash(
    gconstpointer data)
{
    if (data) {
        const GUtilInts* ints = data;
        guint i, h = 1234;
        for (i=0; i<ints->count; i++) {
            h ^= ints->data[i] * (i+1);
        }
        return h;
    } else {
        return 0;
    }
}

gboolean
gutil_ints_equal(
    gconstpointer a,
    gconstpointer b)
{
    const GUtilInts* i1 = a;
    const GUtilInts* i2 = b;
    if (i1 == i2) {
        return TRUE;
    } else if (i1 && i2) {
        return i1->count == i2->count &&
            !memcmp(i1->data, i2->data, i1->count * sizeof(int));
    } else {
        return FALSE;
    }
}


/*
 * Returns: a negative value if b is greater,
 * a positive value if a is greater,
 * and zero if a is equals b
 */
gint
gutil_ints_compare(
    gconstpointer a,
    gconstpointer b)
{
    const GUtilInts* i1 = a;
    const GUtilInts* i2 = b;
    if (i1 == i2) {
        return 0;
    } else if (i1 && i2) {
        const int ret = memcmp(i1->data, i2->data,
            MIN(i1->count, i2->count) * sizeof(int));
        if (ret || i1->count == i2->count) {
            return ret;
        } else {
            return (i1->count > i2->count) ? 1 : -1;
        }
    } else {
        return i1 ? 1 : -1;
    }
}

/*
 * Local Variables:
 * mode: C
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
