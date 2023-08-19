/*
 * Copyright (C) 2023 Slava Monich <slava@monich.com>
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

#include "gutil_objv.h"
#include "gutil_misc.h"

static TestOpt test_opt;

/*==========================================================================*
 * null
 *==========================================================================*/

static
void
test_null(
    void)
{
    g_assert(!gutil_objv_copy(NULL));
    g_assert(!gutil_objv_add(NULL, NULL));
    g_assert(!gutil_objv_append(NULL, NULL));
    g_assert(!gutil_objv_insert(NULL, NULL, 0));
    g_assert(!gutil_objv_remove(NULL, NULL, FALSE));
    g_assert(!gutil_objv_remove_at(NULL, 0));
    g_assert(!gutil_objv_at(NULL, 0));
    g_assert(!gutil_objv_first(NULL));
    g_assert(!gutil_objv_last(NULL));
    g_assert(!gutil_objv_contains(NULL, NULL));
    g_assert_cmpint(gutil_objv_find(NULL, NULL), < ,0);
    g_assert_cmpint(gutil_objv_find_last(NULL, NULL), < ,0);
    g_assert(gutil_objv_equal(NULL, NULL));
    gutil_objv_free(NULL);
}

/*==========================================================================*
 * basic
 *==========================================================================*/

static
void
test_basic(
    void)
{
    GObject* o1 = g_object_new(TEST_OBJECT_TYPE, NULL);
    GObject* o2 = g_object_new(TEST_OBJECT_TYPE, NULL);
    GObject** v = gutil_objv_add(NULL, o1);
    GWeakRef r1, r2;

    g_weak_ref_init(&r1, o1);
    g_weak_ref_init(&r2, o2);

    /* v keeps references to both objects */
    g_object_unref(o1);
    g_assert(g_weak_ref_get(&r1) == o1);
    g_object_unref(o1);

    g_assert(gutil_objv_contains(v, o1));
    g_assert(!gutil_objv_contains(v, o2));

    g_assert_cmpuint(gutil_ptrv_length(v), == ,1);
    v = gutil_objv_add(v, o2);
    g_assert_cmpuint(gutil_ptrv_length(v), == ,2);
    g_assert(gutil_objv_contains(v, o2));

    g_assert(gutil_objv_at(v, 0) == o1);
    g_assert(gutil_objv_at(v, 1) == o2);
    g_assert(!gutil_objv_at(v, 2));
    g_assert(!gutil_objv_at(v, 3));

    g_assert(gutil_objv_first(v) == o1);
    g_assert(gutil_objv_last(v) == o2);
    g_assert_cmpint(gutil_objv_find(v, o1), == ,0);
    g_assert_cmpint(gutil_objv_find_last(v, o1), == ,0);

    v = gutil_objv_remove(v, o1, FALSE);
    g_assert_cmpuint(gutil_ptrv_length(v), == ,1);
    g_assert(!gutil_objv_at(v, 1));
    g_assert(gutil_objv_remove(v, o1, FALSE) == v);
    g_assert(gutil_objv_remove(v, NULL, FALSE) == v);
    g_assert_cmpuint(gutil_ptrv_length(v), == ,1);
    g_assert(!g_weak_ref_get(&r1));

    g_object_unref(o2);
    gutil_objv_free(v);
    g_assert(!g_weak_ref_get(&r2));
}

/*==========================================================================*
 * new
 *==========================================================================*/

static
void
test_new(
    void)
{
    GObject* o1 = g_object_new(TEST_OBJECT_TYPE, NULL);
    GObject* o2 = g_object_new(TEST_OBJECT_TYPE, NULL);
    GObject** v;
    GWeakRef r1, r2;

    g_weak_ref_init(&r1, o1);
    g_weak_ref_init(&r2, o2);

    v = gutil_objv_new(NULL, NULL);
    g_assert(v);
    g_assert(!gutil_ptrv_length(v));
    gutil_objv_free(v);

    /* v keeps references to both objects */
    v = gutil_objv_new(o1, o2, NULL);
    g_assert(v[0] == o1);
    g_assert(v[1] == o2);
    g_assert(!v[2]);

    g_object_unref(o1);
    g_assert(g_weak_ref_get(&r1) == o1);
    g_object_unref(o1);

    g_object_unref(o2);
    g_assert(g_weak_ref_get(&r2) == o2);
    g_object_unref(o2);

    gutil_objv_free(v);
    g_assert(!g_weak_ref_get(&r1));
    g_assert(!g_weak_ref_get(&r2));
}

/*==========================================================================*
 * insert
 *==========================================================================*/

