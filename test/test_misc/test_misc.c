/*
 * Copyright (C) 2023 Slava Monich <slava@monich.com>
 * Copyright (C) 2016-2022 Jolla Ltd.
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

#include "test_common.h"

#include "gutil_misc.h"
#include "gutil_idlepool.h"
#include "gutil_log.h"
#include "gutil_version.h"

static TestOpt test_opt;

/*==========================================================================*
 * version
 *==========================================================================*/

static
void
test_version(
    void)
{
    g_assert_cmpuint(gutil_version(), == ,GUTIL_VERSION);
}

/*==========================================================================*
 * disconnect
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
 * ref
 *==========================================================================*/

static
void
test_ref(
    void)
{
    GObject* obj = g_object_new(TEST_OBJECT_TYPE, NULL);

    g_assert(!gutil_object_ref(NULL));
    g_assert(gutil_object_ref(obj) == obj);
    g_object_unref(obj);
    g_object_unref(obj);
}

/*==========================================================================*
 * unref
 *==========================================================================*/

static
void
test_unref(
    void)
{
    gutil_object_unref(NULL);
    gutil_object_unref(g_object_new(TEST_OBJECT_TYPE, NULL));
}

/*==========================================================================*
 * hex2bin
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
 * bin2hex
 *==========================================================================*/

static
void
test_bin2hex(
    void)
{
    static const guchar bin[] = { 0x89, 0xab, 0xcd, 0xef };
    static const GUtilData data = { TEST_ARRAY_AND_SIZE(bin) };
    char* str;

    /* gutil_data2hex return NULL if data is NULL */
    g_assert(!gutil_data2hex(NULL, FALSE));

    /* Data isn't touched if len is zero */
    str = gutil_bin2hex(NULL, 0, FALSE);
    g_assert_cmpstr(str, == ,"");
    g_free(str);

    /* Lower case */
    str = gutil_bin2hex(bin, sizeof(bin), FALSE);
    g_assert_cmpstr(str, == ,"89abcdef");
    g_free(str);

    str = gutil_data2hex(&data, FALSE);
    g_assert_cmpstr(str, == ,"89abcdef");
    g_free(str);

    /* Upper case */
    str = gutil_bin2hex(bin, sizeof(bin), TRUE);
    g_assert_cmpstr(str, == ,"89ABCDEF");
    g_free(str);

    str = gutil_data2hex(&data, TRUE);
    g_assert_cmpstr(str, == ,"89ABCDEF");
    g_free(str);
}

/*==========================================================================*
 * hexdump
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
 * parse_int
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
    g_assert(!gutil_parse_int("0", -1, NULL));
    g_assert(!gutil_parse_int("0", 1, NULL));
    g_assert(gutil_parse_int("0", 0, NULL));
    g_assert(gutil_parse_int("0", 0, &value));
    g_assert_cmpint(value, == ,0);
    g_assert(gutil_parse_int("-1", 0, &value));
    g_assert_cmpint(value, == ,-1);
    g_assert(gutil_parse_int("42", 0, &value));
    g_assert_cmpint(value, == ,42);
    g_assert(!gutil_parse_int("0x10000000000000000", 0, &value));
    g_assert(!gutil_parse_int("-2147483649", 0, &value));
    g_assert(!gutil_parse_int("4294967295", 0, &value));
    g_assert(gutil_parse_int(" 0x7fffffff ", 0, &value));
    g_assert_cmpint(value, == ,0x7fffffff);
    g_assert(gutil_parse_int(" 7fffffff ", 16, &value));
    g_assert_cmpint(value, == ,0x7fffffff);
    g_assert(gutil_parse_int("7ffffffe ", 16, &value));
    g_assert_cmpint(value, == ,0x7ffffffe);
    g_assert(!gutil_parse_int("0xffffffff", 0, &value));
}

/*==========================================================================*
 * parse_uint
 *==========================================================================*/

