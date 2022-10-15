/*
 * Copyright (C) 2017-2022 Jolla Ltd.
 * Copyright (C) 2017-2022 Slava Monich <slava.monich@jolla.com>
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

#include "test_common.h"

#include "gutil_ints.h"
#include "gutil_misc.h"

static TestOpt test_opt;

/*==========================================================================*
 * NULL tolerance
 *==========================================================================*/

static
void
test_ints_null(
    void)
{
    int val = 0;
    guint count;
    gutil_ints_unref(NULL);
    g_assert(!gutil_ints_ref(NULL));
    g_assert(!gutil_ints_new(NULL, 0));
    g_assert(!gutil_ints_new(NULL, 1));
    g_assert(!gutil_ints_new(&val, 0));
    g_assert(!gutil_ints_new_take(NULL, 0));
    g_assert(!gutil_ints_new_take(NULL, 1));
    g_assert(!gutil_ints_new_take(&val, 0));
    g_assert(!gutil_ints_new_static(NULL, 0));
    g_assert(!gutil_ints_new_static(NULL, 1));
    g_assert(!gutil_ints_new_static(&val, 0));
    g_assert(!gutil_ints_new_with_free_func(NULL, 0, NULL, NULL));
    g_assert(!gutil_ints_new_with_free_func(NULL, 1, NULL, NULL));
    g_assert(!gutil_ints_new_with_free_func(&val, 0, NULL, NULL));
    g_assert(!gutil_ints_new_from_ints(NULL, 0, 0));
    g_assert(!gutil_ints_hash(NULL));
    g_assert(!gutil_ints_get_count(NULL));
    g_assert(!gutil_ints_contains(NULL, 0));
    g_assert(gutil_ints_find(NULL, 0) < 0);
    count = 1;
    g_assert(!gutil_ints_get_data(NULL, NULL));
    g_assert(!gutil_ints_get_data(NULL, &count));
    g_assert(!count);
    count = 1;
    g_assert(!gutil_ints_unref_to_data(NULL, NULL));
    g_assert(!gutil_ints_unref_to_data(NULL, &count));
    g_assert(!count);
}

/*==========================================================================*
 * Basic
 *==========================================================================*/

static
void
test_custom_free(
    gpointer data)
{
    g_free(data);
}

static
void
test_ints_basic(
    void)
{
    /* First 3 elements match */
    const int a1[] = { 1, 2, 3 };
    const int a2[] = { 1, 2, 3, 4 };
    int* data;
    guint count;
    GUtilInts* i1 = gutil_ints_new(a1, G_N_ELEMENTS(a1));
    GUtilInts* i2 = gutil_ints_new_static(a2, G_N_ELEMENTS(a2));
    GUtilInts* i3;
    GUtilInts* i4;
    g_assert(gutil_ints_get_count(i1) == G_N_ELEMENTS(a1));
    g_assert(gutil_ints_get_count(i2) == G_N_ELEMENTS(a2));
    g_assert(gutil_ints_get_data(i1, NULL) != a2);
    g_assert(gutil_ints_get_data(i2, &count) == a2);
    g_assert(count == G_N_ELEMENTS(a2));
    g_assert(!gutil_ints_new_from_ints(i1, G_N_ELEMENTS(a2), 1));
    i3 = gutil_ints_new_from_ints(i1, 0, G_N_ELEMENTS(a1) + 1);
    i4 = gutil_ints_new_from_ints(i2, 0, G_N_ELEMENTS(a1));

    g_assert(gutil_ints_equal(i1, i1));
    g_assert(!gutil_ints_equal(i1, i2));
    g_assert(gutil_ints_equal(i1, i3));
    g_assert(gutil_ints_equal(i1, i3));

    g_assert(!gutil_ints_contains(i1, 0));
    g_assert(gutil_ints_contains(i1, 1));

    g_assert(gutil_ints_find(i1, 0) < 0);
    g_assert(gutil_ints_find(i1, 1) == 0);
    g_assert(gutil_ints_find(i1, 2) == 1);

    /* This gutil_ints_unref_to_data doesn't actually free i1 because
     * a reference to it is held by i3: */
    data = gutil_ints_unref_to_data(i1, &count);
    g_assert(count == G_N_ELEMENTS(a1));
    g_assert(!memcmp(a1, data, count*sizeof(int)));
    g_free(data);

    gutil_ints_unref(i2);
    gutil_ints_unref(i3);
    gutil_ints_unref(i4);

    /* This gutil_ints_unref_to_data actually does free i1: */
    i1 = gutil_ints_new(a1, G_N_ELEMENTS(a1));
    data = gutil_ints_unref_to_data(i1, NULL);
    g_assert(!memcmp(a1, data, G_N_ELEMENTS(a1)*sizeof(int)));
    g_free(data);

    /* And this one duplicates the data because we use test_custom_free: */
    data = gutil_memdup(a1, sizeof(a1));
    i1 = gutil_ints_new_with_free_func(data, G_N_ELEMENTS(a1),
        test_custom_free, data);
    data = gutil_ints_unref_to_data(i1, &count);
    g_assert(count == G_N_ELEMENTS(a1));
    g_assert(!memcmp(a1, data, count*sizeof(int)));
    g_free(data);
}

