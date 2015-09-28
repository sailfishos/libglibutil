/*
 * Copyright (C) 2014-2015 Jolla Ltd.
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
 *   3. Neither the name of the Jolla Ltd nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
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

#include "gutil_strv.h"

/**
 * NULL-tolerant version of g_strv_length
 */
guint
gutil_strv_length(
    gchar** sv)
{
    if (G_LIKELY(sv)) {
        guint i = 0;
        while (sv[i]) i++;
        return i;
    } else {
        return 0;
    }
}

/**
 * Returns i-th strings in the array, NULL if index is out of bounds.
 */
const char*
gutil_strv_at(
    gchar** sv,
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
 * Returns index of the specified string in the string array,
 * or -1 if the string is not found.
 */
int
gutil_strv_find(
    char** sv,
    const char* s)
{
    if (sv && s) {
        int i = 0;
        char** ptr;
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
    char** sv,
    const char* s)
{
    return gutil_strv_find(sv, s) >= 0;
}

/**
 * Appends new string to the array.
 */
char**
gutil_strv_add(
    char** sv,
    const char* s)
{
    if (s) {
        const guint len = gutil_strv_length(sv);
        char** newsv = g_realloc(sv, sizeof(char*)*(len+2));
        newsv[len] = g_strdup(s);
        newsv[len+1] = NULL;
        return newsv;
    } else {
        return sv;
    }
}

/**
 * Checks two string arrays for equality.
 */
gboolean
gutil_strv_equal(
    char** sv1,
    char** sv2)
{
    const guint len1 = gutil_strv_length(sv1);
    const guint len2 = gutil_strv_length(sv2);
    if (len1 == len2) {
        guint i;
        for (i=0; i<len1; i++) {
            if (strcmp(sv1[i], sv2[i])) {
                return FALSE;
            }
        }
        return TRUE;
    }
    return FALSE;
}

/*
 * Local Variables:
 * mode: C
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