static
void
test_parse_uint(
    void)
{
    unsigned int value;

    g_assert(!gutil_parse_uint(NULL, 0, NULL));
    g_assert(!gutil_parse_uint("", 0, NULL));
    g_assert(!gutil_parse_uint("garbage", 0, NULL));
    g_assert(!gutil_parse_uint("0 trailing garbage", 0, NULL));
    g_assert(!gutil_parse_uint("0", -1, NULL));
    g_assert(!gutil_parse_uint("0", 1, NULL));
    g_assert(gutil_parse_uint("0", 0, NULL));
    g_assert(gutil_parse_uint("0", 0, &value));
    g_assert_cmpuint(value, == ,0);
    g_assert(gutil_parse_uint("42", 0, &value));
    g_assert_cmpuint(value, == ,42);
    g_assert(!gutil_parse_uint("0x10000000000000000", 0, &value));
#if defined __SIZEOF_INT__ && __SIZEOF_INT__ == 4
    g_assert(!gutil_parse_uint("0x100000000", 0, &value));
#endif
    g_assert(!gutil_parse_uint("-2147483649", 0, &value));
    g_assert(!gutil_parse_uint("-1", 0, &value));
    g_assert(!gutil_parse_uint(" -1 ", 0, &value));
    g_assert(gutil_parse_uint("4294967295", 0, &value));
    g_assert_cmpuint(value, == ,4294967295U);
    g_assert(gutil_parse_uint(" 0x7fffffff ", 0, &value));
    g_assert_cmpuint(value, == ,0x7fffffff);
    g_assert(gutil_parse_uint(" 7fffffff ", 16, &value));
    g_assert_cmpuint(value, == ,0x7fffffff);
    g_assert(gutil_parse_uint("7ffffffe ", 16, &value));
    g_assert_cmpuint(value, == ,0x7ffffffe);
    g_assert(gutil_parse_uint("0xffffffff", 0, &value));
    g_assert_cmpuint(value, == ,0xffffffff);
}

/*==========================================================================*
 * parse_int64
 *==========================================================================*/

static
void
test_parse_int64(
    void)
{
    gint64 value;

    g_assert(!gutil_parse_int64(NULL, 0, NULL));
    g_assert(!gutil_parse_int64("", 0, NULL));
    g_assert(!gutil_parse_int64("garbage", 0, NULL));
    g_assert(!gutil_parse_int64("0 trailing garbage", 0, NULL));
    g_assert(!gutil_parse_int64("0", -1, NULL));
    g_assert(!gutil_parse_int64("0", 1, NULL));
    g_assert(gutil_parse_int64("0", 0, NULL));
    g_assert(gutil_parse_int64("0", 0, &value));
    g_assert_cmpint(value, == ,0);
    g_assert(gutil_parse_int64("-1", 0, &value));
    g_assert_cmpint(value, == ,-1);
    g_assert(gutil_parse_int64("42", 0, &value));
    g_assert_cmpint(value, == ,42);
    g_assert(gutil_parse_int64("-2147483649", 0, &value));
    g_assert_cmpint(value, == ,G_GINT64_CONSTANT(-2147483649));
    g_assert(gutil_parse_int64("4294967295", 0, &value));
    g_assert_cmpint(value, == ,G_GINT64_CONSTANT(4294967295));
    g_assert(gutil_parse_int64(" 0x7fffffff ", 0, &value));
    g_assert_cmpint(value, == ,0x7fffffff);
    g_assert(gutil_parse_int64(" 7fffffff ", 16, &value));
    g_assert_cmpint(value, == ,0x7fffffff);
    g_assert(gutil_parse_int64("7ffffffe ", 16, &value));
    g_assert_cmpint(value, == ,0x7ffffffe);
    g_assert(gutil_parse_int64("0xffffffff", 0, &value));
    g_assert_cmpint(value, == ,0xffffffff);
    g_assert(gutil_parse_int64("-9223372036854775808", 0, &value));
    g_assert_cmpint(value, == ,0x8000000000000000);
    g_assert(gutil_parse_int64("9223372036854775807", 0, &value));
    g_assert_cmpint(value, == ,0x7fffffffffffffff);
#ifndef _WIN32
    g_assert(!gutil_parse_int64("0x10000000000000000", 0, &value));
    g_assert(!gutil_parse_int64("-9223372036854775809", 0, &value));
    g_assert(!gutil_parse_int64("9223372036854775808", 0, &value));
#endif
}

/*==========================================================================*
 * parse_uint64
 *==========================================================================*/

