/*
 * SPDX-FileCopyrightText: 2026 Jolla Mobile Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "test_common.h"

#include "gutil_cleanup.h"
#include "gutil_strv.h"

static TestOpt test_opt;

static
void
test_cleanup_inc(
    gpointer data)
{
    (*((int*)data))++;
}

/*==========================================================================*
 * null
 *==========================================================================*/

static
void
test_null(
    void)
{
    GUtilCleanup* cleanup = gutil_cleanup_new();

    g_assert(!gutil_cleanup_add(NULL, NULL, NULL));
    g_assert(!gutil_cleanup_add(NULL, test_cleanup_inc, NULL));
    g_assert(!gutil_cleanup_add(cleanup, NULL, cleanup));
    g_assert(!gutil_cleanup_add_ptr(NULL, NULL));
    g_assert(!gutil_cleanup_add_ptr(cleanup, NULL));
    g_assert(!gutil_cleanup_add_strv(NULL, NULL));
    g_assert(!gutil_cleanup_add_strv(cleanup, NULL));
    g_assert(!gutil_cleanup_add_object(NULL, NULL));
    g_assert(!gutil_cleanup_add_object(cleanup, NULL));
    g_assert(!gutil_cleanup_add_object_ref(NULL, NULL));
    g_assert(!gutil_cleanup_add_object_ref(cleanup, NULL));
    g_assert(!gutil_cleanup_add_variant(NULL, NULL));
    g_assert(!gutil_cleanup_add_variant(cleanup, NULL));
    g_assert(!gutil_cleanup_add_variant_ref(NULL, NULL));
    g_assert(!gutil_cleanup_add_variant_ref(cleanup, NULL));
    g_assert(!gutil_cleanup_add_bytes(NULL, NULL));
    g_assert(!gutil_cleanup_add_bytes(cleanup, NULL));
    g_assert(!gutil_cleanup_add_bytes_ref(NULL, NULL));
    g_assert(!gutil_cleanup_add_bytes_ref(cleanup, NULL));
    g_assert_false(gutil_cleanup_drop(NULL, NULL));
    g_assert_false(gutil_cleanup_drop(cleanup, NULL));
    gutil_cleanup_free(NULL);
    gutil_cleanup_clear(NULL);
    gutil_cleanup_free(cleanup);
}

/*==========================================================================*
 * basic
 *==========================================================================*/

static
void
test_basic(
    void)
{
    int n1 = 0, n2 = 0;
    GUtilCleanup* cleanup = gutil_cleanup_new();
    GBytes* bytes = g_bytes_new(&n1, sizeof(n1));
    GObject* object = g_object_new(TEST_OBJECT_TYPE, NULL);

    g_assert(gutil_cleanup_add_strv(cleanup, gutil_strv_add(NULL, "")));
    g_assert(gutil_cleanup_add_variant_ref(cleanup, g_variant_new_string("")));
    g_assert(gutil_cleanup_add_bytes_ref(cleanup, bytes) == bytes);
    g_assert(gutil_cleanup_add_object_ref(cleanup, object) == object);
    g_assert(gutil_cleanup_add(cleanup, test_cleanup_inc, &n1) == &n1);
    g_assert(gutil_cleanup_add(cleanup, test_cleanup_inc, &n2) == &n2);
    gutil_cleanup_free(cleanup);
    g_assert_cmpint(n1, == ,1);
    g_assert_cmpint(n2, == ,1);
    g_object_unref(object);
    g_bytes_unref(bytes);
}

/*==========================================================================*
 * drop
 *==========================================================================*/

static
void
test_drop(
    void)
{
    int n1 = 0, n2 = 0;
    GUtilCleanup* cleanup = gutil_cleanup_new();

    g_assert(gutil_cleanup_add(cleanup, test_cleanup_inc, &n1) == &n1);
    g_assert(gutil_cleanup_add(cleanup, test_cleanup_inc, &n2) == &n2);
    g_assert_true(gutil_cleanup_drop(cleanup, &n1));
    g_assert_cmpint(n1, == ,1);
    g_assert_cmpint(n2, == ,0);

    /* Second drop is a noop */
    g_assert_false(gutil_cleanup_drop(cleanup, &n1));
    g_assert_cmpint(n1, == ,1);
    g_assert_cmpint(n2, == ,0);

    gutil_cleanup_free(cleanup);
    g_assert_cmpint(n1, == ,1);
    g_assert_cmpint(n2, == ,1);
}

/*==========================================================================*
 * clear
 *==========================================================================*/

static
void
test_clear(
    void)
{
    int n1 = 0, n2 = 0;
    GUtilCleanup* cleanup = gutil_cleanup_new();

    g_assert(gutil_cleanup_add(cleanup, test_cleanup_inc, &n1) == &n1);
    g_assert(gutil_cleanup_add(cleanup, test_cleanup_inc, &n2) == &n2);
    gutil_cleanup_clear(cleanup);
    g_assert_cmpint(n1, == ,1);
    g_assert_cmpint(n2, == ,1);

    gutil_cleanup_free(cleanup);
    g_assert_cmpint(n1, == ,1);
    g_assert_cmpint(n2, == ,1);
}

/*==========================================================================*
 * Common
 *==========================================================================*/

#define TEST_PREFIX "/cleanup/"
#define TEST_(t) TEST_PREFIX t

int main(int argc, char* argv[])
{
    g_test_init(&argc, &argv, NULL);
    g_test_add_func(TEST_("null"), test_null);
    g_test_add_func(TEST_("basic"), test_basic);
    g_test_add_func(TEST_("drop"), test_drop);
    g_test_add_func(TEST_("clear"), test_clear);
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
