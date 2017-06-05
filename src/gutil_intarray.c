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

#include "gutil_intarray.h"
#include "gutil_ints.h"

G_STATIC_ASSERT(sizeof(GUtilIntArray) == sizeof(GArray));
#define ELEMENT_SIZE (sizeof(int))

GUtilIntArray*
gutil_int_array_new()
{
    return (GUtilIntArray*)g_array_sized_new(FALSE, FALSE, ELEMENT_SIZE, 0);
}

GUtilIntArray*
gutil_int_array_sized_new(
    guint reserved)
{
    return (GUtilIntArray*)g_array_sized_new(FALSE, FALSE, ELEMENT_SIZE,
        reserved * ELEMENT_SIZE);
}

GUtilIntArray*
gutil_int_array_new_from_vals(
    const int* vals,
    guint count)
{
    GUtilIntArray* array = gutil_int_array_sized_new(count);
    gutil_int_array_append_vals(array, vals, count);
    return array;
}

GUtilIntArray*
gutil_int_array_new_from_value(
    int value)
{
    return gutil_int_array_new_from_vals(&value, 1);
}

int*
gutil_int_array_free(
    GUtilIntArray* array,
    gboolean free_data)
{
    if (array) {
        if (array->count) {
            return (int*)g_array_free((GArray*)array, free_data);
        } else {
            return (int*)g_array_free((GArray*)array, TRUE);
        }
    } else {
        return NULL;
    }
}

GUtilInts*
gutil_int_array_free_to_ints(
    GUtilIntArray* array)
{
    if (array) {
        if (array->count) {
            guint count = array->count;
            int* values = gutil_int_array_free(array, FALSE);
            return gutil_ints_new_take(values, count);
        } else {
            gutil_int_array_free(array, TRUE);
        }
    }
    return NULL;
}

GUtilIntArray*
gutil_int_array_ref(
    GUtilIntArray* array)
{
    if (array) {
        return (GUtilIntArray*)g_array_ref((GArray*)array);
    }
    return NULL;
}

void
gutil_int_array_unref(
    GUtilIntArray* array)
{
    if (array) {
        g_array_unref((GArray*)array);
    }
}

GUtilIntArray*
gutil_int_array_append(
    GUtilIntArray* array,
    int val)
{
    return gutil_int_array_append_vals(array, &val, 1);
}

GUtilIntArray*
gutil_int_array_append_vals(
    GUtilIntArray* array,
    const int* vals,
    guint count)
{
    if (array) {
        g_array_append_vals((GArray*)array, (void*)vals, count);
    }
    return array;
}

GUtilIntArray*
gutil_int_array_prepend(
    GUtilIntArray* array,
    int val)
{
    return gutil_int_array_prepend_vals(array, &val, 1);
}

GUtilIntArray*
gutil_int_array_prepend_vals(
    GUtilIntArray* array,
    const int* vals,
    guint count)
{
    if (array) {
        g_array_prepend_vals((GArray*)array, (void*)vals, count);
    }
    return array;
}

GUtilIntArray*
gutil_int_array_insert(
    GUtilIntArray* array,
    guint pos,
    int val)
{
    return gutil_int_array_insert_vals(array, pos, &val, 1);
}

GUtilIntArray*
gutil_int_array_insert_vals(
    GUtilIntArray* array,
    guint pos,
    const int* vals,
    guint count)
{
    if (array) {
        g_array_insert_vals((GArray*)array, pos, (void*)vals, count);
    }
    return array;
}

GUtilIntArray*
gutil_int_array_set_count(
    GUtilIntArray* array,
    guint count)
{
    if (array) {
        g_array_set_size((GArray*)array, count);
    }
    return array;
}

int
gutil_int_array_find(
    GUtilIntArray* array,
    int value)
{
    if (array) {
        guint i;
        for (i = 0; i < array->count; i++) {
            if (array->data[i] == value) {
                return i;
            }
        }
    }
    return -1;
}

gboolean
gutil_int_array_contains(
    GUtilIntArray* array,
    int value)
{
    return gutil_int_array_find(array, value) >= 0;
}

gboolean
gutil_int_array_remove(
    GUtilIntArray* array,
    int value)
{
    int pos = gutil_int_array_find(array, value);
    if (pos >= 0) {
        g_array_remove_index((GArray*)array, pos);
        return TRUE;
    }
    return FALSE;
}

gboolean
gutil_int_array_remove_fast(
    GUtilIntArray* array,
    int value)
{
    int pos = gutil_int_array_find(array, value);
    if (pos >= 0) {
        g_array_remove_index_fast((GArray*)array, pos);
        return TRUE;
    }
    return FALSE;
}

guint
gutil_int_array_remove_all(
    GUtilIntArray* array,
    int value)
{
    guint n;
    for (n = 0; gutil_int_array_remove(array, value); n++);
    return n;
}

guint
gutil_int_array_remove_all_fast(
    GUtilIntArray* array,
    int value)
{
    guint n;
    for (n = 0; gutil_int_array_remove_fast(array, value); n++);
    return n;
}

GUtilIntArray*
gutil_int_array_remove_index(
    GUtilIntArray* array,
    guint pos)
{
    if (array && pos < array->count) {
        g_array_remove_index((GArray*)array, pos);
    }
    return array;
}

GUtilIntArray*
gutil_int_array_remove_index_fast(
    GUtilIntArray* array,
    guint pos)
{
    if (array) {
        g_array_remove_index_fast((GArray*)array, pos);
    }
    return array;
}

GUtilIntArray*
gutil_int_array_remove_range(
    GUtilIntArray* array,
    guint pos,
    guint count)
{
    if (array && pos < array->count && count) {
        guint end = pos + count;
        if (end > array->count) {
            end = array->count;
        }
        g_array_remove_range((GArray*)array, pos, end - pos);
    }
    return array;
}

static
gint
gutil_int_array_sort_ascending_proc(
    gconstpointer a,
    gconstpointer b)
{
    const int* v1 = a;
    const int* v2 = b;
    return *v1 - *v2;
}

void
gutil_int_array_sort_ascending(
    GUtilIntArray* array)
{
    if (array) {
        g_array_sort((GArray*)array, gutil_int_array_sort_ascending_proc);
    }
}

static
gint
gutil_int_array_sort_descending_proc(
    gconstpointer a,
    gconstpointer b)
{
    const int* v1 = a;
    const int* v2 = b;
    return *v2 - *v1;
}

void
gutil_int_array_sort_descending(
    GUtilIntArray* array)
{
    if (array) {
        g_array_sort((GArray*)array, gutil_int_array_sort_descending_proc);
    }
}

/*
 * Local Variables:
 * mode: C
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