/*==========================================================================*
 * Compare
 *==========================================================================*/

static
void
test_ints_compare(
    void)
{
    const int a1[] = { 1 };
    const int a2[] = { 1, 2 };
    const int a3[] = { 2 };
    GUtilInts* i1 = gutil_ints_new_static(a1, G_N_ELEMENTS(a1));
    GUtilInts* i2 = gutil_ints_new_static(a2, G_N_ELEMENTS(a2));
    GUtilInts* i3 = gutil_ints_new_static(a3, G_N_ELEMENTS(a3));
    GUtilInts* i4 = gutil_ints_new_from_ints(i1, 0, G_N_ELEMENTS(a1));

    g_assert(gutil_ints_hash(i1) == 1235);
    g_assert(gutil_ints_hash(i2) == 1239);

    g_assert(gutil_ints_equal(NULL, NULL));
    g_assert(!gutil_ints_equal(NULL, i1));
    g_assert(!gutil_ints_equal(i1, NULL));
    g_assert(!gutil_ints_equal(i1, i2));
    g_assert(!gutil_ints_equal(i2, i1));
    g_assert(!gutil_ints_equal(i1, i3));
    g_assert(!gutil_ints_equal(i3, i1));
    g_assert(gutil_ints_equal(i1, i4));
    g_assert(gutil_ints_equal(i4, i1));

    g_assert(gutil_ints_compare(NULL, NULL) == 0);
    g_assert(gutil_ints_compare(NULL, i1) < 0);
    g_assert(gutil_ints_compare(i1, NULL) > 0);
    g_assert(gutil_ints_compare(i1, i2) < 0);
    g_assert(gutil_ints_compare(i2, i1) > 0);
    g_assert(gutil_ints_compare(i1, i3) < 0);
    g_assert(gutil_ints_compare(i3, i1) > 0);
    g_assert(gutil_ints_compare(i1, i4) == 0);
    g_assert(gutil_ints_compare(i4, i1) == 0);

    gutil_ints_unref(i1);
    gutil_ints_unref(i2);
    gutil_ints_unref(i3);
    gutil_ints_unref(i4);
}

/*==========================================================================*
 * Common
 *==========================================================================*/

#define TEST_PREFIX "/ints/"

int main(int argc, char* argv[])
{
    g_test_init(&argc, &argv, NULL);
    g_test_add_func(TEST_PREFIX "null", test_ints_null);
    g_test_add_func(TEST_PREFIX "basic", test_ints_basic);
    g_test_add_func(TEST_PREFIX "compare", test_ints_compare);
    test_init(&test_opt, argc, argv);
    return g_test_run();
}

/*
 * Local Variables:
 * mode: C
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
