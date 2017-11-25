/*
 * Copyright (C) 2017 Jolla Ltd.
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

#include "gutil_idlequeue.h"
#include "gutil_log.h"

#define TEST_TIMEOUT (10) /* seconds */

static TestOpt test_opt;

static
void
test_idlequeue_noooo(
    gpointer param)
{
    g_assert(!"NOOO!!!");
}

static
gboolean
test_idlequeue_timeout(
    gpointer param)
{
    g_assert(!"TIMEOUT");
    return G_SOURCE_REMOVE;
}

static
void
test_idlequeue_loop_quit(
    gpointer loop)
{
    g_main_loop_quit(loop);
}

static
void
test_idlequeue_int_inc(
    gpointer data)
{
    int* ptr = data;
    (*ptr)++;
}

/*==========================================================================*
 * Null
 *==========================================================================*/

static
void
test_idlequeue_null(
    void)
{
    /* Test NULL tolerance */
    int i = 0;
    g_assert(!gutil_idle_queue_ref(NULL));
    gutil_idle_queue_unref(NULL);
    gutil_idle_queue_free(NULL);
    g_assert(!gutil_idle_queue_contains_tag(NULL, 0));
    g_assert(!gutil_idle_queue_cancel_tag(NULL, 0));
    gutil_idle_queue_cancel_all(NULL);
    gutil_idle_queue_add(NULL, NULL, NULL);
    gutil_idle_queue_add_full(NULL, NULL, NULL, NULL);
    gutil_idle_queue_add_tag(NULL, 0, NULL, NULL);
    gutil_idle_queue_add_tag_full(NULL, 0, NULL, NULL, NULL);
    /* IF queue is NULL, the destroy function gets invoked immediately: */
    gutil_idle_queue_add_tag_full(NULL, 0, NULL, &i, test_idlequeue_int_inc);
    g_assert(i == 1);
}

/*==========================================================================*
 * Basic
 *==========================================================================*/

static
void
test_idlequeue_basic(
    void)
{
    guint timeout_id = 0;
    int count = 0;
    GUtilIdleQueue* q = gutil_idle_queue_new();
    GMainLoop* loop = g_main_loop_new(NULL, TRUE);

    gutil_idle_queue_unref(gutil_idle_queue_ref(q));
    gutil_idle_queue_add_tag_full(q, 1, test_idlequeue_int_inc, &count, NULL);
    gutil_idle_queue_add_tag_full(q, 2, NULL, &count, test_idlequeue_int_inc);
    gutil_idle_queue_add_tag_full(q, 3, test_idlequeue_loop_quit, loop, NULL);
    g_assert(!gutil_idle_queue_contains_tag(q, 0));
    g_assert(gutil_idle_queue_contains_tag(q, 1));
    g_assert(gutil_idle_queue_contains_tag(q, 2));
    g_assert(gutil_idle_queue_contains_tag(q, 3));

    if (!(test_opt.flags & TEST_FLAG_DEBUG)) {
        timeout_id = g_timeout_add_seconds(TEST_TIMEOUT,
            test_idlequeue_timeout, NULL);
    }

    g_main_loop_run(loop);
    g_assert(count == 2);

    if (timeout_id) {
        g_source_remove(timeout_id);
    }

    gutil_idle_queue_unref(q);
    g_main_loop_unref(loop);
}
/*==========================================================================*
 * Add
 *==========================================================================*/

typedef struct test_idlequeue_add_data {
    GUtilIdleQueue* q;
    GMainLoop* loop;
} TestAdd;

static
void
test_idlequeue_add_cb(
    gpointer data)
{
    /* Adding new item from the callback */
    TestAdd* test = data;
    gutil_idle_queue_add(test->q, test_idlequeue_loop_quit, test->loop);
}

static
void
test_idlequeue_add(
    void)
{
    guint timeout_id = 0;
    TestAdd test;

    test.q = gutil_idle_queue_new();
    test.loop = g_main_loop_new(NULL, TRUE);

    if (!(test_opt.flags & TEST_FLAG_DEBUG)) {
        timeout_id = g_timeout_add_seconds(TEST_TIMEOUT,
            test_idlequeue_timeout, NULL);
    }

    gutil_idle_queue_add(test.q, test_idlequeue_add_cb, &test);
    g_main_loop_run(test.loop);

    if (timeout_id) {
        g_source_remove(timeout_id);
    }

    gutil_idle_queue_unref(test.q);
    g_main_loop_unref(test.loop);
}

