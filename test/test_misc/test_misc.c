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

#include "gutil_misc.h"
#include "gutil_idlepool.h"
#include "gutil_log.h"

static TestOpt test_opt;

/*==========================================================================*
 * Disconnect
 *==========================================================================*/

static
void
test_notify(
    GObject* object,
    GParamSpec* spec,
    gpointer data)
{
}

static
void
test_disconnect(
    void)
{
    gulong id[2];
    GObject* obj = g_object_new(TEST_OBJECT_TYPE, NULL);

    /* These have no effect */
    gutil_disconnect_handlers(NULL, id, G_N_ELEMENTS(id));
    gutil_disconnect_handlers(obj, NULL, 0);

    id[0] = g_signal_connect(obj, "notify", G_CALLBACK(test_notify), NULL);
    id[1] = g_signal_connect(obj, "notify", G_CALLBACK(test_notify), NULL);
    GASSERT(id[0] && id[1]);

    /* gutil_disconnect_handlers zeros the ids */
    gutil_disconnect_handlers(obj, id, G_N_ELEMENTS(id));
    g_assert(!id[0]);
    g_assert(!id[1]);

    /* Second time has no effect */
    gutil_disconnect_handlers(obj, id, G_N_ELEMENTS(id));
    g_assert(!id[0]);
    g_assert(!id[1]);

    g_object_unref(obj);
}

/*==========================================================================*
 * Hex2bin
 *==========================================================================*/

static
void
test_hex2bin(
    void)
{
    guint8 buf[4];
    GBytes* bytes;
    gsize size;
    const void* data;
    static const guint8 buf1[4] = { 0x01, 0x23, 0x45, 0x67 };
    static const guint8 buf2[4] = { 0x89, 0xab, 0xcd, 0xef };
    g_assert(!gutil_hex2bin(NULL, 0, NULL));
    g_assert(!gutil_hex2bin("x", 0, NULL));
    g_assert(!gutil_hex2bin("x", 0, buf));
    g_assert(!gutil_hex2bin("x", -1, buf));
    g_assert(!gutil_hex2bin("x", 1, buf));
    g_assert(!gutil_hex2bin("xy", 2, buf));
    g_assert(!gutil_hex2bin(" 1", 2, buf));
    g_assert(!gutil_hex2bin("1 ", 2, buf));
    g_assert(!gutil_hex2bin("1234FG", 6, buf));
    g_assert(gutil_hex2bin("01234567", 8, buf));
    g_assert(!memcmp(buf, buf1, sizeof(buf)));
    g_assert(gutil_hex2bin("89abcdef", 8, buf));
    g_assert(!memcmp(buf, buf2, sizeof(buf)));
    g_assert(gutil_hex2bin("89ABCDEF", 8, buf));
    g_assert(!memcmp(buf, buf2, sizeof(buf)));

    g_assert(!gutil_hex2bytes(NULL, 0));
    g_assert(!gutil_hex2bytes("x", 0));
    g_assert(!gutil_hex2bytes("x", 1));
    g_assert(!gutil_hex2bytes("x", -1));
    g_assert(!gutil_hex2bytes("xy", -1));
    bytes = gutil_hex2bytes("01234567", -1);
    g_assert(bytes);
    data = g_bytes_get_data(bytes, &size);
    g_assert(data);
    g_assert(size == 4);
    g_assert(!memcmp(data, buf1, sizeof(buf1)));
    g_bytes_unref(bytes);
}

/*==========================================================================*
 * Hexdump
 *==========================================================================*/

static
void
test_hexdump(
    void)
{
    char buf[GUTIL_HEXDUMP_BUFSIZE];
    static const guchar data[] = {
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
        0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f, 0x00
    };

    g_assert(gutil_hexdump(buf, data, sizeof(data)) == GUTIL_HEXDUMP_MAXBYTES);
    g_assert(strlen(buf) == GUTIL_HEXDUMP_BUFSIZE - 1);
    GDEBUG("%s", buf);
    g_assert(!strcmp(buf,
        "30 31 32 33 34 35 36 37  "
        "38 39 3a 3b 3c 3d 3e 3f    "
        "01234567 89:;<=>?"));

    g_assert(gutil_hexdump(buf, data + GUTIL_HEXDUMP_MAXBYTES, 1) == 1);
    g_assert(strlen(buf) == 53);
    GDEBUG("%s", buf);
    g_assert(!strcmp(buf,
        "00                       "
        "                           "
        "."));
}

/*==========================================================================*
 * ParseInt
 *==========================================================================*/