static
void
test_parse_uint64(
    void)
{
    guint64 value;

    g_assert(!gutil_parse_uint64(NULL, 0, NULL));
    g_assert(!gutil_parse_uint64("", 0, NULL));
    g_assert(!gutil_parse_uint64("garbage", 0, NULL));
    g_assert(!gutil_parse_uint64("0 trailing garbage", 0, NULL));
    g_assert(!gutil_parse_uint64("0", -1, NULL));
    g_assert(!gutil_parse_uint64("0", 1, NULL));
    g_assert(gutil_parse_uint64("0", 0, NULL));
    g_assert(gutil_parse_uint64("0", 0, &value));
    g_assert_cmpuint(value, == ,0);
    g_assert(gutil_parse_uint64("42", 0, &value));
    g_assert_cmpuint(value, == ,42);
    g_assert(!gutil_parse_uint64("-2147483649", 0, &value));
    g_assert(!gutil_parse_uint64("-1", 0, &value));
    g_assert(!gutil_parse_uint64(" -1 ", 0, &value));
    g_assert(gutil_parse_uint64("4294967295", 0, &value));
    g_assert_cmpuint(value, == ,4294967295U);
    g_assert(!gutil_parse_uint64(" 0x7fffffff ffffffff ", 0, &value));
    g_assert(gutil_parse_uint64(" 0x7fffffffffffffff ", 0, &value));
    g_assert_cmpuint(value, == ,0x7fffffffffffffff);
    g_assert(!gutil_parse_uint64(" 7fffffff ffffffff", 16, &value));
    g_assert(gutil_parse_uint64(" 7fffffffffffffff ", 16, &value));
    g_assert_cmpuint(value, == ,0x7fffffffffffffff);
    g_assert(gutil_parse_uint64("0x100000000", 0, &value));
    g_assert_cmpuint(value, == ,0x100000000);
    g_assert(gutil_parse_uint64("0xffffffffffffffff", 0, &value));
    g_assert_cmpuint(value, == ,0xffffffffffffffff);
#ifndef _WIN32
    g_assert(!gutil_parse_uint64("0x10000000000000000", 0, &value));
#endif
}

/*==========================================================================*
 * data_equal
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
 * data_prefix
 *==========================================================================*/

static
void
test_data_prefix(
    void)
{
    static const guint8 val_123[] = { '1', '2', '3' };
    static const guint8 val_1234[] = { '1', '2', '3', '4' };
    static const guint8 val_234[] = { '2', '3', '4' };

    GUtilData data_empty, data_123, data_1234, data_234;

    memset(&data_empty, 0, sizeof(data_empty));
    TEST_INIT_DATA(data_123, val_123);
    TEST_INIT_DATA(data_1234, val_1234);
    TEST_INIT_DATA(data_234, val_234);

    g_assert(gutil_data_has_prefix(NULL, NULL));
    g_assert(!gutil_data_has_prefix(&data_empty, NULL));
    g_assert(!gutil_data_has_prefix(NULL, &data_empty));
    g_assert(gutil_data_has_prefix(&data_empty, &data_empty));
    g_assert(gutil_data_has_prefix(&data_123, &data_empty));
    g_assert(gutil_data_has_prefix(&data_1234, &data_123));
    g_assert(!gutil_data_has_prefix(&data_123, &data_1234));
    g_assert(!gutil_data_has_prefix(&data_1234, &data_234));
}

/*==========================================================================*
 * data_suffix
 *==========================================================================*/

static
void
test_data_suffix(
    void)
{
    static const guint8 val_123[] = { '1', '2', '3' };
    static const guint8 val_1234[] = { '1', '2', '3', '4' };
    static const guint8 val_234[] = { '2', '3', '4' };

    GUtilData data_empty, data_123, data_1234, data_234;

    memset(&data_empty, 0, sizeof(data_empty));
    TEST_INIT_DATA(data_123, val_123);
    TEST_INIT_DATA(data_1234, val_1234);
    TEST_INIT_DATA(data_234, val_234);

    g_assert(gutil_data_has_suffix(NULL, NULL));
    g_assert(!gutil_data_has_suffix(&data_empty, NULL));
    g_assert(!gutil_data_has_suffix(NULL, &data_empty));
    g_assert(gutil_data_has_suffix(&data_empty, &data_empty));
    g_assert(gutil_data_has_suffix(&data_123, &data_empty));
    g_assert(gutil_data_has_suffix(&data_1234, &data_234));
    g_assert(!gutil_data_has_suffix(&data_234, &data_1234));
    g_assert(!gutil_data_has_suffix(&data_1234, &data_123));
}

/*==========================================================================*
 * data_from_bytes
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
 * data_from_string
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
 * data_copy
 *==========================================================================*/

