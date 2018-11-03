/*
 * Copyright (C) 2017-2018 Jolla Ltd.
 * Copyright (C) 2017-2018 Slava Monich <slava.monich@jolla.com>
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

#include "gutil_strv.h"
#include "gutil_log.h"

static TestOpt test_opt;

/*==========================================================================*
 * Basic
 *==========================================================================*/

static GString* test_log_basic_buf;

static
void
test_log_basic_fn(
    const char* name,
    int level,
    const char* format,
    va_list va)
{
    g_string_append_vprintf(test_log_basic_buf, format, va);
}

static
void
test_log_basic(
    void)
{
    const GLogProc fn = gutil_log_func;
    const GLogProc2 fn2 = gutil_log_func2;
    const int level = gutil_log_default.level;
    GLOG_MODULE_DEFINE2_(module, "test", gutil_log_default);
    test_log_basic_buf = g_string_new(NULL);
    gutil_log_func = test_log_basic_fn;

    module.level = GLOG_LEVEL_INHERIT;
    gutil_log_default.level = GLOG_LEVEL_ERR;
    gutil_log(NULL, GLOG_LEVEL_NONE, "Debug!");
    gutil_log(NULL, GLOG_LEVEL_DEBUG, "Debug!");
    gutil_log(&module, GLOG_LEVEL_DEBUG, "Debug!");
    gutil_log_assert(NULL, GLOG_LEVEL_WARN, "Test!", __FILE__, __LINE__);
    g_assert(!test_log_basic_buf->len);
    gutil_log(&module, GLOG_LEVEL_ERR, "Err!");
    g_assert(test_log_basic_buf->len);
    g_string_set_size(test_log_basic_buf, 0);

    /* With NULL parent, still gutil_log_default is going to be checked */
    module.parent = NULL;
    gutil_log(NULL, GLOG_LEVEL_NONE, "Debug!");
    gutil_log(NULL, GLOG_LEVEL_DEBUG, "Debug!");
    gutil_log(&module, GLOG_LEVEL_DEBUG, "Debug!");
    gutil_log_assert(NULL, GLOG_LEVEL_WARN, "Test!", __FILE__, __LINE__);
    g_assert(!test_log_basic_buf->len);
    gutil_log(&module, GLOG_LEVEL_ERR, "Err!");
    g_assert(test_log_basic_buf->len);
    g_string_set_size(test_log_basic_buf, 0);

    gutil_log(&module, GLOG_LEVEL_ALWAYS, "Always!");
    g_assert(test_log_basic_buf->len);
    g_string_set_size(test_log_basic_buf, 0);

    /* Test GLOG_FLAG_DISABLE */
    module.flags |= GLOG_FLAG_DISABLE;
    gutil_log(&module, GLOG_LEVEL_ALWAYS, "Always!");
    g_assert(!test_log_basic_buf->len);
    module.flags &= ~GLOG_FLAG_DISABLE;

    /* Without log functions these calls have no effect */
    gutil_log_func = NULL;
    gutil_log(NULL, GLOG_LEVEL_ALWAYS, "Always!");
    gutil_log_func2 = NULL;
    gutil_log(NULL, GLOG_LEVEL_ALWAYS, "Always!");

    g_string_free(test_log_basic_buf, TRUE);
    test_log_basic_buf = NULL;
    gutil_log_default.level = level;
    gutil_log_func = fn;
    gutil_log_func2 = fn2;
}

/*==========================================================================*
 * Enabled
 *==========================================================================*/

static
void
test_log_enabled(
    void)
{
    const GLogProc2 fn = gutil_log_func2;
    const int level = gutil_log_default.level;
    GLOG_MODULE_DEFINE2_(module, "test", gutil_log_default);

    gutil_log_default.level = GLOG_LEVEL_NONE;
    g_assert(gutil_log_enabled(NULL, GLOG_LEVEL_ALWAYS));
    g_assert(!gutil_log_enabled(NULL, GLOG_LEVEL_ERR));
    g_assert(!gutil_log_enabled(NULL, GLOG_LEVEL_NONE));

    gutil_log_default.level = GLOG_LEVEL_INFO;
    g_assert(gutil_log_enabled(&gutil_log_default, GLOG_LEVEL_ALWAYS));
    g_assert(gutil_log_enabled(&gutil_log_default, GLOG_LEVEL_INFO));
    gutil_log_default.flags |= GLOG_FLAG_DISABLE;
    g_assert(!gutil_log_enabled(&gutil_log_default, GLOG_LEVEL_INFO));
    gutil_log_default.flags &= ~GLOG_FLAG_DISABLE;
    g_assert(!gutil_log_enabled(&gutil_log_default, GLOG_LEVEL_DEBUG));

    /* It makes no sense to have default as INHERIT so it's treated as NONE */
    gutil_log_default.level = GLOG_LEVEL_INHERIT;
    g_assert(!gutil_log_enabled(&module, GLOG_LEVEL_ERR));
    g_assert(gutil_log_enabled(&module, GLOG_LEVEL_ALWAYS));

    /* Test parenting */
    module.level = GLOG_LEVEL_INHERIT;
    gutil_log_default.level = GLOG_LEVEL_INFO;
    g_assert(gutil_log_enabled(&module, GLOG_LEVEL_ALWAYS));
    g_assert(gutil_log_enabled(&module, GLOG_LEVEL_INFO));
    g_assert(!gutil_log_enabled(&module, GLOG_LEVEL_DEBUG));

    /* No handler = no logging */
    gutil_log_func2 = NULL;
    gutil_log_default.level = GLOG_LEVEL_VERBOSE;
    g_assert(!gutil_log_enabled(NULL, GLOG_LEVEL_ALWAYS));

    gutil_log_default.level = level;
    gutil_log_func2 = fn;
}

/*==========================================================================*
 * Misc
 *==========================================================================*/

static
void
test_log_misc(
    void)
{
    const GLogProc fn = gutil_log_func;
    g_assert(gutil_log_set_type(GLOG_TYPE_STDOUT, "test"));
    g_assert(gutil_log_func == gutil_log_stdout);
    g_assert(!g_strcmp0(gutil_log_get_type(), GLOG_TYPE_STDOUT));
    g_assert(gutil_log_set_type(GLOG_TYPE_STDERR, "test"));
    g_assert(gutil_log_func == gutil_log_stderr);
    g_assert(!g_strcmp0(gutil_log_get_type(), GLOG_TYPE_STDERR));
    g_assert(!gutil_log_set_type("whatever", "test"));
    gutil_log_func = NULL;
    g_assert(!g_strcmp0(gutil_log_get_type(), GLOG_TYPE_CUSTOM));
    gutil_log_func = fn;
}

/*==========================================================================*
 * Common
 *==========================================================================*/

#define TEST_PREFIX "/log/"

int main(int argc, char* argv[])
{
    g_test_init(&argc, &argv, NULL);
    g_test_add_func(TEST_PREFIX "basic", test_log_basic);
    g_test_add_func(TEST_PREFIX "enabled", test_log_enabled);
    g_test_add_func(TEST_PREFIX "misc", test_log_misc);
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
