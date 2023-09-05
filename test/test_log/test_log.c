/*
 * Copyright (C) 2017-2023 Slava Monich <slava@monich.com>
 * Copyright (C) 2017-2022 Jolla Ltd.
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

#define _GNU_SOURCE /* for fopencookie */

#include "test_common.h"

#include "gutil_strv.h"
#include "gutil_log.h"

#ifdef __GLIBC__
/* glibc has writeable stdout */
#  define HAVE_TEST_LOG_FILE
#endif

static TestOpt test_opt;
static GString* test_log_buf;

static
void
test_log_fn(
    const char* name,
    int level,
    const char* format,
    va_list va)
{
    g_string_append_vprintf(test_log_buf, format, va);
    g_string_append_c(test_log_buf, '\n');
}

/*==========================================================================*
 * Basic
 *==========================================================================*/

static
void
test_log_basic(
    void)
{
    const GLogProc fn = gutil_log_func;
    const GLogProc2 fn2 = gutil_log_func2;
    const int level = gutil_log_default.level;
    GLOG_MODULE_DEFINE2_(module, "test", gutil_log_default);
    test_log_buf = g_string_new(NULL);
    gutil_log_func = test_log_fn;

    module.level = GLOG_LEVEL_INHERIT;
    gutil_log_default.level = GLOG_LEVEL_ERR;
    gutil_log(NULL, GLOG_LEVEL_NONE, "Debug!");
    gutil_log(NULL, GLOG_LEVEL_DEBUG, "Debug!");
    gutil_log(&module, GLOG_LEVEL_DEBUG, "Debug!");
    gutil_log_assert(NULL, GLOG_LEVEL_WARN, "Test!", __FILE__, __LINE__);
    g_assert(!test_log_buf->len);
    gutil_log(&module, GLOG_LEVEL_ERR, "Err!");
    g_assert(test_log_buf->len);
    g_string_set_size(test_log_buf, 0);

    /* With NULL parent, still gutil_log_default is going to be checked */
    module.parent = NULL;
    gutil_log(NULL, GLOG_LEVEL_NONE, "Debug!");
    gutil_log(NULL, GLOG_LEVEL_DEBUG, "Debug!");
    gutil_log(&module, GLOG_LEVEL_DEBUG, "Debug!");
    gutil_log_assert(NULL, GLOG_LEVEL_WARN, "Test!", __FILE__, __LINE__);
    g_assert(!test_log_buf->len);
    gutil_log(&module, GLOG_LEVEL_ERR, "Err!");
    g_assert(test_log_buf->len);
    g_string_set_size(test_log_buf, 0);

    gutil_log(&module, GLOG_LEVEL_ALWAYS, "Always!");
    g_assert(test_log_buf->len);
    g_string_set_size(test_log_buf, 0);

    /* Test GLOG_FLAG_DISABLE */
    module.flags |= GLOG_FLAG_DISABLE;
    gutil_log(&module, GLOG_LEVEL_ALWAYS, "Always!");
    g_assert(!test_log_buf->len);
    module.flags &= ~GLOG_FLAG_DISABLE;

    /* Without log functions these calls have no effect */
    gutil_log_func = NULL;
    gutil_log(NULL, GLOG_LEVEL_ALWAYS, "Always!");
    gutil_log_func2 = NULL;
    gutil_log(NULL, GLOG_LEVEL_ALWAYS, "Always!");

    g_string_free(test_log_buf, TRUE);
    test_log_buf = NULL;
    gutil_log_default.level = level;
    gutil_log_func = fn;
    gutil_log_func2 = fn2;
}

/*==========================================================================*
 * File
 *==========================================================================*/

#ifdef HAVE_TEST_LOG_FILE

static
ssize_t
test_log_file_write(
    void* buf,
    const char* chars,
    size_t size)
{
    g_string_append_len(buf, chars, size);
    return size;
}

static
void
test_log_drop(
    const GLogModule* module,
    int level,
    const char* format,
    va_list va)
{
}