static
void
test_data_copy(
    void)
{
    static const guint8 src_data[] = { '1', '2', '3' };
    GUtilData* data;
    GUtilData src;

    TEST_INIT_DATA(src, src_data);

    g_assert(!gutil_data_copy(NULL));
    data = gutil_data_copy(&src);
    g_assert(gutil_data_equal(data, &src));
    g_free(data);

    /* This is wrong but when pointer is NULL, the length is ignored */
    data = gutil_data_new(NULL, 1000);
    g_assert(!data->bytes);
    g_assert(!data->size);
    g_free(data);

    data = gutil_data_new(TEST_ARRAY_AND_SIZE(src_data));
    g_assert(gutil_data_equal(data, &src));
    g_free(data);
}

/*==========================================================================*
 * bytes_concat
 *==========================================================================*/

static
void
test_bytes_concat(
    void)
{
    static const guint8 val1[] = {0x01,0x02,0x03,0x04,0x05};
    static const guint8 val2[] = {0x06,0x07,0x08,0x09};
    static const guint8 val3[] = {0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09};
    GBytes* b1 = g_bytes_new_static(val1, sizeof(val1));
    GBytes* b2 = g_bytes_new_static(val2, sizeof(val2));
    GBytes* b3 = g_bytes_new_static(val3, sizeof(val3));
    GBytes* empty = g_bytes_new(NULL, 0);
    GBytes* b;

    g_assert(!gutil_bytes_concat(NULL, NULL));

    b = gutil_bytes_concat(b1, NULL);
    g_assert(b == b1);
    g_bytes_unref(b);

    b = gutil_bytes_concat(empty, NULL);
    g_assert(b == empty);
    g_bytes_unref(b);

    b = gutil_bytes_concat(b1, empty, NULL);
    g_assert(b == b1);
    g_bytes_unref(b);

    b = gutil_bytes_concat(empty, b1, NULL);
    g_assert(b == b1);
    g_bytes_unref(b);

    b = gutil_bytes_concat(b1, empty, b2, NULL);
    g_assert(g_bytes_equal(b, b3));
    g_bytes_unref(b);

    g_bytes_unref(b1);
    g_bytes_unref(b2);
    g_bytes_unref(b3);
    g_bytes_unref(empty);
}

/*==========================================================================*
 * bytes_xor
 *==========================================================================*/

static
void
test_bytes_xor(
    void)
{
    static const guint8 val1[] = {0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09};
    static const guint8 val2[] = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08};
    static const guint8 val3[] = {0x01,0x03,0x01,0x07,0x01,0x03,0x01,0x0F,0x01};
    static const guint8 val4[] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
    static const guint8 val5[] = {0x05,0x06,0x07,0x08};
    static const guint8 val6[] = {0x04,0x04,0x04,0x0C};
    GBytes* b1 = g_bytes_new_static(val1, sizeof(val1));
    GBytes* b2 = g_bytes_new_static(val2, sizeof(val2));
    GBytes* b3 = g_bytes_new_static(val3, sizeof(val3));
    GBytes* b4 = g_bytes_new_static(val4, sizeof(val4));
    GBytes* b5 = g_bytes_new_static(val5, sizeof(val5));
    GBytes* b6 = g_bytes_new_static(val6, sizeof(val6));
    GBytes* empty = g_bytes_new(NULL, 0);
    GBytes* b;

    g_assert(!gutil_bytes_xor(b1, NULL));
    g_assert(!gutil_bytes_xor(NULL, b1));
    g_assert(!gutil_bytes_xor(NULL, NULL));

    b = gutil_bytes_xor(empty, b1);
    g_assert(b == empty);
    g_bytes_unref(b);

    b = gutil_bytes_xor(b1, empty);
    g_assert(b == empty);
    g_bytes_unref(b);

    b = gutil_bytes_xor(b1, b1);    /* 010203040506070809^010203040506070809 */
    g_assert(g_bytes_equal(b, b4));                   /* =000000000000000000 */
    g_bytes_unref(b);

    b = gutil_bytes_xor(b1, b2);    /* 010203040506070809^000102030405060708 */
    g_assert(g_bytes_equal(b, b3));                   /* =010301070103010F01 */
    g_bytes_unref(b);

    b = gutil_bytes_xor(b1, b5);    /* 010203040506070809^05060708 */
    g_assert(g_bytes_equal(b, b6));                   /* =0404040C */
    g_bytes_unref(b);

    g_bytes_unref(b1);
    g_bytes_unref(b2);
    g_bytes_unref(b3);
    g_bytes_unref(b4);
    g_bytes_unref(b5);
    g_bytes_unref(b6);
    g_bytes_unref(empty);
}

