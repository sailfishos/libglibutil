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

#include "test_common.h"

#include <gutil_log.h>

typedef struct test_app {
    const void* tests;
    gsize test_size;
    guint flags;
    int test_count;
} TestApp;

#define test_at(app,i) \
    ((const TestDesc*)(((guint8*)(app)->tests) + i * (app)->test_size))

static
int
test_main_run_one(
    TestApp* app,
    const TestDesc* test)
{
    int ret = test->run(test, app->flags);
    gutil_log(GLOG_MODULE_CURRENT, GLOG_LEVEL_ALWAYS, "%s: %s",
        (ret == RET_OK) ? "OK" : "FAILED", test->name);
    return ret;
}

static
int
test_main_run(
    TestApp* app,
    const char* name)
{
    int i, ret;
    if (name) {
        const TestDesc* found = NULL;
        for (i=0, ret = RET_NOTFOUND; i<app->test_count; i++) {
            const TestDesc* test = test_at(app, i);
            if (!strcmp(test->name, name)) {
                ret = test_main_run_one(app, test);
                found = test;
                break;
            }
        }
        if (!found) GERR("No such test: %s", name);
    } else {
        for (i=0, ret = RET_OK; i<app->test_count; i++) {
            int test_status = test_main_run_one(app, test_at(app, i));
            if (ret == RET_OK && test_status != RET_OK) {
                ret = test_status;
            }
        }
    }
    return ret;
}

int
test_main(
    int argc,
    char* argv[],
    const void* tests,
    gsize test_size,
    int test_count,
    guint flags)
{
    int ret = RET_ERR;
    gboolean debug = FALSE;
    gboolean verbose = FALSE;
    GError* error = NULL;
    GOptionContext* options;
    const GOptionEntry debug_entry = {
        "debug", 'd', 0, G_OPTION_ARG_NONE, &debug,
        "Disable timeout for debugging", NULL
    };
    GOptionEntry entries[] = {
        { "verbose", 'v', 0, G_OPTION_ARG_NONE, &verbose,
          "Enable verbose output", NULL },
        { NULL /* debug */},
        { NULL }
    };
    /* g_type_init has been deprecated since version 2.36
     * the type system is initialised automagically since then */
    G_GNUC_BEGIN_IGNORE_DEPRECATIONS;
    g_type_init();
    G_GNUC_END_IGNORE_DEPRECATIONS;
    if (flags & TEST_FLAG_DEBUG) {
        entries[1] = debug_entry;
    }
    options = g_option_context_new("[TEST]");
    g_option_context_add_main_entries(options, entries, NULL);
    if (g_option_context_parse(options, &argc, &argv, &error)) {
        TestApp app;
        memset(&app, 0, sizeof(app));
        app.tests = tests;
        app.test_size = test_size;
        app.test_count = test_count;
        if (debug) {
            app.flags |= TEST_FLAG_DEBUG;
        }
        gutil_log_timestamp = FALSE;
        gutil_log_default.level = verbose ? GLOG_LEVEL_VERBOSE :
            GLOG_LEVEL_NONE;
        if (argc < 2) {
            ret = test_main_run(&app, NULL);
        } else {
            int i;
            for (i=1, ret = RET_OK; i<argc; i++) {
                int test_status = test_main_run(&app, argv[i]);
                if (ret == RET_OK && test_status != RET_OK) {
                    ret = test_status;
                }
            }
        }
    } else {
        fprintf(stderr, "%s\n", GERRMSG(error));
        g_error_free(error);
        ret = RET_CMDLINE;
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
