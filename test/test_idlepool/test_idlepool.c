/*
 * Copyright (C) 2016 Jolla Ltd.
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

#include "gutil_idlepool.h"
#include "gutil_log.h"

#define RET_OK       (0)
#define RET_ERR      (1)
#define RET_TIMEOUT  (2)

#define TEST_TIMEOUT (10) /* seconds */

typedef struct test_desc {
    const char* name;
    int (*run)(GMainLoop* loop);
} TestDesc;

/*==========================================================================*
 * Basic
 *==========================================================================*/

typedef struct test_basic {
    GMainLoop* loop;
    GUtilIdlePool* pool;
    gboolean array_free_count;
    int ret;
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
        test->ret = RET_OK;
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

int
test_basic(GMainLoop* loop)
{
    GPtrArray* array = g_ptr_array_new_with_free_func(test_basic_array_free);
    GVariant* variant = g_variant_take_ref(g_variant_new_int32(1));
    GObject* object = g_object_new(G_TYPE_OBJECT, NULL);
    TestBasic test;

    memset(&test, 0, sizeof(test));
    test.loop = loop;
    test.pool = gutil_idle_pool_new();
    test.ret = RET_ERR;

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
    g_main_loop_run(loop);
    gutil_idle_pool_add(test.pool, &test, test_basic_add_during_drain);
    gutil_idle_pool_unref(test.pool);

    return test.ret;
}

/*==========================================================================*
 * Common
 *==========================================================================*/

static const TestDesc all_tests[] = {
    {
        "Basic",
        test_basic
    }
};

typedef struct test_run_context {
    const TestDesc* desc;
    GMainLoop* loop;
    guint timeout_id;
    gboolean timeout_occured;
} TestRun;

static
gboolean
test_timer(
    gpointer param)
{
    TestRun* run = param;
    GERR("%s TIMEOUT", run->desc->name);
    run->timeout_id = 0;
    run->timeout_occured = TRUE;
    g_main_loop_quit(run->loop);
    return G_SOURCE_REMOVE;
}

static
int
test_run_once(
    const TestDesc* desc,
    gboolean debug)
{
    TestRun run;
    int ret;

    memset(&run, 0, sizeof(run));
    run.loop = g_main_loop_new(NULL, TRUE);
    run.desc = desc;
    if (!debug) {
        run.timeout_id = g_timeout_add_seconds(TEST_TIMEOUT, test_timer, &run);
    }

    ret = desc->run(run.loop);

    if (run.timeout_occured) {
        ret = RET_TIMEOUT;
    }
    if (run.timeout_id) {
        g_source_remove(run.timeout_id);
    }
    g_main_loop_unref(run.loop);
    GINFO("%s: %s", (ret == RET_OK) ? "OK" : "FAILED", desc->name);
    return ret;
}

static
int
test_run(
    const char* name,
    gboolean debug)
{
    int i, ret;
    if (name) {
        const TestDesc* found = NULL;
        for (i=0, ret = RET_ERR; i<G_N_ELEMENTS(all_tests); i++) {
            const TestDesc* test = all_tests + i;
            if (!strcmp(test->name, name)) {
                ret = test_run_once(test, debug);
                found = test;
                break;
            }
        }
        if (!found) GERR("No such test: %s", name);
    } else {
        for (i=0, ret = RET_OK; i<G_N_ELEMENTS(all_tests); i++) {
            int test_status = test_run_once(all_tests + i, debug);
            if (ret == RET_OK && test_status != RET_OK) ret = test_status;
        }
    }
    return ret;
}

int main(int argc, char* argv[])
{
    int ret = RET_ERR;
    gboolean verbose = FALSE;
    gboolean debug = FALSE;
    GError* error = NULL;
    GOptionContext* options;
    GOptionEntry entries[] = {
        { "verbose", 'v', 0, G_OPTION_ARG_NONE, &verbose,
          "Enable verbose output", NULL },
        { "debug", 'd', 0, G_OPTION_ARG_NONE, &debug,
          "Disable timeout for debugging", NULL },
        { NULL }
    };

    /* g_type_init has been deprecated since version 2.36
     * the type system is initialised automagically since then */
    G_GNUC_BEGIN_IGNORE_DEPRECATIONS;
    g_type_init();
    G_GNUC_END_IGNORE_DEPRECATIONS;

    options = g_option_context_new("[TEST]");
    g_option_context_add_main_entries(options, entries, NULL);
    if (g_option_context_parse(options, &argc, &argv, &error)) {
        gutil_log_timestamp = FALSE;
        if (verbose) {
            gutil_log_default.level = GLOG_LEVEL_VERBOSE;
        }

        if (argc < 2) {
            ret = test_run(NULL, debug);
        } else {
            int i;
            for (i=1, ret = RET_OK; i<argc; i++) {
                int test_status =  test_run(argv[i], debug);
                if (ret == RET_OK && test_status != RET_OK) ret = test_status;
            }
        }
    } else {
        fprintf(stderr, "%s\n", GERRMSG(error));
        g_error_free(error);
        ret = RET_ERR;
    }
    g_option_context_free(options);
    return ret;
}

/*
 * Local Variables:
 * mode: C
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