/*==========================================================================*
 * bytes_equal
 *==========================================================================*/

static
void
test_bytes_equal(
    void)
{
    static const guint8 data1[] =  { 0x01 };
    static const guint8 data11[] =  { 0x01, 0x01 };
    static const guint8 data2[] =  { 0x02 };
    GUtilData data;
    GBytes* bytes = g_bytes_new(NULL, 0);

    memset(&data, 0, sizeof(data));
    g_assert(gutil_bytes_equal(NULL, NULL, 0));
    g_assert(gutil_bytes_equal_data(NULL, NULL));

    /* One of the arguments is not NULL => not equal */
    g_assert(!gutil_bytes_equal(NULL, &data, 0));
    g_assert(!gutil_bytes_equal(bytes, NULL, 0));
    g_assert(!gutil_bytes_equal_data(bytes, NULL));
    g_assert(!gutil_bytes_equal_data(NULL, &data));

    /* Empty blocks of data are equal */
    g_assert(gutil_bytes_equal(bytes, &data, 0));
    g_assert(gutil_bytes_equal_data(bytes, &data));

    /* Different sizes */
    data.bytes = data11;
    data.size = sizeof(data11);
    g_assert(!gutil_bytes_equal(bytes, data.bytes, data.size));
    g_assert(!gutil_bytes_equal_data(bytes, &data));

    g_bytes_unref(bytes);
    bytes = g_bytes_new_static(data1, sizeof(data1));
    g_assert(!gutil_bytes_equal(bytes, data.bytes, data.size));
    g_assert(!gutil_bytes_equal_data(bytes, &data));

    /* Different contents */
    data.bytes = data2;
    data.size = sizeof(data2);
    g_assert(!gutil_bytes_equal(bytes, data.bytes, data.size));
    g_assert(!gutil_bytes_equal_data(bytes, &data));

    /* Match */
    data.bytes = data1;
    data.size = sizeof(data1);
    g_assert(gutil_bytes_equal(bytes, data.bytes, data.size));
    g_assert(gutil_bytes_equal_data(bytes, &data));
    g_bytes_unref(bytes);
}

/*==========================================================================*
 * ptrv_length
 *==========================================================================*/

static
void
test_ptrv_length(
    void)
{
    static const gconstpointer ptrv0[] = { NULL };
    static const gconstpointer ptrv1[] = { ptrv0, NULL };

    g_assert_cmpuint(gutil_ptrv_length(NULL), == ,0);
    g_assert_cmpuint(gutil_ptrv_length(ptrv0), == ,0);
    g_assert_cmpuint(gutil_ptrv_length(ptrv1), == ,1);
}

/*==========================================================================*
 * ptrv_is_empty
 *==========================================================================*/

static
void
test_ptrv_is_empty(
    void)
{
    static const gconstpointer ptrv0[] = { NULL };
    static const gconstpointer ptrv1[] = { ptrv0, NULL };

    g_assert(gutil_ptrv_is_empty(NULL));
    g_assert(gutil_ptrv_is_empty(ptrv0));
    g_assert(!gutil_ptrv_is_empty(ptrv1));
}

/*==========================================================================*
 * ptrv_free
 *==========================================================================*/

static
void
test_ptrv_free(
    void)
{
    void** ptrv0 = g_new0(void*, 1);
    void** ptrv1 = g_new0(void*, 2);

    ptrv1[0] = g_new0(int, 1);
    gutil_ptrv_free(NULL);
    gutil_ptrv_free(ptrv0);
    gutil_ptrv_free(ptrv1);
}

/*==========================================================================*
 * memdup
 *==========================================================================*/

static
void
test_memdup(
    void)
{
    static const guint8 data[] = { 0x01, 0x02, 0x03 };
    void* copy = gutil_memdup(data, sizeof(data));

    g_assert(copy);
    g_assert(!memcmp(copy, data, sizeof(data)));
    g_free(copy);

    g_assert(!gutil_memdup(data, 0));
    g_assert(!gutil_memdup(NULL, 0));
    g_assert(!gutil_memdup(NULL, 1));
}

/*==========================================================================*
 * strlen
 *==========================================================================*/

