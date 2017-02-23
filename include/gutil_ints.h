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

#ifndef GUTIL_INTS_H
#define GUTIL_INTS_H

#include "gutil_types.h"

G_BEGIN_DECLS

/*
 * A read-only non-empty array of ints. NULL is the empty array.
 * If it's not NULL then it's not empty.
 */

GUtilInts*
gutil_ints_new(
    const int* data,
    guint count);

GUtilInts*
gutil_ints_new_take(
    int* data,
    guint count);

GUtilInts*
gutil_ints_new_static(
    const int* data,
    guint count);

GUtilInts*
gutil_ints_new_with_free_func(
    const int* data,
    guint count,
    GDestroyNotify free_func,
    gpointer user_data);

GUtilInts*
gutil_ints_new_from_ints(
    GUtilInts* ints,
    guint offset,
    guint count);

GUtilInts*
gutil_ints_ref(
    GUtilInts* ints);

void
gutil_ints_unref(
    GUtilInts* ints);

int*
gutil_ints_unref_to_data(
    GUtilInts* ints,
    guint* count);

const int*
gutil_ints_get_data(
    GUtilInts* ints,
    guint* count);

guint
gutil_ints_get_count(
    GUtilInts* ints);

int
gutil_ints_find(
    GUtilInts* ints,
    int value);

gboolean
gutil_ints_contains(
    GUtilInts* ints,
    int value);

guint
gutil_ints_hash(
    gconstpointer ints);

gboolean
gutil_ints_equal(
    gconstpointer a,
    gconstpointer b);

gint
gutil_ints_compare(
    gconstpointer a,
    gconstpointer b);

G_END_DECLS

#endif /* GUTIL_INTS_H */

/*
 * Local Variables:
 * mode: C
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