/*==========================================================================*
 * Cancel
 *==========================================================================*/

static
void
test_idlequeue_cancel(
    void)
{
    int count = 0;
    GUtilIdleQueue* q = gutil_idle_queue_new();

    /* Destroying the queue cancels the callbacks */
    gutil_idle_queue_add(q, test_idlequeue_int_inc, &count);
    gutil_idle_queue_add_full(q, NULL, &count, test_idlequeue_int_inc);
    gutil_idle_queue_add_full(q, NULL, &count, test_idlequeue_int_inc);
    gutil_idle_queue_unref(q);
    g_assert(count == 2);
    count = 0;

    /* Cancelling all requests in an empty queue has no effect */
    q = gutil_idle_queue_new();
    gutil_idle_queue_cancel_all(q);

    /* Cancel by tag */
    gutil_idle_queue_add_tag_full(q, 1, NULL, &count, test_idlequeue_int_inc);
    gutil_idle_queue_add_tag_full(q, 2, NULL, &count, test_idlequeue_int_inc);
    gutil_idle_queue_add_tag_full(q, 3, NULL, &count, test_idlequeue_int_inc);
    gutil_idle_queue_add_tag_full(q, 4, NULL, &count, test_idlequeue_int_inc);
    g_assert(!gutil_idle_queue_cancel_tag(q, 0));
    g_assert(gutil_idle_queue_cancel_tag(q, 3));
    g_assert(gutil_idle_queue_cancel_tag(q, 4));
    g_assert(gutil_idle_queue_cancel_tag(q, 1));
    g_assert(gutil_idle_queue_cancel_tag(q, 2));
    g_assert(!gutil_idle_queue_cancel_tag(q, 1));
    g_assert(count == 4);
    count = 0;

    gutil_idle_queue_free(q);
}

/*==========================================================================*
 * CancelAll
 *==========================================================================*/

static
void
test_idlequeue_cancel_all_submit_new(
    gpointer q)
{
    /* Adding new item from the destroy callback */
    gutil_idle_queue_add_tag(q, 42, test_idlequeue_noooo, NULL);
}

static
void
test_idlequeue_cancel_all(
    void)
{
    int count = 0;
    GUtilIdleQueue* q = gutil_idle_queue_new();

    /* Add new item from the destroy callback */
    gutil_idle_queue_add(q, test_idlequeue_int_inc, &count);
    gutil_idle_queue_add_tag_full(q, 1, NULL, &count, test_idlequeue_int_inc);
    gutil_idle_queue_add_tag_full(q, 2, NULL, q,
        test_idlequeue_cancel_all_submit_new);
    gutil_idle_queue_cancel_all(q);
    g_assert(count == 1);

    /* We should still have 42 in there */
    g_assert(!gutil_idle_queue_contains_tag(q, 1));
    g_assert(!gutil_idle_queue_contains_tag(q, 2));
    g_assert(gutil_idle_queue_contains_tag(q, 42));
    gutil_idle_queue_cancel_all(q);

    /* Now it has to be really empty */
    g_assert(!gutil_idle_queue_contains_tag(q, 42));
    gutil_idle_queue_free(q);
}

/*==========================================================================*
 * Common
 *==========================================================================*/

#define TEST_PREFIX "/idlequeue/"

int main(int argc, char* argv[])
{
    G_GNUC_BEGIN_IGNORE_DEPRECATIONS;
    g_type_init();
    G_GNUC_END_IGNORE_DEPRECATIONS;
    g_test_init(&argc, &argv, NULL);
    g_test_add_func(TEST_PREFIX "null", test_idlequeue_null);
    g_test_add_func(TEST_PREFIX "basic", test_idlequeue_basic);
    g_test_add_func(TEST_PREFIX "add", test_idlequeue_add);
    g_test_add_func(TEST_PREFIX "cancel", test_idlequeue_cancel);
    g_test_add_func(TEST_PREFIX "cancel_all", test_idlequeue_cancel_all);
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
