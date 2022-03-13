/*
 * Copyright (C) 2014-2022 Jolla Ltd.
 * Copyright (C) 2014-2022 Slava Monich <slava.monich@jolla.com>
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

#ifndef GUTIL_STRV_H
#define GUTIL_STRV_H

#include "gutil_types.h"

G_BEGIN_DECLS

/**
 * NULL-tolerant version of g_strv_length
 */
guint
gutil_strv_length(
    const GStrV* sv);

/**
 * Returns i-th strings in the array, NULL if index is out of bounds.
 */
const char*
gutil_strv_at(
    const GStrV* sv,
    guint i);

/**
 * Returns the last string in the array, NULL if there's nothing there.
 */
const char*
gutil_strv_last(
    const GStrV* sv); /* Since 1.0.35 */

/**
 * Returns index of the specified string in the string array,
 * or -1 if the string is not found.
 */
int
gutil_strv_find(
    const GStrV* sv,
    const char* s);

/**
 * Returns index of the last occurrence of specified string in the string
 * array, or -1 if the string is not found.
 */
int
gutil_strv_find_last(
    const GStrV* sv,
    const char* s);  /* Since 1.0.62 */

/**
 * Checks if string array contains the specified string.
 */
gboolean
gutil_strv_contains(
    const GStrV* sv,
    const char* s);

/**
 * Appends new string to the array.
 */
GStrV*
gutil_strv_add(
    GStrV* sv,
    const char* s)
    G_GNUC_WARN_UNUSED_RESULT;

/**
 * Appends new string(s) to the array.
 */
GStrV*
gutil_strv_addv(
    GStrV* sv,
    const char* s,
    ...) /* Since 1.0.47 */
    G_GNUC_WARN_UNUSED_RESULT
    G_GNUC_NULL_TERMINATED;

/**
 * Removes the string from the specified position in the array.
 */
GStrV*
gutil_strv_remove_at(
    GStrV* sv,
    int pos,
    gboolean free_string)
    G_GNUC_WARN_UNUSED_RESULT;

/**
 * Removes one or all matching strings from the array and frees them.
 */
GStrV*
gutil_strv_remove(
    GStrV* sv,
    const char* s,
    gboolean remove_all) /* Since 1.0.61 */
    G_GNUC_WARN_UNUSED_RESULT;

#define gutil_strv_remove_one(sv, s) gutil_strv_remove(sv, s, FALSE)
#define gutil_strv_remove_all(sv, s) gutil_strv_remove(sv, s, TRUE)

/**
 * Checks two string arrays for equality.
 */
gboolean
gutil_strv_equal(
    const GStrV* sv1,
    const GStrV* sv2);

/**
 * Removes all duplicates from the string array.
 */
GStrV*
gutil_strv_remove_dups(
    GStrV* sv); /* Since 1.0.62 */

/**
 * Sorts the string array
 */
GStrV*
gutil_strv_sort(
    GStrV* sv,
    gboolean ascending);

/**
 * Binary search in the sorted string array. Returns index of the
 * specified string in the string array, or -1 if the string is not
 * found. It's basically a version of gutil_strv_find optimized for
 * sorted arrays. The string array must be sorted by gutil_strv_sort
 * with the same 'ascending' argument.
 */
int
gutil_strv_bsearch(
    GStrV* sv,
    const char* s,
    gboolean ascending); /* Since 1.0.40 */

/**
 * Removes leading and trailing whitespaces from all strings in the vector.
 */
GStrV*
gutil_strv_strip(
    GStrV* sv);

G_END_DECLS

#endif /* GUTIL_STRV_H */

/*
 * Local Variables:
 * mode: C
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