static
void
test_log_file(
    void)
{
    static const cookie_io_functions_t funcs = {
        .write = test_log_file_write
    };
    GString* buf = g_string_new(NULL);
    FILE* out = fopencookie(buf, "w", funcs);
    FILE* default_stdout = stdout;
    const int level = gutil_log_default.level;
    gboolean use_timestamp;
    GLogProc2 log_proc;

    g_assert(out);
    g_assert(gutil_log_set_type(GLOG_TYPE_STDOUT, NULL));
    g_assert(gutil_log_func == gutil_log_stdout);
    gutil_log_timestamp = FALSE;
    gutil_log_default.level = GLOG_LEVEL_WARN;

    /* Warning pefix */
    stdout = out;
    gutil_log(NULL, GLOG_LEVEL_WARN, "Test");
    stdout = default_stdout;
    g_assert(fflush(out) == 0);
    GDEBUG("%s", buf->str);
    g_assert_cmpstr(buf->str, == ,"WARNING: Test\n");
    g_string_set_size(buf, 0);

    /* Error prefix */
    stdout = out;
    gutil_log(NULL, GLOG_LEVEL_ERR, "Test");
    stdout = default_stdout;
    g_assert(fflush(out) == 0);
    GDEBUG("%s", buf->str);
    g_assert_cmpstr(buf->str, == ,"ERROR: Test\n");
    g_string_set_size(buf, 0);

    /* Empty name (dropped) */
    gutil_log_default.name = "";
    stdout = out;
    gutil_log(NULL, GLOG_LEVEL_ALWAYS, "Test");
    stdout = default_stdout;
    g_assert(fflush(out) == 0);
    GDEBUG("%s", buf->str);
    g_assert_cmpstr(buf->str, == ,"Test\n");
    g_string_set_size(buf, 0);

    /* Non-empty name */
    gutil_log_default.name = "test";
    stdout = out;
    gutil_log(NULL, GLOG_LEVEL_ALWAYS, "Test");
    stdout = default_stdout;
    g_assert(fflush(out) == 0);
    GDEBUG("%s", buf->str);
    g_assert_cmpstr(buf->str, == ,"[test] Test\n");
    g_string_set_size(buf, 0);

    /* Hide the name */
    gutil_log_default.flags |= GLOG_FLAG_HIDE_NAME;
    stdout = out;
    gutil_log(NULL, GLOG_LEVEL_ALWAYS, "Test");
    stdout = default_stdout;
    g_assert(fflush(out) == 0);
    GDEBUG("%s", buf->str);
    g_assert_cmpstr(buf->str, == ,"Test\n");
    g_string_set_size(buf, 0);

    /* Timestamp prefix */
    use_timestamp = gutil_log_timestamp;
    gutil_log_timestamp = TRUE;
    stdout = out;
    gutil_log_set_timestamp_format("timestamp1 ");
    gutil_log_set_timestamp_format("timestamp1 ");
    gutil_log(NULL, GLOG_LEVEL_ALWAYS, "test1");
    stdout = default_stdout;
    g_assert(fflush(out) == 0);
    GDEBUG("%s", buf->str);
    g_assert_cmpstr(buf->str, == ,"timestamp1 test1\n");
    g_string_set_size(buf, 0);

    stdout = out;
    gutil_log_set_timestamp_format("timestamp2 ");
    gutil_log(NULL, GLOG_LEVEL_ALWAYS, "test2");
    stdout = default_stdout;
    g_assert(fflush(out) == 0);
    GDEBUG("%s", buf->str);
    g_assert_cmpstr(buf->str, == ,"timestamp2 test2\n");
    g_string_set_size(buf, 0);

    stdout = out;
    gutil_log_set_timestamp_format("");
    gutil_log(NULL, GLOG_LEVEL_ALWAYS, "test");
    stdout = default_stdout;
    g_assert(fflush(out) == 0);
    GDEBUG("%s", buf->str);
    g_assert_cmpstr(buf->str, == ,"test\n");
    g_string_set_size(buf, 0);

    gutil_log_set_timestamp_format(NULL);
    gutil_log_set_timestamp_format(NULL);
    gutil_log_timestamp = use_timestamp;

    /* Forward output to test_log_drop */
    log_proc = gutil_log_default.log_proc;
    gutil_log_default.log_proc = test_log_drop;
    gutil_log(&gutil_log_default, GLOG_LEVEL_ALWAYS, "Test");
    g_assert(!buf->len);  /* Dropped by test_log_drop */
    gutil_log_default.log_proc = log_proc;

    fclose(out);
    gutil_log_default.level = level;
    g_string_free(buf, TRUE);
}

