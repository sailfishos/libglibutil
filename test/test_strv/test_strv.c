/*
 * Copyright (C) 2015-2016 Jolla Ltd.
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

#include "test_common.h"

#include "gutil_strv.h"
#include "gutil_log.h"

/*==========================================================================*
 * Basic
 *==========================================================================*/

static
int
test_basic(
    const TestDesc* test,
    guint flags)
{
    int ret = RET_ERR;
    char** sv = g_strsplit("a,b", ",", 0);

    if (gutil_strv_length(NULL) == 0 &&
        gutil_strv_length(sv) == 2 &&
        !g_strcmp0(gutil_strv_at(sv, 0), "a") &&
        !g_strcmp0(gutil_strv_at(sv, 1), "b") &&
        !gutil_strv_at(sv, 2) &&
        !gutil_strv_at(sv, 3) &&
        !gutil_strv_at(NULL, 0)) {
        ret = RET_OK;
    }

    g_strfreev(sv);
    return ret;
}

/*==========================================================================*
 * Equal
 *==========================================================================*/

static
int
test_equal(
    const TestDesc* test,
    guint flags)
{
    int ret = RET_OK;
    /* gutil_strv_add(NULL, NULL) is a nop */
    char** sv1 = gutil_strv_add(gutil_strv_add(gutil_strv_add(gutil_strv_add(
        gutil_strv_add(gutil_strv_add(NULL, NULL), "a"), "b"), "c"), " "), "");
    char** sv2 = g_strsplit("a,b,c, ,", ",", 0);
    char** sv3 = g_strsplit("a,a,a, ,", ",", 0);
    char** sv4 = g_strsplit("a,b,c,,", ",", 0);
    char** sv5 = g_strsplit("a,b,c,", ",", 0);

    GASSERT(gutil_strv_equal(sv1, sv2));
    if (!gutil_strv_equal(sv1, sv2)) ret = RET_ERR;

    GASSERT(!gutil_strv_equal(sv1, sv3));
    if (gutil_strv_equal(sv1, sv3)) ret = RET_ERR;

    GASSERT(!gutil_strv_equal(sv1, sv4));
    if (gutil_strv_equal(sv1, sv4)) ret = RET_ERR;

    GASSERT(!gutil_strv_equal(sv1, sv5));
    if (gutil_strv_equal(sv1, sv5)) ret = RET_ERR;

    g_strfreev(sv1);
    g_strfreev(sv2);
    g_strfreev(sv3);
    g_strfreev(sv4);
    g_strfreev(sv5);
    return ret;
}

/*==========================================================================*
 * Find
 *==========================================================================*/

static
int
test_find(
    const TestDesc* test,
    guint flags)
{
    int ret = RET_OK;
    char** sv = g_strsplit("a,b,b,c", ",", 0);

    GASSERT(gutil_strv_contains(sv, "a"));
    if (!gutil_strv_contains(sv, "a")) ret = RET_ERR;

    GASSERT(gutil_strv_contains(sv, "b"));
    if (!gutil_strv_contains(sv, "b")) ret = RET_ERR;

    GASSERT(gutil_strv_contains(sv, "c"));
    if (!gutil_strv_contains(sv, "c")) ret = RET_ERR;

    GASSERT(!gutil_strv_contains(sv, "d"));
    if (gutil_strv_contains(sv, "d")) ret = RET_ERR;

    GASSERT(gutil_strv_find(sv, "b") == 1);
    if (gutil_strv_find(sv, "b") != 1) ret = RET_ERR;

    GASSERT(!gutil_strv_contains(NULL, "a"));
    if (gutil_strv_contains(NULL, "a")) ret = RET_ERR;

    GASSERT(!gutil_strv_contains(NULL, NULL));
    if (gutil_strv_contains(NULL, NULL)) ret = RET_ERR;

    g_strfreev(sv);
    return ret;
}

/*==========================================================================*
 * Remove
 *==========================================================================*/

static
int
test_remove(
    const TestDesc* test,
    guint flags)
{
    int ret = RET_OK;
    char** sv = g_strsplit("a,b,c", ",", 0);
    char* c = sv[2];

    if (gutil_strv_remove_at(NULL, 0, FALSE) ||
        gutil_strv_remove_at(sv, 3, FALSE) != sv ||
        gutil_strv_remove_at(sv, -1, FALSE) != sv) {
        ret = RET_ERR;
    }

    sv = gutil_strv_remove_at(sv, 2, FALSE);
    if (gutil_strv_contains(sv, "c")) ret = RET_ERR;

    sv = gutil_strv_remove_at(sv, 0, TRUE);
    if (gutil_strv_contains(sv, "a")) ret = RET_ERR;

    GASSERT(gutil_strv_length(sv) == 1);
    if (gutil_strv_length(sv) != 1) ret = RET_ERR;

    g_free(c);
    g_strfreev(sv);
    return ret;
}

/*==========================================================================*
 * Sort
 *==========================================================================*/

static
int
test_sort(
    const TestDesc* test,
    guint flags)
{
    int ret = RET_OK;
    char** in = g_strsplit("c,a,d,b", ",", 0);
    char** a1 = g_strsplit("a,b,c,d", ",", 0);
    char** d1 = g_strsplit("d,c,b,a", ",", 0);
    char** a2 = gutil_strv_sort(g_strdupv(in), TRUE);
    char** d2 = gutil_strv_sort(g_strdupv(in), FALSE);

    GASSERT(gutil_strv_equal(a1, a2));
    if (!gutil_strv_equal(a1, a2)) ret = RET_ERR;

    GASSERT(gutil_strv_equal(d1, d2));
    if (!gutil_strv_equal(d1, d2)) ret = RET_ERR;

    if (gutil_strv_sort(NULL, FALSE)) ret = RET_ERR;

    g_strfreev(a1);
    g_strfreev(a2);
    g_strfreev(d1);
    g_strfreev(d2);
    g_strfreev(in);
    return ret;
}

/*==========================================================================*
 * Common
 *==========================================================================*/

static const TestDesc all_tests[] = {
    { "Basic", test_basic },
    { "Equal", test_equal },
    { "Find", test_find },
    { "Remove", test_remove },
    { "Sort", test_sort }
};

int main(int argc, char* argv[])
{
    return TEST_MAIN(argc, argv, all_tests);
}

/*
 * Local Variables:
 * mode: C
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