static
void
test_strlen(
    void)
{
    g_assert(!gutil_strlen0(NULL));
    g_assert_cmpuint(gutil_strlen0(""), == ,0);
    g_assert_cmpuint(gutil_strlen0("1"), == ,1);
}

/*==========================================================================*
 * range_init
 *==========================================================================*/

static
void
test_range_init(
    void)
{
    static const guint8 data[] = { 0x01, 0x02, 0x03 };
    GBytes* bytes = g_bytes_new_static(data, sizeof(data));
    GUtilRange range;

    g_assert(!gutil_range_init_with_bytes(NULL, NULL));
    g_assert(!gutil_range_init_with_bytes(&range, NULL));
    g_assert(!range.ptr);
    g_assert(!range.end);

    g_assert(gutil_range_init_with_bytes(&range, bytes) == sizeof(data));
    g_assert(range.ptr == data);
    g_assert(range.end == (data + sizeof(data)));
    g_bytes_unref(bytes);
}

/*==========================================================================*
 * range_prefix
 *==========================================================================*/

static
void
test_range_prefix(
    void)
{
    static const guint8 data[] = { 0x01, 0x02, 0x03, 0x04 };
    static const guint8 prefix[] = { 0x01, 0x02 };
    static const guint8 not_prefix[] = { 0x03, 0x04 };
    static const guint8 too_long[] = { 0x01, 0x02, 0x03, 0x04, 0x05 };
    GUtilData prefix_data, not_prefix_data, too_long_data;
    GUtilRange range;

    memset(&range, 0, sizeof(range));
    g_assert(!gutil_range_has_prefix(NULL, NULL));
    g_assert(!gutil_range_has_prefix(&range, NULL));

    not_prefix_data.bytes = not_prefix;
    not_prefix_data.size = sizeof(not_prefix);
    too_long_data.bytes = too_long;
    too_long_data.size = sizeof(too_long);

    range.end = range.ptr = data; /* Empty range */
    memset(&prefix_data, 0, sizeof(prefix_data)); /* Empty prefix */
    /* Empty range doesn't have NULL prefix */
    g_assert(!gutil_range_has_prefix(&range, NULL));
    /* But does have empty prefix */
    g_assert(gutil_range_has_prefix(&range, &prefix_data));

    /* And doesn't have non-empty prefix */
    g_assert(!gutil_range_has_prefix(&range, &not_prefix_data));
    prefix_data.bytes = prefix;
    prefix_data.size = sizeof(prefix);
    g_assert(!gutil_range_has_prefix(&range, &prefix_data));

    range.end = range.ptr + sizeof(data);

    g_assert(gutil_range_has_prefix(&range, &prefix_data));
    g_assert(!gutil_range_has_prefix(&range, &not_prefix_data));
    g_assert(!gutil_range_has_prefix(&range, &too_long_data));

    /* Test skipping */
    g_assert(!gutil_range_skip_prefix(&range, &not_prefix_data));
    g_assert(range.ptr == data);

    g_assert(gutil_range_skip_prefix(&range, &prefix_data));
    g_assert(range.ptr == (data + prefix_data.size));
}

/*==========================================================================*
 * bytes_prefix
 *==========================================================================*/

static
void
test_bytes_prefix(
    void)
{
    static const guint8 data[] = { 0x01, 0x02, 0x03, 0x04 };
    static const guint8 prefix[] = { 0x01, 0x02 };
    static const guint8 not_prefix[] = { 0x03, 0x04 };
    static const guint8 too_long[] = { 0x01, 0x02, 0x03, 0x04, 0x05 };
    GBytes* empty = g_bytes_new_static(NULL, 0);
    GBytes* bytes = g_bytes_new_static(TEST_ARRAY_AND_SIZE(data));

    /* NULL GBytes doesn't have any prefix, even an empty one */
    g_assert(!gutil_bytes_has_prefix(NULL, NULL, 0));

    /* Anything has an empty prefix */
    g_assert(gutil_bytes_has_prefix(empty, NULL, 0));
    g_assert(gutil_bytes_has_prefix(bytes, prefix, 0));

    /* Test the matching */
    g_assert(gutil_bytes_has_prefix(bytes, TEST_ARRAY_AND_SIZE(prefix)));
    g_assert(!gutil_bytes_has_prefix(bytes, TEST_ARRAY_AND_SIZE(not_prefix)));
    g_assert(!gutil_bytes_has_prefix(bytes, TEST_ARRAY_AND_SIZE(too_long)));

    g_bytes_unref(empty);
    g_bytes_unref(bytes);
}

