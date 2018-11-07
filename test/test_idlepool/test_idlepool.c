/*
 * Copyright (C) 2016-2018 Jolla Ltd.
 * Copyright (C) 2016-2018 Slava Monich <slava.monich@jolla.com>
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
 *      contributors may be used to endorse or promote products derived from
 *      this software without specific prior written permission.
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

#include "gutil_idlepool.h"
#include "gutil_strv.h"
#include "gutil_log.h"

#define TEST_TIMEOUT (10) /* seconds */

static TestOpt test_opt;

static
gboolean
test_timeout(
    gpointer loop)
{
    g_assert(!"TIMEOUT");
    return G_SOURCE_REMOVE;
}

static
void
test_done(
    gpointer loop)
{
    g_main_loop_quit(loop);
}

/*==========================================================================*
 * Null
 *==========================================================================*/

static
void
test_null(
    void)
{
    GUtilIdlePool* pool = gutil_idle_pool_new();

    /* These have no effect, just testing NULL-telerance */
    gutil_idle_pool_add_strv(NULL, NULL);
    gutil_idle_pool_add_strv(pool, NULL);
    gutil_idle_pool_add_ptr_array(NULL, NULL);
    gutil_idle_pool_add_ptr_array(pool, NULL);
    gutil_idle_pool_add_ptr_array_ref(NULL, NULL);
    gutil_idle_pool_add_ptr_array_ref(pool, NULL);
    gutil_idle_pool_add_variant(NULL, NULL);
    gutil_idle_pool_add_variant(pool, NULL);
    gutil_idle_pool_add_variant_ref(NULL, NULL);
    gutil_idle_pool_add_variant_ref(pool, NULL);
    gutil_idle_pool_add_object(NULL, NULL);
    gutil_idle_pool_add_object(pool, NULL);
    gutil_idle_pool_add_object_ref(NULL, NULL);
    gutil_idle_pool_add_object_ref(pool, NULL);
    gutil_idle_pool_add_bytes(NULL, NULL);
    gutil_idle_pool_add_bytes(pool, NULL);
    gutil_idle_pool_add_bytes_ref(NULL, NULL);
    gutil_idle_pool_add_bytes_ref(pool, NULL);
    gutil_idle_pool_add(NULL, NULL, NULL);
    gutil_idle_pool_add(pool, NULL, NULL);
    gutil_idle_pool_ref(NULL);
    gutil_idle_pool_unref(NULL);
    gutil_idle_pool_drain(NULL);
    gutil_idle_pool_unref(pool);
}

/*==========================================================================*
 * Basic
 *==========================================================================*/

typedef struct test_basic {
    GUtilIdlePool* pool;
    gboolean array_free_count;
    gboolean ok;
} TestBasic;

static
void
test_basic_array_free(
    gpointer param)
{
    TestBasic* test = param;
    test->array_free_count++;
}

static
void
test_basic_unref_pool(
    gpointer param)
{
    TestBasic* test = param;
    gutil_idle_pool_unref(test->pool);
}

static
void
test_basic_last(
    gpointer param)
{
    TestBasic* test = param;
    if (test->array_free_count == 1) {
        test->ok = TRUE;
    }
}

static
void
test_basic_add_during_drain(
    gpointer param)
{
    /* Add an item when pool is being drained */
    TestBasic* test = param;
    gutil_idle_pool_add(test->pool, test, test_basic_last);
}

static
void
test_basic(
    void)
{
    GPtrArray* array = g_ptr_array_new_with_free_func(test_basic_array_free);
    GVariant* variant = g_variant_take_ref(g_variant_new_int32(1));
    GObject* object = g_object_new(TEST_OBJECT_TYPE, NULL);
    GMainLoop* loop = g_main_loop_new(NULL, TRUE);
    GStrV* strv = gutil_strv_add(NULL, "foo");
    GBytes* bytes = g_bytes_new("bar", 3);
    TestBasic test;

    memset(&test, 0, sizeof(test));
    test.pool = gutil_idle_pool_new();

    if (!(test_opt.flags & TEST_FLAG_DEBUG)) {
        g_timeout_add_seconds(TEST_TIMEOUT, test_timeout, loop);
    }

    gutil_idle_pool_unref(gutil_idle_pool_new());
    gutil_idle_pool_add_strv(test.pool, strv);
    g_ptr_array_add(array, &test);
    gutil_idle_pool_add_ptr_array_ref(test.pool, array);
    g_ptr_array_unref(array);
    gutil_idle_pool_add_variant_ref(test.pool, variant);
    g_variant_unref(variant);
    gutil_idle_pool_add_object_ref(test.pool, object);
    g_object_unref(object);
    gutil_idle_pool_add_bytes_ref(test.pool, bytes);
    g_bytes_unref(bytes);
    gutil_idle_pool_ref(test.pool);
    gutil_idle_pool_add(test.pool, &test, test_basic_unref_pool);
    gutil_idle_pool_add(test.pool, loop, test_done);
    g_main_loop_run(loop);
    gutil_idle_pool_add(test.pool, &test, test_basic_add_during_drain);
    gutil_idle_pool_destroy(test.pool);

    g_assert(test.ok);
    g_assert(!test_object_count);
    g_main_loop_unref(loop);
}

/*==========================================================================*
 * Shared
 *==========================================================================*/

static
void
test_shared(
    void)
{
    GUtilIdlePool* shared = NULL;
    GUtilIdlePool* pool = gutil_idle_pool_get(&shared);
    GUtilIdlePool* pool2 = gutil_idle_pool_get(NULL);
    GMainLoop* loop = g_main_loop_new(NULL, TRUE);

    g_assert(shared == pool);
    g_assert(gutil_idle_pool_get(&shared) == pool);
    g_assert(pool2 != pool);

    if (!(test_opt.flags & TEST_FLAG_DEBUG)) {
        g_timeout_add_seconds(TEST_TIMEOUT, test_timeout, loop);
    }

    /* The pool invokes the callback and destroys itself */
    gutil_idle_pool_add(pool, loop, test_done);
    g_main_loop_run(loop);
    g_assert(!shared);
    gutil_idle_pool_unref(pool2);
    g_main_loop_unref(loop);
}

/*==========================================================================*
 * Common
 *==========================================================================*/

#define TEST_(test) "/idlepool/" test

int main(int argc, char* argv[])
{
    G_GNUC_BEGIN_IGNORE_DEPRECATIONS;
    g_type_init();
    G_GNUC_END_IGNORE_DEPRECATIONS;
    g_test_init(&argc, &argv, NULL);
    g_test_add_func(TEST_("null"), test_null);
    g_test_add_func(TEST_("basic"), test_basic);
    g_test_add_func(TEST_("shared"), test_shared);
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
