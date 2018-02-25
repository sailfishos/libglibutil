/*
 * Copyright (C) 2016-2018 Jolla Ltd.
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

#include "test_common.h"

#include "gutil_idlepool.h"
#include "gutil_log.h"

#define TEST_TIMEOUT (10) /* seconds */

static TestOpt test_opt;

/*==========================================================================*
 * Basic
 *==========================================================================*/

typedef struct test_basic {
    GMainLoop* loop;
    GUtilIdlePool* pool;
    guint timeout_id;
    gboolean array_free_count;
    gboolean ok;
} TestBasic;

static
void
test_basic_done(
    gpointer param)
{
    TestBasic* test = param;
    g_main_loop_quit(test->loop);
}

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
gboolean
test_basic_timeout(
    gpointer param)
{
    TestBasic* test = param;
    GERR("TIMEOUT");
    test->timeout_id = 0;
    g_main_loop_quit(test->loop);
    return G_SOURCE_REMOVE;
}

static
void
test_basic(
    void)
{
    GPtrArray* array = g_ptr_array_new_with_free_func(test_basic_array_free);
    GVariant* variant = g_variant_take_ref(g_variant_new_int32(1));
    GObject* object = g_object_new(TEST_OBJECT_TYPE, NULL);
    TestBasic test;

    memset(&test, 0, sizeof(test));
    test.loop = g_main_loop_new(NULL, TRUE);
    test.pool = gutil_idle_pool_new();

    if (!(test_opt.flags & TEST_FLAG_DEBUG)) {
        test.timeout_id = g_timeout_add_seconds(TEST_TIMEOUT,
            test_basic_timeout, &test);
    }

    /* These have no effect, just testing NULL-telerance */
    gutil_idle_pool_unref(gutil_idle_pool_new());
    gutil_idle_pool_add_ptr_array(NULL, array);
    gutil_idle_pool_add_ptr_array(test.pool, NULL);
    gutil_idle_pool_add_ptr_array_ref(NULL, array);
    gutil_idle_pool_add_ptr_array_ref(test.pool, NULL);
    gutil_idle_pool_add_variant(NULL, variant);
    gutil_idle_pool_add_variant(test.pool, NULL);
    gutil_idle_pool_add_variant_ref(NULL, variant);
    gutil_idle_pool_add_variant_ref(test.pool, NULL);
    gutil_idle_pool_add_object(NULL, object);
    gutil_idle_pool_add_object(test.pool, NULL);
    gutil_idle_pool_add_object_ref(NULL, object);
    gutil_idle_pool_add_object_ref(test.pool, NULL);
    gutil_idle_pool_add(NULL, NULL, test_basic_unref_pool);
    gutil_idle_pool_add(test.pool, NULL, NULL);
    gutil_idle_pool_ref(NULL);
    gutil_idle_pool_unref(NULL);
    gutil_idle_pool_drain(NULL);

    g_ptr_array_add(array, &test);
    gutil_idle_pool_add_ptr_array_ref(test.pool, array);
    g_ptr_array_unref(array);
    gutil_idle_pool_add_variant_ref(test.pool, variant);
    g_variant_unref(variant);
    gutil_idle_pool_add_object_ref(test.pool, object);
    g_object_unref(object);
    gutil_idle_pool_ref(test.pool);
    gutil_idle_pool_add(test.pool, &test, test_basic_unref_pool);
    gutil_idle_pool_add(test.pool, &test, test_basic_done);
    g_main_loop_run(test.loop);
    gutil_idle_pool_add(test.pool, &test, test_basic_add_during_drain);
    gutil_idle_pool_unref(test.pool);

    g_assert(test.ok);
    g_assert(!test_object_count);
    if (!(test_opt.flags & TEST_FLAG_DEBUG)) {
        g_assert(test.timeout_id);
        g_source_remove(test.timeout_id);
    }

    g_main_loop_unref(test.loop);
}

/*==========================================================================*
 * Common
 *==========================================================================*/

#define TEST_PREFIX "/idlepool/"

int main(int argc, char* argv[])
{
    G_GNUC_BEGIN_IGNORE_DEPRECATIONS;
    g_type_init();
    G_GNUC_END_IGNORE_DEPRECATIONS;
    g_test_init(&argc, &argv, NULL);
    g_test_add_func(TEST_PREFIX "basic", test_basic);
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
