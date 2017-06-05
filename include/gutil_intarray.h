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

#ifndef GUTIL_INTARRAY_H
#define GUTIL_INTARRAY_H

#include "gutil_types.h"

G_BEGIN_DECLS

/*
 * This is basically a GArray providing better type safety at compile time.
 */
struct gutil_int_array {
    int* data;
    guint count;
};

GUtilIntArray*
gutil_int_array_new(
    void);

GUtilIntArray*
gutil_int_array_sized_new(
    guint reserved_count);

GUtilIntArray*
gutil_int_array_new_from_vals(
    const int* vals,
    guint count);

GUtilIntArray*
gutil_int_array_new_from_value(
    int value);

int*
gutil_int_array_free(
    GUtilIntArray* array,
    gboolean free_data);

GUtilInts*
gutil_int_array_free_to_ints(
    GUtilIntArray* array);

GUtilIntArray*
gutil_int_array_ref(
    GUtilIntArray* array);

void
gutil_int_array_unref(
    GUtilIntArray* array);

GUtilIntArray*
gutil_int_array_append(
    GUtilIntArray* array,
    int val);

GUtilIntArray*
gutil_int_array_append_vals(
    GUtilIntArray* array,
    const int* vals,
    guint count);

GUtilIntArray*
gutil_int_array_prepend(
    GUtilIntArray* array,
    int val);

GUtilIntArray*
gutil_int_array_prepend_vals(
    GUtilIntArray* array,
    const int* vals,
    guint count);

GUtilIntArray*
gutil_int_array_insert(
    GUtilIntArray* array,
    guint pos,
    int val);

GUtilIntArray*
gutil_int_array_insert_vals(
    GUtilIntArray* array,
    guint pos,
    const int* vals,
    guint count);

GUtilIntArray*
gutil_int_array_set_count(
    GUtilIntArray* array,
    guint count);

int
gutil_int_array_find(
    GUtilIntArray* array,
    int value);

gboolean
gutil_int_array_contains(
    GUtilIntArray* array,
    int value);

gboolean
gutil_int_array_remove(
    GUtilIntArray* array,
    int value);

gboolean
gutil_int_array_remove_fast(
    GUtilIntArray* array,
    int value);

guint
gutil_int_array_remove_all(
    GUtilIntArray* array,
    int value);

guint
gutil_int_array_remove_all_fast(
    GUtilIntArray* array,
    int value);

GUtilIntArray*
gutil_int_array_remove_index(
    GUtilIntArray* array,
    guint pos);

GUtilIntArray*
gutil_int_array_remove_index_fast(
    GUtilIntArray* array,
    guint pos);

GUtilIntArray*
gutil_int_array_remove_range(
    GUtilIntArray* array,
    guint pos,
    guint count);

void
gutil_int_array_sort_ascending(
    GUtilIntArray* array);

void
gutil_int_array_sort_descending(
    GUtilIntArray* array);

G_END_DECLS

#endif /* GUTIL_INTARRAY_H */

/*
 * Local Variables:
 * mode: C
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
