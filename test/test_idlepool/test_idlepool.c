/*
 * Copyright (C) 2016-2024 Slava Monich <slava@monich.com>
 * Copyright (C) 2016-2018 Jolla Ltd.
 *
 * You may use this file under the terms of the BSD license as follows:
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

#include "gutil_idlepool.h"
#include "gutil_strv.h"
#include "gutil_log.h"

#define TEST_TIMEOUT_SEC (10) /* seconds */

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

static
gboolean
test_done_cb(
    gpointer loop)
{
    g_main_loop_quit(loop);
    return G_SOURCE_REMOVE;
}

/*==========================================================================*
 * Test thread
 *==========================================================================*/

typedef struct test_thread TestThread;

typedef
void
(*ThreadBeforeProc)(
    TestThread* thread,
    gpointer user_data);

struct test_thread {
    GThread* thread;
    GMutex mutex;
    GCond start_cond;
    GMainLoop* loop;
    GMainContext* context;
    ThreadBeforeProc before_proc;
    gpointer before_data;
};

static
gpointer
test_thread_proc(
    gpointer user_data)
{
    TestThread* thread = user_data;
    GMainLoop* loop = g_main_loop_ref(thread->loop);
    GMainContext* context = g_main_context_ref(thread->context);

    /* Invoke this callback before pushing GMainContext */
    if (thread->before_proc) {
        thread->before_proc(thread, thread->before_data);
    }

    /* Lock */
    g_mutex_lock(&thread->mutex);
    g_main_context_push_thread_default(context);
    g_cond_broadcast(&thread->start_cond);
    GDEBUG("Test thread started");
    g_mutex_unlock(&thread->mutex);
    /* Unlock */

    g_main_loop_run(loop);
    g_main_loop_unref(loop);
    g_main_context_pop_thread_default(context);
    g_main_context_unref(context);
    GDEBUG("Test thread exiting");
    return NULL;
}

static
TestThread*
test_thread_new(
    ThreadBeforeProc before_proc,
    gpointer before_data)
{
    TestThread* thread = g_new0(TestThread, 1);

    g_cond_init(&thread->start_cond);
    g_mutex_init(&thread->mutex);
    thread->context = g_main_context_new();
    thread->loop = g_main_loop_new(thread->context, FALSE);
    thread->before_proc = before_proc;
    thread->before_data = before_data;

    /* Lock */
    g_mutex_lock(&thread->mutex);
    thread->thread = g_thread_new("test", test_thread_proc, thread);
    g_assert(g_cond_wait_until(&thread->start_cond, &thread->mutex,
        g_get_monotonic_time() + TEST_TIMEOUT_SEC * G_TIME_SPAN_SECOND));
    g_mutex_unlock(&thread->mutex);
    /* Unlock */

    return thread;
}

static
void
test_thread_invoke_later(
    TestThread* thread,
    GSourceFunc fn,
    gpointer user_data)
{
    GSource* src = g_idle_source_new();

    g_source_set_priority(src, G_PRIORITY_DEFAULT);
    g_source_set_callback(src, fn, user_data, NULL);
    g_source_attach(src, thread->context);
    g_source_unref(src);
}

static
void
test_thread_free(
    TestThread* thread)
{
    g_main_loop_quit(thread->loop);
    g_thread_join(thread->thread);
    g_main_loop_unref(thread->loop);
    g_main_context_unref(thread->context);
    g_cond_clear(&thread->start_cond);
    g_mutex_clear(&thread->mutex);
    g_free(thread);
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
    gutil_idle_pool_destroy(NULL);
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
        g_timeout_add_seconds(TEST_TIMEOUT_SEC, test_timeout, loop);
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
        g_timeout_add_seconds(TEST_TIMEOUT_SEC, test_timeout, loop);
    }

    /* The pool invokes the callback and destroys itself */
    gutil_idle_pool_add(pool, loop, test_done);
    g_main_loop_run(loop);
    g_assert(!shared);
    gutil_idle_pool_unref(pool2);
    g_main_loop_unref(loop);
}

/*==========================================================================*
 * Default
 *==========================================================================*/

typedef struct test_default_data {
    GMainLoop* loop;
    TestThread* thread;
    int step;
} TestDefault;

static
void
test_default_1(
    gpointer user_data)
{
    TestDefault* test = user_data;

    g_assert(g_main_context_get_thread_default());
    g_assert_cmpint(test->step, == ,1);
    test->step++;
}

static
void
test_default_2(
    gpointer user_data)
{
    TestDefault* test = user_data;

    /* This function is invoked on the test thread */
    g_assert(test->thread->thread == g_thread_self());
    g_assert_cmpint(test->step, == ,2);
    test->step++;
    g_idle_add(test_done_cb, test->loop);
}

static
gboolean
test_default_proc(
    gpointer user_data)
{
    TestDefault* test = user_data;
    GUtilIdlePool* pool = gutil_idle_pool_get_default();

    /* This function is invoked on the test thread */
    g_assert(test->thread->thread == g_thread_self());
    g_assert(pool == gutil_idle_pool_get_default());
    gutil_idle_pool_add(pool, test, test_default_2);
    return G_SOURCE_REMOVE;
}

static
gboolean
test_default_main(
    gpointer user_data)
{
    TestDefault* test = user_data;

    /*
     * This function is invoked on the main thread.
     * test_default_proc will add an item to the thread specific pool
     * on the test thread, which will schedule gutil_idle_pool_idle
     * in the right context and eventually invoke test_default_1 and
     * test_default_2 on the test thread in the right order.
     */
    test_thread_invoke_later(test->thread, test_default_proc, test);
    return G_SOURCE_REMOVE;
}

static
void
test_default_start(
    TestThread* thread,
    gpointer user_data)
{
    TestDefault* test = user_data;
    GUtilIdlePool* pool = gutil_idle_pool_get_default();

    g_assert_cmpint(test->step, == ,0);
    test->step++;
    g_assert(!g_main_context_get_thread_default());
    g_assert(pool == gutil_idle_pool_get_default());

    /* This schedules gutil_idle_pool_idle on the main thread */
    gutil_idle_pool_add(NULL /* use default */, test, test_default_1);

    /*
     * This will invoke test_default_main on the main thread after
     * gutil_idle_pool_idle
     */
    g_idle_add(test_default_main, test);
}

static
void
test_default(
    void)
{
    TestDefault test;

    memset(&test, 0, sizeof(test));
    test.loop = g_main_loop_new(NULL, TRUE);
    test.thread = test_thread_new(test_default_start, &test);

    if (!(test_opt.flags & TEST_FLAG_DEBUG)) {
        g_timeout_add_seconds(TEST_TIMEOUT_SEC, test_timeout, test.loop);
    }

    g_main_loop_run(test.loop);
    test_thread_free(test.thread);
    g_assert_cmpint(test.step, == ,3);
    g_main_loop_unref(test.loop);
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
    g_test_add_func(TEST_("default"), test_default);
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
