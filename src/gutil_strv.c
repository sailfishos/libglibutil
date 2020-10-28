/*
 * Copyright (C) 2014-2020 Jolla Ltd.
 * Copyright (C) 2014-2020 Slava Monich <slava.monich@jolla.com>
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

#include "gutil_strv.h"
#include "gutil_misc.h"

#include <stdlib.h>

/**
 * NULL-tolerant version of g_strv_length
 */
guint
gutil_strv_length(
    const GStrV* sv)
{
    return (guint) gutil_ptrv_length(sv);
}

/**
 * Returns i-th strings in the array, NULL if index is out of bounds.
 */
const char*
gutil_strv_at(
    const GStrV* sv,
    guint i)
{
    if (G_LIKELY(sv)) {
        guint k = 0;
        while (sv[k] && k < i) k++;
        if (k == i) {
            /* We also end up here if i == len but that's OK */
            return sv[k];
        }
    }
    return NULL;
}

/**
 * Returns the last string in the array, NULL if there's nothing there.
 */
const char*
gutil_strv_last(
    const GStrV* sv) /* Since 1.0.35 */
{
    if (G_LIKELY(sv) && G_LIKELY(sv[0])) {
        guint k = 0;
        while (sv[k + 1]) k++;
        return sv[k];
    }
    return NULL;
}

/**
 * Returns index of the specified string in the string array,
 * or -1 if the string is not found.
 */
int
gutil_strv_find(
    const GStrV* sv,
    const char* s)
{
    if (sv && s) {
        int i = 0;
        const GStrV* ptr;
        for (ptr = sv; *ptr; ptr++, i++) {
            if (!strcmp(*ptr, s)) {
                return i;
            }
        }
    }
    return -1;
}

/**
 * Checks if string array contains the specified string.
 */
gboolean
gutil_strv_contains(
    const GStrV* sv,
    const char* s)
{
    return gutil_strv_find(sv, s) >= 0;
}

/**
 * Appends new string to the array.
 */
GStrV*
gutil_strv_add(
    GStrV* sv,
    const char* s)
{
    if (s) {
        guint len = gutil_strv_length(sv);

        sv = g_renew(char*, sv, len + 2);
        sv[len++] = g_strdup(s);
        sv[len] = NULL;
    }
    return sv;
}

/**
 * Appends new strings to the array.
 */
GStrV*
gutil_strv_addv(
    GStrV* sv,
    const char* s,
    ...)
{
    if (s) {
        va_list va;
        const char* s1;
        guint len, i, n;

        va_start(va, s);
        for (n = 1; (s1 = va_arg(va, char*)) != NULL; n++);
        va_end(va);

        len = gutil_strv_length(sv);
        sv = g_renew(gchar*, sv, len + n + 1);
        sv[len++] = g_strdup(s);
        va_start(va, s);
        for (i = 1; i < n; i++) sv[len++] = g_strdup(va_arg(va, char*));
        va_end(va);
        sv[len] = NULL;
    }
    return sv;
}

/**
 * Removes the string from the specified position in the array.
 */
GStrV*
gutil_strv_remove_at(
    GStrV* sv,
    int pos,
    gboolean free_string)
{
    if (G_LIKELY(sv) && G_LIKELY(pos >= 0)) {
        const int len = gutil_strv_length(sv);
        if (pos < len) {
            if (free_string) {
                g_free(sv[pos]);
            }
            if (pos < len-1) {
                memmove(sv + pos, sv + pos + 1, sizeof(char*)*(len-pos-1));
            }
            sv[len-1] = NULL;
            sv = g_realloc(sv, sizeof(char*)*len);
        }
    }
    return sv;
}

/**
 * Checks two string arrays for equality. NULL and empty arrays are equal.
 *
 * This is basically a NULL-tolerant equivalent of g_strv_equal which
 * appeared in glib 2.60.
 */
gboolean
gutil_strv_equal(
    const GStrV* sv1,
    const GStrV* sv2)
{
    if (sv1 == sv2) {
        return TRUE;
    } else if (!sv1) {
        return !sv2[0];
    } else if (!sv2) {
        return !sv1[0];
    } else {
        guint len = 0;

        while (sv1[len] && sv2[len]) len++;
        if (!sv1[len] && !sv2[len]) {
            guint i;
            for (i=0; i<len; i++) {
                if (strcmp(sv1[i], sv2[i])) {
                    return FALSE;
                }
            }
            return TRUE;
        }
        return FALSE;
    }
}

static
int
gutil_strv_sort_ascending(
    const void* p1,
    const void* p2)
{
    return strcmp(*(char**)p1, *(char**)p2);
}

static
int
gutil_strv_sort_descending(
    const void* p1,
    const void* p2)
{
    return -strcmp(*(char**)p1, *(char**)p2);
}

/**
 * Sorts the string array
 */
GStrV*
gutil_strv_sort(
    GStrV* sv,
    gboolean ascending)
{
    guint len = gutil_strv_length(sv);
    if (len > 0) {
        qsort(sv, len, sizeof(char*), ascending ?
              gutil_strv_sort_ascending :
              gutil_strv_sort_descending);
    }
    return sv;
}

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
    gboolean ascending) /* Since 1.0.40 */
{
    if (s) {
        guint len = gutil_strv_length(sv);
        if (len > 0) {
            GStrV* found = bsearch(&s, sv, len, sizeof(char*), ascending ?
                gutil_strv_sort_ascending : gutil_strv_sort_descending);
            if (found) {
                return found - sv;
            }
        }
    }
    return -1;
}

/**
 * Removes leading and trailing whitespaces from all strings in the vector.
 */
GStrV*
gutil_strv_strip(
    GStrV* sv)
{
    if (sv) {
        GStrV* ptr;
        for (ptr = sv; *ptr; ptr++) {
            *ptr = g_strstrip(*ptr);
        }
    }
    return sv;
}

/*
 * Local Variables:
 * mode: C
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