static
void
test_insert(
    void)
{
    GObject* o1 = g_object_new(TEST_OBJECT_TYPE, NULL);
    GObject* o2 = g_object_new(TEST_OBJECT_TYPE, NULL);
    GObject* o3 = g_object_new(TEST_OBJECT_TYPE, NULL);
    GObject** v;
    GWeakRef r1, r2, r3;

    g_weak_ref_init(&r1, o1);
    g_weak_ref_init(&r2, o2);
    g_weak_ref_init(&r3, o3);

    v = gutil_objv_add(gutil_objv_add(NULL, o1), o2);

    /* Insert at the end (with index beyond the valid range) */
    v = gutil_objv_insert(v, o3, 100);

    g_assert_cmpuint(gutil_ptrv_length(v), == ,3);
    g_assert(gutil_objv_at(v, 0) == o1);
    g_assert(gutil_objv_at(v, 1) == o2);
    g_assert(gutil_objv_at(v, 2) == o3);

    /* Again at the end (with the right index) */
    v = gutil_objv_remove_at(v, 2);

    g_assert_cmpuint(gutil_ptrv_length(v), == ,2);
    g_assert(gutil_objv_at(v, 0) == o1);
    g_assert(gutil_objv_at(v, 1) == o2);

    v = gutil_objv_insert(v, o3, 2);

    g_assert_cmpuint(gutil_ptrv_length(v), == ,3);
    g_assert(gutil_objv_at(v, 0) == o1);
    g_assert(gutil_objv_at(v, 1) == o2);
    g_assert(gutil_objv_at(v, 2) == o3);

    /* At the beginning */
    v = gutil_objv_remove_at(v, 0);

    g_assert_cmpuint(gutil_ptrv_length(v), == ,2);
    g_assert(gutil_objv_at(v, 0) == o2);
    g_assert(gutil_objv_at(v, 1) == o3);

    v = gutil_objv_insert(v, o1, 0);

    g_assert_cmpuint(gutil_ptrv_length(v), == ,3);
    g_assert(gutil_objv_at(v, 0) == o1);
    g_assert(gutil_objv_at(v, 1) == o2);
    g_assert(gutil_objv_at(v, 2) == o3);

    /* And at the middle */
    v = gutil_objv_remove_at(v, 1);

    g_assert_cmpuint(gutil_ptrv_length(v), == ,2);
    g_assert(gutil_objv_at(v, 0) == o1);
    g_assert(gutil_objv_at(v, 1) == o3);

    v = gutil_objv_insert(v, o2, 1);

    g_assert_cmpuint(gutil_ptrv_length(v), == ,3);
    g_assert(gutil_objv_at(v, 0) == o1);
    g_assert(gutil_objv_at(v, 1) == o2);
    g_assert(gutil_objv_at(v, 2) == o3);

    g_object_unref(o1);
    g_object_unref(o2);
    g_object_unref(o3);
    gutil_objv_free(v);

    g_assert(!g_weak_ref_get(&r1));
    g_assert(!g_weak_ref_get(&r2));
    g_assert(!g_weak_ref_get(&r3));
}

/*==========================================================================*
 * append
 *==========================================================================*/

static
void
test_append(
    void)
{
    GObject* o1 = g_object_new(TEST_OBJECT_TYPE, NULL);
    GObject* o2 = g_object_new(TEST_OBJECT_TYPE, NULL);
    GObject* o3 = g_object_new(TEST_OBJECT_TYPE, NULL);
    GObject** v1 = NULL;
    GObject** v2 = NULL;
    GWeakRef r1, r2, r3;

    g_weak_ref_init(&r1, o1);
    g_weak_ref_init(&r2, o2);
    g_weak_ref_init(&r3, o3);

    v1 = gutil_objv_add(NULL, o1);
    v2 = gutil_objv_append(NULL, v1);
    g_assert_cmpuint(gutil_ptrv_length(v2), == ,1);
    g_assert(gutil_objv_equal(v1, v2));
    gutil_objv_free(v2);

    v2 = gutil_objv_add(gutil_objv_add(NULL, o2), o3);
    g_assert(gutil_objv_append(v2, NULL) == v2);

    v1 = gutil_objv_append(v1, v2);

    g_assert_cmpuint(gutil_ptrv_length(v1), == ,3);
    g_assert_cmpuint(gutil_ptrv_length(v2), == ,2);
    g_assert(gutil_objv_at(v1, 0) == o1);
    g_assert(gutil_objv_at(v1, 1) == o2);
    g_assert(gutil_objv_at(v1, 2) == o3);

    g_object_unref(o1);
    g_object_unref(o2);
    g_object_unref(o3);

    gutil_objv_free(v1);
    gutil_objv_free(v2);

    g_assert(!g_weak_ref_get(&r1));
    g_assert(!g_weak_ref_get(&r2));
    g_assert(!g_weak_ref_get(&r3));
}