/*==========================================================================*
 * bytes_suffix
 *==========================================================================*/

static
void
test_bytes_suffix(
    void)
{
    static const guint8 data[] = { 0x01, 0x02, 0x03, 0x04 };
    static const guint8 suffix[] = { 0x03, 0x04 };
    static const guint8 not_suffix[] = { 0x02, 0x03 };
    static const guint8 too_long[] = { 0x01, 0x02, 0x03, 0x04, 0x05 };
    GBytes* empty = g_bytes_new_static(NULL, 0);
    GBytes* bytes = g_bytes_new_static(TEST_ARRAY_AND_SIZE(data));

    /* NULL GBytes doesn't have any suffix, even an empty one */
    g_assert(!gutil_bytes_has_suffix(NULL, NULL, 0));

    /* Anything has an empty suffix */
    g_assert(gutil_bytes_has_suffix(empty, NULL, 0));
    g_assert(gutil_bytes_has_suffix(bytes, suffix, 0));

    /* Test the matching */
    g_assert(gutil_bytes_has_suffix(bytes, TEST_ARRAY_AND_SIZE(suffix)));
    g_assert(!gutil_bytes_has_suffix(bytes, TEST_ARRAY_AND_SIZE(not_suffix)));
    g_assert(!gutil_bytes_has_suffix(bytes, TEST_ARRAY_AND_SIZE(too_long)));

    g_bytes_unref(empty);
    g_bytes_unref(bytes);
}

/*==========================================================================*
 * Common
 *==========================================================================*/

#define TEST_(x) "/misc/" x

int main(int argc, char* argv[])
{
    G_GNUC_BEGIN_IGNORE_DEPRECATIONS;
    g_type_init();
    G_GNUC_END_IGNORE_DEPRECATIONS;
    g_test_init(&argc, &argv, NULL);

    gutil_log_timestamp = FALSE;
    gutil_log_default.level = g_test_verbose() ?
        GLOG_LEVEL_VERBOSE : GLOG_LEVEL_NONE;

    g_test_add_func(TEST_("version"), test_version);
    g_test_add_func(TEST_("disconnect"), test_disconnect);
    g_test_add_func(TEST_("ref"), test_ref);
    g_test_add_func(TEST_("unref"), test_unref);
    g_test_add_func(TEST_("hex2bin"), test_hex2bin);
    g_test_add_func(TEST_("bin2hex"), test_bin2hex);
    g_test_add_func(TEST_("hexdump"), test_hexdump);
    g_test_add_func(TEST_("parse_int"), test_parse_int);
    g_test_add_func(TEST_("parse_uint"), test_parse_uint);
    g_test_add_func(TEST_("parse_int64"), test_parse_int64);
    g_test_add_func(TEST_("parse_uint64"), test_parse_uint64);
    g_test_add_func(TEST_("data_equal"), test_data_equal);
    g_test_add_func(TEST_("data_prefix"), test_data_prefix);
    g_test_add_func(TEST_("data_suffix"), test_data_suffix);
    g_test_add_func(TEST_("data_from_bytes"), test_data_from_bytes);
    g_test_add_func(TEST_("data_from_string"), test_data_from_string);
    g_test_add_func(TEST_("data_copy"), test_data_copy);
    g_test_add_func(TEST_("bytes_concat"), test_bytes_concat);
    g_test_add_func(TEST_("bytes_xor"), test_bytes_xor);
    g_test_add_func(TEST_("bytes_equal"), test_bytes_equal);
    g_test_add_func(TEST_("ptrv_length"), test_ptrv_length);
    g_test_add_func(TEST_("ptrv_is_empty"), test_ptrv_is_empty);
    g_test_add_func(TEST_("ptrv_free"), test_ptrv_free);
    g_test_add_func(TEST_("memdup"), test_memdup);
    g_test_add_func(TEST_("strlen"), test_strlen);
    g_test_add_func(TEST_("range_init"), test_range_init);
    g_test_add_func(TEST_("range_prefix"), test_range_prefix);
    g_test_add_func(TEST_("bytes_prefix"), test_bytes_prefix);
    g_test_add_func(TEST_("bytes_suffix"), test_bytes_suffix);
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