#endif /* HAVE_TEST_LOG_FILE */

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
    g_assert_cmpstr(gutil_log_get_type(), == ,GLOG_TYPE_STDOUT);
    g_assert(gutil_log_set_type(GLOG_TYPE_STDERR, "test"));
    g_assert(gutil_log_func == gutil_log_stderr);
    g_assert_cmpstr(gutil_log_get_type(), == ,GLOG_TYPE_STDERR);
    g_assert(!gutil_log_set_type("whatever", "test"));
    gutil_log_func = NULL;
    g_assert_cmpstr(gutil_log_get_type(), == ,GLOG_TYPE_CUSTOM);
    gutil_log_func = fn;
}

/*==========================================================================*
 * Dump
 *==========================================================================*/

static
void
test_log_dump(
    void)
{
    static const guint8 short_data[] = { 0x01, 0x02, 0x03, 0x04 };
    static const char short_data_dump[] =
        "  0000: 01 02 03 04                                         ....\n";
    static const guint8 long_data[] = {
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
        0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
        0x00
    };
    static const char long_data_dump[] =
        "0000: 30 31 32 33 34 35 36 37  38 39 3a 3b 3c 3d 3e 3f    "
        "01234567 89:;<=>?\n"
        "0010: 00                                                  "
        ".\n";

    const GLogProc fn = gutil_log_func;
    const GLogModule* log = &gutil_log_default;
    GBytes* bytes;

    test_log_buf = g_string_new(NULL);
    gutil_log_func = test_log_fn;

    gutil_log_dump_bytes(log, GLOG_LEVEL_NONE, "  ", NULL);
    gutil_log_dump(log,GLOG_LEVEL_NONE,"  ",TEST_ARRAY_AND_SIZE(short_data));
    g_assert_cmpuint(test_log_buf->len, == ,0);

    gutil_log_dump(log,GLOG_LEVEL_ALWAYS,"  ",TEST_ARRAY_AND_SIZE(short_data));
    g_assert_cmpstr(test_log_buf->str, == ,short_data_dump);

    g_string_set_size(test_log_buf, 0);
    bytes = g_bytes_new_static(TEST_ARRAY_AND_SIZE(short_data));
    gutil_log_dump_bytes(log, GLOG_LEVEL_NONE, "  ", bytes);
    g_assert_cmpuint(test_log_buf->len, == ,0);
    gutil_log_dump_bytes(log, GLOG_LEVEL_ALWAYS, "  ", bytes);
    g_assert_cmpstr(test_log_buf->str, == ,short_data_dump);
    g_bytes_unref(bytes);

    g_string_set_size(test_log_buf, 0);
    gutil_log_dump(log,GLOG_LEVEL_ALWAYS,NULL,TEST_ARRAY_AND_SIZE(long_data));
    g_assert_cmpstr(test_log_buf->str, == ,long_data_dump);

    g_string_set_size(test_log_buf, 0);
    bytes = g_bytes_new_static(TEST_ARRAY_AND_SIZE(long_data));
    gutil_log_dump_bytes(log, GLOG_LEVEL_ALWAYS, NULL, bytes);
    g_assert_cmpstr(test_log_buf->str, == ,long_data_dump);
    g_bytes_unref(bytes);

    g_string_free(test_log_buf, TRUE);
    test_log_buf = NULL;
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
#ifdef HAVE_TEST_LOG_FILE
    g_test_add_func(TEST_PREFIX "file", test_log_file);
#endif
    g_test_add_func(TEST_PREFIX "enabled", test_log_enabled);
    g_test_add_func(TEST_PREFIX "misc", test_log_misc);
    g_test_add_func(TEST_PREFIX "dump", test_log_dump);
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