/*==========================================================================*
 * copy
 *==========================================================================*/

static
void
test_copy(
    void)
{
    GObject* o1 = g_object_new(TEST_OBJECT_TYPE, NULL);
    GObject* o2 = g_object_new(TEST_OBJECT_TYPE, NULL);
    GObject** v1;
    GObject** v2;
    GWeakRef r1, r2;

    g_weak_ref_init(&r1, o1);
    g_weak_ref_init(&r2, o2);

    v1 = gutil_objv_add(gutil_objv_add(NULL, o1), o2);
    v2 = gutil_objv_copy(v1);

    /* Don't need these references anymore */
    g_object_unref(o1);
    g_object_unref(o2);

    g_assert_cmpuint(gutil_ptrv_length(v1), == ,2);
    g_assert_cmpuint(gutil_ptrv_length(v2), == ,2);
    g_assert(gutil_objv_equal(v1, v2));
    g_assert(gutil_objv_equal(v2, v1));
    g_assert(gutil_objv_equal(v1, v1));

    v1 = gutil_objv_remove_at(v1, 1);
    g_assert(!gutil_objv_equal(v1, v2));
    g_assert(!gutil_objv_equal(v2, v1));
    g_assert(!gutil_objv_equal(v1, NULL));
    g_assert(!gutil_objv_equal(NULL, v1));

    v2 = gutil_objv_remove_at(v2, 0);
    g_assert(!gutil_objv_equal(v1, v2));
    g_assert(!gutil_objv_equal(v2, v1));

    v1 = gutil_objv_remove_at(v1, 0);
    g_assert(gutil_objv_remove_at(v1, 0) == v1);
    g_assert(gutil_objv_equal(v1, NULL));
    g_assert(gutil_objv_equal(NULL, v1));
    g_assert(!gutil_objv_first(v1));
    g_assert(!gutil_objv_last(v1));

    g_assert_cmpint(gutil_objv_find(v1, NULL), < ,0);
    g_assert_cmpint(gutil_objv_find_last(v1, NULL), < ,0);
    g_assert_cmpint(gutil_objv_find(v1, o1), < ,0);
    g_assert_cmpint(gutil_objv_find_last(v1, o1), < ,0);
    g_assert(!gutil_objv_contains(v1, NULL));
    g_assert(!gutil_objv_contains(v1, o1));

    gutil_objv_free(v1);
    gutil_objv_free(v2);

    g_assert(!g_weak_ref_get(&r1));
    g_assert(!g_weak_ref_get(&r2));
}

/*==========================================================================*
 * remove
 *==========================================================================*/

static
void
test_remove(
    void)
{
    GObject* o1 = g_object_new(TEST_OBJECT_TYPE, NULL);
    GObject* o2 = g_object_new(TEST_OBJECT_TYPE, NULL);
    GObject** v;
    GWeakRef r1, r2;

    g_weak_ref_init(&r1, o1);
    g_weak_ref_init(&r2, o2);
    v = gutil_objv_add(gutil_objv_add(gutil_objv_add(NULL, o1), o2), o1);
    g_assert_cmpint(gutil_objv_find(v, o1), == ,0);
    g_assert_cmpint(gutil_objv_find_last(v, o1), == ,2);
    v = gutil_objv_remove(v, o1, TRUE);
    g_assert_cmpuint(gutil_ptrv_length(v), == ,1);
    g_assert(!gutil_objv_contains(v, o1));
    gutil_objv_free(v);

    g_object_unref(o1);
    g_object_unref(o2);

    g_assert(!g_weak_ref_get(&r1));
    g_assert(!g_weak_ref_get(&r2));
}

/*==========================================================================*
 * Common
 *==========================================================================*/

#define TEST_(t) "/objv/" t

int main(int argc, char* argv[])
{
    g_type_init();
    g_test_init(&argc, &argv, NULL);
    g_test_add_func(TEST_("null"), test_null);
    g_test_add_func(TEST_("basic"), test_basic);
    g_test_add_func(TEST_("new"), test_new);
    g_test_add_func(TEST_("insert"), test_insert);
    g_test_add_func(TEST_("append"), test_append);
    g_test_add_func(TEST_("copy"), test_copy);
    g_test_add_func(TEST_("remove"), test_remove);
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