static
void
test_parse_int(
    void)
{
    int value;

    g_assert(!gutil_parse_int(NULL, 0, NULL));
    g_assert(!gutil_parse_int("", 0, NULL));
    g_assert(!gutil_parse_int("garbage", 0, NULL));
    g_assert(!gutil_parse_int("0 trailing garbage", 0, NULL));
    g_assert(gutil_parse_int("0", 0, NULL));
    g_assert(gutil_parse_int("0", 0, &value));
    g_assert(value == 0);
    g_assert(!gutil_parse_int("0x10000000000000000", 0, &value));
    g_assert(!gutil_parse_int("-2147483649", 0, &value));
    g_assert(!gutil_parse_int("4294967295", 0, &value));
    g_assert(gutil_parse_int(" 0x7fffffff ", 0, &value));
    g_assert(value == 0x7fffffff);
    g_assert(gutil_parse_int(" 7fffffff ", 16, &value));
    g_assert(value == 0x7fffffff);
    g_assert(!gutil_parse_int("0xffffffff", 0, &value));
}

/*==========================================================================*
 * DataEqual
 *==========================================================================*/

static
void
test_data_equal(
    void)
{
    static const guint8 val_123[] = { '1', '2', '3' };
    static const guint8 val_1234[] = { '1', '2', '3', '4' };
    static const guint8 val_321[] = { '3', '2', '1' };

    GUtilData data_123, data_123a, data_1234, data_321;

    TEST_INIT_DATA(data_123, val_123);
    TEST_INIT_DATA(data_123a, val_123);
    TEST_INIT_DATA(data_1234, val_1234);
    TEST_INIT_DATA(data_321, val_321);

    g_assert(gutil_data_equal(NULL, NULL));
    g_assert(gutil_data_equal(&data_123, &data_123));
    g_assert(gutil_data_equal(&data_123, &data_123a));
    g_assert(!gutil_data_equal(&data_123, &data_1234));
    g_assert(!gutil_data_equal(&data_123, &data_321));
    g_assert(!gutil_data_equal(&data_123, NULL));
    g_assert(!gutil_data_equal(NULL, &data_123));
}

/*==========================================================================*
 * DataFromBytes
 *==========================================================================*/

static
void
test_data_from_bytes(
    void)
{
    static const guint8 val_123[] = { '1', '2', '3' };
    GBytes* bytes_123 = g_bytes_new_static(val_123, sizeof(val_123));
    GUtilData data_123, data;

    TEST_INIT_DATA(data_123, val_123);
    TEST_INIT_DATA(data, val_123);

    g_assert(!gutil_data_from_bytes(NULL, NULL));

    g_assert(gutil_data_from_bytes(&data, NULL) == &data);
    g_assert(!data.bytes);
    g_assert(!data.size);

    g_assert(gutil_data_from_bytes(&data, bytes_123) == &data);
    g_assert(gutil_data_equal(&data_123, &data));
    g_bytes_unref(bytes_123);
}

/*==========================================================================*
 * DataFromString
 *==========================================================================*/

static
void
test_data_from_string(
    void)
{
    static const guint8 val_123[] = { '1', '2', '3' };
    GUtilData data_123, data;

    TEST_INIT_DATA(data_123, val_123);
    TEST_INIT_DATA(data, val_123);

    g_assert(!gutil_data_from_string(NULL, NULL));

    g_assert(gutil_data_from_string(&data, NULL) == &data);
    g_assert(!data.bytes);
    g_assert(!data.size);

    g_assert(gutil_data_from_string(&data, "123") == &data);
    g_assert(gutil_data_equal(&data_123, &data));
}

/*==========================================================================*
 * Common
 *==========================================================================*/

#define TEST_PREFIX "/misc/"

int main(int argc, char* argv[])
{
    G_GNUC_BEGIN_IGNORE_DEPRECATIONS;
    g_type_init();
    G_GNUC_END_IGNORE_DEPRECATIONS;
    g_test_init(&argc, &argv, NULL);

    gutil_log_timestamp = FALSE;
    gutil_log_default.level = g_test_verbose() ?
        GLOG_LEVEL_VERBOSE : GLOG_LEVEL_NONE;

    g_test_add_func(TEST_PREFIX "disconnect", test_disconnect);
    g_test_add_func(TEST_PREFIX "hex2bin", test_hex2bin);
    g_test_add_func(TEST_PREFIX "hexdump", test_hexdump);
    g_test_add_func(TEST_PREFIX "parse_int", test_parse_int);
    g_test_add_func(TEST_PREFIX "data_equal", test_data_equal);
    g_test_add_func(TEST_PREFIX "data_from_bytes", test_data_from_bytes);
    g_test_add_func(TEST_PREFIX "data_from_string", test_data_from_string);
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
