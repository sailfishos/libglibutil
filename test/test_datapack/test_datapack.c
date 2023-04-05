/*
 * Copyright (C) 2023 Slava Monich <slava@monich.com>
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

#include "gutil_datapack.h"

static TestOpt test_opt;

#define TEST_(name) "/datapack/" name

static const guint8 test_unsigned_mbn_data_0[] = { 0x00 };
static const guint8 test_unsigned_mbn_data_64[] = { 0x40 };
static const guint8 test_unsigned_mbn_data_127[] = { 0x7f };
static const guint8 test_unsigned_mbn_data_128[] = { 0x81, 0x00 };
static const guint8 test_unsigned_mbn_data_257[] = { 0x82, 0x01 };
static const guint8 test_unsigned_mbn_data_383[] = { 0x82, 0x7f };
static const guint8 test_unsigned_mbn_data_16383[] = { 0xff, 0x7f };
static const guint8 test_unsigned_mbn_data_max64[] =
    { 0x81, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f };

static const guint8 test_signed_mbn_data_0[] = { 0x00 };
static const guint8 test_signed_mbn_data_33[] = { 0x21 };
static const guint8 test_signed_mbn_data_m33[] = { 0x5f };
static const guint8 test_signed_mbn_data_65[] = { 0x80, 0x41 };
static const guint8 test_signed_mbn_data_m65[] = { 0xff, 0x3f };
static const guint8 test_signed_mbn_data_127[] = { 0x80, 0x7f };
static const guint8 test_signed_mbn_data_128[] = { 0x81, 0x00 };
static const guint8 test_signed_mbn_data_129[] = { 0x81, 0x01 };
static const guint8 test_signed_mbn_data_m129[] = { 0xfe, 0x7f };
static const guint8 test_signed_mbn_data_257[] = { 0x82, 0x01 };
static const guint8 test_signed_mbn_data_383[] = { 0x82, 0x7f };
static const guint8 test_signed_mbn_data_16383[] = { 0x80, 0xff, 0x7f };
static const guint8 test_signed_mbn_data_m16383[] = { 0xff, 0x80, 0x01 };
static const guint8 test_signed_mbn_data_min64[] = {
    0xff, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x00
};
static const guint8 test_signed_mbn_data_max64[] = {
    0x80, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0x7f
};

typedef struct {
    const char* name;
    GUtilData input;
} TestDecodeFail;

static const guint8 test_mbn_decode_fail_data_short[] = { 0x80 };
static const guint8 test_mbn_decode_fail_data_too_long[] = {
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
    0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f
};

/*==========================================================================*
 * mbn/corner_cases
 *==========================================================================*/

static
void
test_mbn_corner_cases()
{
    guint8 buf[1];

    g_assert_cmpuint(gutil_signed_mbn_encode2(buf, 0, 0), == ,0);
    g_assert_cmpuint(gutil_unsigned_mbn_encode2(buf, 0, 0), == ,0);
}

/*==========================================================================*
 * signed_mbn/size
 *==========================================================================*/

typedef struct {
    const char* name;
    gint64 input;
    guint output;
} TestSignedMbnSizeData;

/* Use test_mbn_write test data as input */
static const TestSignedMbnSizeData test_signed_mbn_size_data[] = {
#define TEST_CASE(x) TEST_("signed_mbn/size/") x
#define TEST_DATA(x) TEST_CASE(#x), x, sizeof(test_signed_mbn_data_##x)
#define TEST_DATA_(x) TEST_CASE("-" #x), -x, sizeof(test_signed_mbn_data_m##x)
    { TEST_DATA(0) },
    { TEST_DATA(33) },
    { TEST_DATA_(33) },
    { TEST_DATA(65) },
    { TEST_DATA_(65) },
    { TEST_DATA(127) },
    { TEST_DATA(128) },
    { TEST_DATA(129) },
    { TEST_DATA_(129) },
    { TEST_DATA(257) },
    { TEST_DATA(383) },
    { TEST_DATA(16383) },
    { TEST_DATA_(16383) },
    { TEST_CASE("min64"), G_MININT64, sizeof(test_signed_mbn_data_min64) },
    { TEST_CASE("max64"), G_MAXINT64, sizeof(test_signed_mbn_data_max64) }
#undef TEST_CASE
#undef TEST_DATA
#undef TEST_DATA_
};

static
void
test_signed_mbn_size(
    gconstpointer data)
{
    const TestSignedMbnSizeData* test = data;

    g_assert_cmpuint(gutil_signed_mbn_size(test->input), == ,test->output);
}

/*==========================================================================*
 * signed_mbn/encode
 *==========================================================================*/

typedef struct {
    const char* name;
    gint64 input;
    GUtilData output;
} TestSignedMbnEncode;

static const TestSignedMbnEncode test_signed_mbn_encode_data[] = {
#define TEST_CASE(x) TEST_("signed_mbn/encode/") x
#define TEST_DATA(x) TEST_CASE(#x), x, \
    { TEST_ARRAY_AND_SIZE(test_signed_mbn_data_##x) }
#define TEST_DATA_(x) TEST_CASE("-" #x), -x, \
    { TEST_ARRAY_AND_SIZE(test_signed_mbn_data_m##x) }
    { TEST_DATA(0) },
    { TEST_DATA(33) },
    { TEST_DATA_(33) },
    { TEST_DATA(65) },
    { TEST_DATA_(65) },
    { TEST_DATA(127) },
    { TEST_DATA(128) },
    { TEST_DATA(129) },
    { TEST_DATA_(129) },
    { TEST_DATA(257) },
    { TEST_DATA(383) },
    { TEST_DATA(16383) },
    { TEST_DATA_(16383) },
    { TEST_CASE("min64"), G_MININT64,
    { TEST_ARRAY_AND_SIZE(test_signed_mbn_data_min64) } },
    { TEST_CASE("max64"), G_MAXINT64,
    { TEST_ARRAY_AND_SIZE(test_signed_mbn_data_max64) } }
#undef TEST_CASE
#undef TEST_DATA
#undef TEST_DATA_
};

static
void
test_signed_mbn_encode(
    gconstpointer data)
{
    const TestSignedMbnEncode* test = data;
    guint8* buf = g_malloc(test->output.size);

    memset(buf, 0xaa, test->output.size);
    g_assert_cmpuint(gutil_signed_mbn_encode(buf, test->input), == ,
        test->output.size);
    g_assert(!memcmp(buf, test->output.bytes, test->output.size));

    g_free(buf);
}

/*==========================================================================*
 * signed_mbn/decode/ok
 *==========================================================================*/

typedef struct {
    const char* name;
    GUtilData input;
    gint64 output;
} TestSignedMbnDecodeOk;

static const TestSignedMbnDecodeOk test_signed_mbn_decode_ok_data[] = {
#define TEST_CASE(x) TEST_("signed_mbn/decode/ok/") x
#define TEST_DATA(x) TEST_CASE(#x), \
    { TEST_ARRAY_AND_SIZE(test_signed_mbn_data_##x) }, x
#define TEST_DATA_(x) TEST_CASE("-" #x), \
    { TEST_ARRAY_AND_SIZE(test_signed_mbn_data_m##x) }, -x
    { TEST_DATA(0) },
    { TEST_DATA(33) },
    { TEST_DATA_(33) },
    { TEST_DATA(65) },
    { TEST_DATA_(65) },
    { TEST_DATA(127) },
    { TEST_DATA(128) },
    { TEST_DATA(129) },
    { TEST_DATA_(129) },
    { TEST_DATA(257) },
    { TEST_DATA(383) },
    { TEST_DATA(16383) },
    { TEST_DATA_(16383) },
    { TEST_CASE("min64"),
    { TEST_ARRAY_AND_SIZE(test_signed_mbn_data_min64) }, G_MININT64 },
    { TEST_CASE("max64"),
    { TEST_ARRAY_AND_SIZE(test_signed_mbn_data_max64) }, G_MAXINT64 }
#undef TEST_CASE
#undef TEST_DATA
#undef TEST_DATA_
};

static
void
test_signed_mbn_decode_ok(
    gconstpointer data)
{
    const TestSignedMbnDecodeOk* test = data;
    GUtilRange range;
    gint64 value;

    range.ptr = test->input.bytes;
    range.end = range.ptr + test->input.size;
    g_assert(gutil_signed_mbn_decode(&range, NULL));
    g_assert(range.ptr == range.end);

    range.ptr = test->input.bytes;
    g_assert(gutil_signed_mbn_decode(&range, &value));
    g_assert_cmpint(value, == ,test->output);
    g_assert(range.ptr == range.end);
}

/*==========================================================================*
 * signed_mbn/decode/fail
 *==========================================================================*/

static const guint8 test_signed_mbn_decode_fail_data_positive_sign_ext[] = {
    0x8f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0x7f
};
static const guint8 test_signed_mbn_decode_fail_data_negative_sign_ext[] = {
    0xf1, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x00
};

static const TestDecodeFail test_signed_mbn_decode_fail_data[] = {
#define TEST_CASE(x) TEST_("signed_mbn/decode/fail/") x
#define TEST_DATA(x) TEST_CASE(#x), \
    { TEST_ARRAY_AND_SIZE(test_mbn_decode_fail_data_##x) }
#define TEST_DATA_(x) TEST_CASE(#x), \
    { TEST_ARRAY_AND_SIZE(test_signed_mbn_decode_fail_data_##x) }
    { TEST_CASE("null"), { NULL, 0 } },
    { TEST_CASE("empty"), {test_mbn_decode_fail_data_short, 0} },
    { TEST_DATA(short) },
    { TEST_DATA(too_long) },
    { TEST_DATA_(positive_sign_ext) },
    { TEST_DATA_(negative_sign_ext) }
#undef TEST_CASE
#undef TEST_DATA
#undef TEST_DATA_
};

static
void
test_signed_mbn_decode_fail(
    gconstpointer data)
{
    const TestDecodeFail* test = data;
    GUtilRange range;
    gint64 value;

    range.ptr = test->input.bytes;
    range.end = range.ptr + test->input.size;
    g_assert(!gutil_signed_mbn_decode(&range, &value));
    g_assert(range.ptr == test->input.bytes);
    g_assert(!gutil_signed_mbn_decode(&range, NULL));
    g_assert(range.ptr == test->input.bytes);
}

/*==========================================================================*
 * signed_mbn/decode2
 *==========================================================================*/

static
void
test_signed_mbn_decode2()
{
    static const guint8 trailing_garbage[] = { 0x12, 0x34 };
    const TestDecodeFail* fail = test_signed_mbn_decode_fail_data;
    const TestSignedMbnDecodeOk* ok = test_signed_mbn_decode_ok_data;
    gint64 value = 0xaaaaaaaa;
    GUtilData in;
    guint i;

    g_assert(!gutil_signed_mbn_decode2(NULL, NULL));
    g_assert(!gutil_signed_mbn_decode2(NULL, &value));

    in.bytes = trailing_garbage;
    in.size = sizeof(trailing_garbage);
    g_assert(!gutil_signed_mbn_decode2(&in, NULL));
    g_assert(!gutil_signed_mbn_decode2(&in, &value));

    /* Run only NULL, empty and short cases */
    for (i = 0; i < 3; i++) {
        g_assert(!gutil_signed_mbn_decode2(&fail[i].input, NULL));
        g_assert(!gutil_signed_mbn_decode2(&fail[i].input, &value));
    }

    g_assert(gutil_signed_mbn_decode2(&ok->input, NULL));
    g_assert(gutil_signed_mbn_decode2(&ok->input, &value));
    g_assert_cmpint(value, == ,ok->output);
}

/*==========================================================================*
 * unsigned_mbn/size
 *==========================================================================*/

typedef struct {
    const char* name;
    guint64 input;
    guint output;
} TestUnsignedMbnSizeData;

/* Use test_mbn_write test data as input */
static const TestUnsignedMbnSizeData test_unsigned_mbn_size_data[] = {
#define TEST_CASE(x) TEST_("unsigned_mbn/size/") x
#define TEST_DATA(x) TEST_CASE(#x), x, sizeof(test_unsigned_mbn_data_##x)
    { TEST_DATA(0) },
    { TEST_DATA(64) },
    { TEST_DATA(127) },
    { TEST_DATA(128) },
    { TEST_DATA(257) },
    { TEST_DATA(383) },
    { TEST_DATA(16383) },
    { TEST_CASE("max64"), G_MAXUINT64, sizeof(test_unsigned_mbn_data_max64) }
#undef TEST_CASE
#undef TEST_DATA
};

static
void
test_unsigned_mbn_size(
    gconstpointer data)
{
    const TestUnsignedMbnSizeData* test = data;

    g_assert_cmpuint(gutil_unsigned_mbn_size(test->input), == ,test->output);
}

/*==========================================================================*
 * unsigned_mbn/encode
 *==========================================================================*/

typedef struct {
    const char* name;
    guint64 input;
    GUtilData output;
} TestUnsignedMbnEncode;

static const TestUnsignedMbnEncode test_unsigned_mbn_encode_data[] = {
#define TEST_CASE(x) TEST_("unsigned_mbn/encode/") x
#define TEST_DATA(x) TEST_CASE(#x), x, \
    { TEST_ARRAY_AND_SIZE(test_unsigned_mbn_data_##x) }
    { TEST_DATA(0) },
    { TEST_DATA(64) },
    { TEST_DATA(127) },
    { TEST_DATA(128) },
    { TEST_DATA(257) },
    { TEST_DATA(383) },
    { TEST_DATA(16383) },
    { TEST_CASE("max64"), G_MAXUINT64,
    { TEST_ARRAY_AND_SIZE(test_unsigned_mbn_data_max64) } }
#undef TEST_CASE
#undef TEST_DATA
};

static
void
test_unsigned_mbn_encode(
    gconstpointer data)
{
    const TestUnsignedMbnEncode* test = data;
    guint8* buf = g_malloc(test->output.size);

    memset(buf, 0xaa, test->output.size);
    g_assert_cmpuint(gutil_unsigned_mbn_encode(buf, test->input), == ,
        test->output.size);
    g_assert(!memcmp(buf, test->output.bytes, test->output.size));

    g_free(buf);
}

/*==========================================================================*
 * unsigned_mbn/decode/ok
 *==========================================================================*/

typedef struct {
    const char* name;
    GUtilData input;
    guint64 output;
} TestUnsignedMbnDecodeOk;

static const TestUnsignedMbnDecodeOk test_unsigned_mbn_decode_ok_data[] = {
#define TEST_CASE(x) TEST_("unsigned_mbn/decode/ok/") x
#define TEST_DATA(x) TEST_CASE(#x), \
    { TEST_ARRAY_AND_SIZE(test_unsigned_mbn_data_##x) }, x
    { TEST_DATA(0) },
    { TEST_DATA(64) },
    { TEST_DATA(127) },
    { TEST_DATA(128) },
    { TEST_DATA(257) },
    { TEST_DATA(383) },
    { TEST_DATA(16383) },
    { TEST_CASE("max64"),
    { TEST_ARRAY_AND_SIZE(test_unsigned_mbn_data_max64) }, G_MAXUINT64 }
#undef TEST_CASE
#undef TEST_DATA
};

static
void
test_unsigned_mbn_decode_ok(
    gconstpointer data)
{
    const TestUnsignedMbnDecodeOk* test = data;
    GUtilRange range;
    guint64 value;

    range.ptr = test->input.bytes;
    range.end = range.ptr + test->input.size;
    g_assert(gutil_unsigned_mbn_decode(&range, NULL));
    g_assert(range.ptr == range.end);

    range.ptr = test->input.bytes;
    g_assert(gutil_unsigned_mbn_decode(&range, &value));
    g_assert_cmpuint(value, == ,test->output);
    g_assert(range.ptr == range.end);
}

/*==========================================================================*
 * unsigned_mbn/decode/fail
 *==========================================================================*/

static const guint8 test_unsigned_mbn_decode_fail_data_extra_bits[] = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0x7f
};
static const TestDecodeFail test_unsigned_mbn_decode_fail_data[] = {
#define TEST_CASE(x) TEST_("unsigned_mbn/decode/fail/") x
#define TEST_DATA(x) TEST_CASE(#x), \
    { TEST_ARRAY_AND_SIZE(test_mbn_decode_fail_data_##x) }
#define TEST_DATA_(x) TEST_CASE(#x), \
    { TEST_ARRAY_AND_SIZE(test_unsigned_mbn_decode_fail_data_##x) }
    { TEST_CASE("null"), { NULL, 0 } },
    { TEST_CASE("empty"), {test_mbn_decode_fail_data_short, 0} },
    { TEST_DATA(short) },
    { TEST_DATA(too_long) },
    { TEST_DATA_(extra_bits) }
#undef TEST_CASE
#undef TEST_DATA
#undef TEST_DATA_
};

static
void
test_unsigned_mbn_decode_fail(
    gconstpointer data)
{
    const TestDecodeFail* test = data;
    GUtilRange range;
    guint64 value;

    range.ptr = test->input.bytes;
    range.end = range.ptr + test->input.size;
    g_assert(!gutil_unsigned_mbn_decode(&range, &value));
    g_assert(range.ptr == test->input.bytes);
    g_assert(!gutil_unsigned_mbn_decode(&range, NULL));
    g_assert(range.ptr == test->input.bytes);
}

/*==========================================================================*
 * unsigned_mbn/decode2
 *==========================================================================*/

static
void
test_unsigned_mbn_decode2()
{
    static const guint8 trailing_garbage[] = { 0x12, 0x34 };
    const TestDecodeFail* fail = test_unsigned_mbn_decode_fail_data;
    const TestUnsignedMbnDecodeOk* ok = test_unsigned_mbn_decode_ok_data;
    guint64 value = 0xaaaaaaaa;
    GUtilData in;
    guint i;

    g_assert(!gutil_unsigned_mbn_decode2(NULL, NULL));
    g_assert(!gutil_unsigned_mbn_decode2(NULL, &value));

    in.bytes = trailing_garbage;
    in.size = sizeof(trailing_garbage);
    g_assert(!gutil_unsigned_mbn_decode2(&in, NULL));
    g_assert(!gutil_unsigned_mbn_decode2(&in, &value));


    /* Run only NULL, empty and short cases */
    for (i = 0; i < 3; i++) {
        g_assert(!gutil_unsigned_mbn_decode2(&fail[i].input, NULL));
        g_assert(!gutil_unsigned_mbn_decode2(&fail[i].input, &value));
    }

    g_assert(gutil_unsigned_mbn_decode2(&ok->input, NULL));
    g_assert(gutil_unsigned_mbn_decode2(&ok->input, &value));
    g_assert_cmpuint(value, == ,ok->output);
}

/*==========================================================================*
 * tlv/corner_cases
 *==========================================================================*/

static
void
test_tlv_corner_cases()
{
    guint8 buf[2];

    memset(buf, 0xaa, sizeof(buf));
    g_assert_cmpuint(gutil_tlv_encode(buf, 3, NULL), == ,2);
    g_assert_cmpuint(buf[0], == ,3);
    g_assert_cmpuint(buf[1], == ,0);
}

/*==========================================================================*
 * tlv/size
 *==========================================================================*/

typedef struct {
    const char* name;
    guint tag;
    gsize len;
    guint size;
} TestTlvSize;

static const TestTlvSize test_tlv_size_data[] = {
#define TEST_CASE(x) TEST_("tlv/size/") x
    { TEST_CASE("2"), 1, 0, 2 },
    { TEST_CASE("3"), 1, 1, 3 },
    { TEST_CASE("4"), 128, 1, 4 },
    { TEST_CASE("130"), 128, 127, 130 },
    { TEST_CASE("133"), 128, 129, 133 }
#undef TEST_CASE
};

static
void
test_tlv_size(
    gconstpointer data)
{
    const TestTlvSize* test = data;

    g_assert_cmpuint(gutil_tlv_size(test->tag, test->len), == ,test->size);
}

/*==========================================================================*
 * tlv/encode
 *==========================================================================*/

typedef struct {
    const char* name;
    guint tag;
    GUtilData in;
    GUtilData out;
} TestTlvEncodeDecode;

static const guint8 test_tlv_encode_data_out_0[] = { 0x01, 0x00 };
#define TEST_TLV_ENCODE_TAG_1 1
static const guint8 test_tlv_encode_data_in_1[] = { 0x01, 0x02, 0x03 };
static const guint8 test_tlv_encode_data_out_1[] = {
    0x01, 0x03, 0x01, 0x02, 0x03
};
static const TestTlvEncodeDecode test_tlv_encode_data[] = {
#define TEST_CASE(x) TEST_("tlv/encode/") x
#define TEST_DATA(x) TEST_CASE(#x), TEST_TLV_ENCODE_TAG_##x, \
    { TEST_ARRAY_AND_SIZE(test_tlv_encode_data_in_##x) }, \
    { TEST_ARRAY_AND_SIZE(test_tlv_encode_data_out_##x) }
    { TEST_CASE("0"), 1, { NULL, 0 }, {
      TEST_ARRAY_AND_SIZE(test_tlv_encode_data_out_0)} },
    { TEST_DATA(1) },
#undef TEST_CASE
#undef TEST_DATA
};

static
void
test_tlv_encode(
    gconstpointer data)
{
    const TestTlvEncodeDecode* test = data;
    const gsize size = test->out.size;
    void* buf = g_malloc(size);

    g_assert_cmpuint(gutil_tlv_size(test->tag, test->in.size), == , size);
    g_assert_cmpuint(gutil_tlv_encode(buf, test->tag, &test->in), == , size);
    g_assert(!memcmp(test->out.bytes, buf, size));
    g_free(buf);
}

/*==========================================================================*
 * tlv/decode/ok
 *==========================================================================*/

static const TestTlvEncodeDecode test_tlv_decode_ok_data[] = {
#define TEST_CASE(x) TEST_("tlv/decode/ok/") x
#define TEST_DATA(x) TEST_CASE(#x), TEST_TLV_ENCODE_TAG_##x, \
    { TEST_ARRAY_AND_SIZE(test_tlv_encode_data_out_##x) }, \
    { TEST_ARRAY_AND_SIZE(test_tlv_encode_data_in_##x) }
    { TEST_CASE("0"), 1, { TEST_ARRAY_AND_SIZE(test_tlv_encode_data_out_0) },
        { NULL, 0 } },
    { TEST_DATA(1) },
#undef TEST_CASE
#undef TEST_DATA
};

static
void
test_tlv_decode_ok(
    gconstpointer data)
{
    const TestTlvEncodeDecode* test = data;
    GUtilRange in;
    GUtilData val;

    in.ptr = test->in.bytes;
    in.end = in.ptr + test->in.size;
    g_assert_cmpuint(gutil_tlv_decode(&in, NULL), == ,test->tag);
    g_assert(in.ptr == in.end);

    in.ptr = test->in.bytes;
    in.end = in.ptr + test->in.size;
    g_assert_cmpuint(gutil_tlv_decode(&in, &val), == ,test->tag);
    g_assert_cmpuint(val.size, == ,test->out.size);
    g_assert(!memcmp(val.bytes, test->out.bytes, val.size));
    g_assert(in.ptr == in.end);
}

/*==========================================================================*
 * tlv/decode/fail
 *==========================================================================*/

static const guint8 test_tlv_decode_fail_data_tag_too_large[] =
    { 0x81, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f, 0x00 };
static const guint8 test_tlv_decode_fail_data_broken_len[] =
    { 0x01, 0xff };
static const guint8 test_tlv_decode_fail_data_truncated[] =
    { 0x01, 0x02, 0x00 };

static const TestDecodeFail test_tlv_decode_fail_data[] = {
#define TEST_CASE(x) TEST_("tlv/decode/fail/") x
#define TEST_DATA(x) TEST_CASE(#x), \
    { TEST_ARRAY_AND_SIZE(test_tlv_decode_fail_data_##x) }
    { TEST_CASE("null"), { NULL, 0 } },
    { TEST_CASE("empty"), {test_tlv_decode_fail_data_tag_too_large, 0} },
    { TEST_DATA(tag_too_large) },
    { TEST_DATA(broken_len) },
    { TEST_DATA(truncated) }
#undef TEST_CASE
#undef TEST_DATA
};

static
void
test_tlv_decode_fail(
    gconstpointer data)
{
    const TestDecodeFail* test = data;
    GUtilRange in;
    GUtilData value;

    in.ptr = test->input.bytes;
    in.end = in.ptr + test->input.size;
    g_assert(!gutil_tlv_decode(&in, &value));
    g_assert(in.ptr == test->input.bytes);

    g_assert(!gutil_tlv_decode(&in, NULL));
    g_assert(in.ptr == test->input.bytes);
}

/*==========================================================================*
 * tlvs/decode
 *==========================================================================*/

typedef struct {
    const char* name;
    GUtilData input;
    const guint* tags;
    GUTIL_TLVS_DECODE_FLAGS flags;
    int result;
    const GUtilData* values;
} TestTlvsDecode;


static const guint test_tlvs_decode_tags_1[] = { 1, 0 };
static const guint test_tlvs_decode_tags_1_2[] = { 1, 2, 0 };
static const guint test_tlvs_decode_tags_too_many[] = {
    1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
    11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
    21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
    31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50,
    51, 52, 53, 54, 55, 56, 57, 58, 59, 60,
    61, 62, 63, 64, 65, 66, 67, 68, 69, 70
};
static const guint8 test_tlvs_decode_input_dup[] = {
    0x01, 0x02, 0x03, 0x04,
    0x01, 0x00
};
static const guint8 test_tlvs_decode_input_tags_2_x[] = {
    0x02, 0x03, 0x04, 0x05, 0x06,
    0x7f, 0x00
};
static const guint8 test_tlvs_decode_input_tags_x[] = {
    0x7f, 0x00
};
static const guint8 test_tlvs_decode_input_garbage[] = {
    0x01, 0x02, 0x03, 0x04,
    0x05
};
static const guint8 test_tlvs_decode_output_2_data[] = { 0x04, 0x05, 0x06 };
static const GUtilData test_tlvs_decode_output_x_2[] = {
    { NULL, 0 },
    { TEST_ARRAY_AND_SIZE(test_tlvs_decode_output_2_data) }
};

static const TestTlvsDecode test_tlvs_decode_data[] = {
#define TEST_CASE(x) TEST_("tlvs/decode/") x
    {
        TEST_CASE("empty"), { (void*) test_tlvs_decode_data, 0 },
        test_tlvs_decode_tags_1, GUTIL_TLVS_DECODE_NO_FLAGS, 0, NULL
    },{
        TEST_CASE("dup"),
        { TEST_ARRAY_AND_SIZE(test_tlvs_decode_input_dup) },
        test_tlvs_decode_tags_1, GUTIL_TLVS_DECODE_NO_FLAGS,
        GUTIL_TLVS_DECODE_DUPLICATE_TAG, NULL
    },{
        TEST_CASE("unknown_tag/fail/1"),
        { TEST_ARRAY_AND_SIZE(test_tlvs_decode_input_tags_2_x) },
        test_tlvs_decode_tags_too_many, GUTIL_TLVS_DECODE_NO_FLAGS,
        GUTIL_TLVS_DECODE_UNKNOWN_TAG, NULL
    },{
        TEST_CASE("unknown_tag/fail/2"),
        { TEST_ARRAY_AND_SIZE(test_tlvs_decode_input_tags_2_x) },
        test_tlvs_decode_tags_1_2, GUTIL_TLVS_DECODE_NO_FLAGS,
        GUTIL_TLVS_DECODE_UNKNOWN_TAG, NULL
    },{
        TEST_CASE("unknown_tag/ok/1"),
        { TEST_ARRAY_AND_SIZE(test_tlvs_decode_input_tags_2_x) },
        test_tlvs_decode_tags_1_2, GUTIL_TLVS_DECODE_FLAG_SKIP_UNKNOWN_TAGS,
        0x02, test_tlvs_decode_output_x_2
    },{
        TEST_CASE("unknown_tag/ok/2"),
        { TEST_ARRAY_AND_SIZE(test_tlvs_decode_input_tags_x) },
        test_tlvs_decode_tags_1_2, GUTIL_TLVS_DECODE_FLAG_SKIP_UNKNOWN_TAGS,
        0, NULL
    },{
        TEST_CASE("garbage"),
        { TEST_ARRAY_AND_SIZE(test_tlvs_decode_input_garbage) },
        test_tlvs_decode_tags_1, GUTIL_TLVS_DECODE_NO_FLAGS,
        GUTIL_TLVS_DECODE_ERROR, NULL
    }
#undef TEST_CASE
};

static
void
test_tlvs_decode(
    gconstpointer data)
{
    const TestTlvsDecode* test = data;
    int i, n = 0;
    const guint* tag = test->tags;
    GUtilData* vals = NULL;

    while (*tag++) n++;
    if (n) {
        vals = g_new(GUtilData, n);
        memset(vals, 0xaa, sizeof(vals[0]) * n);
    }

    g_assert_cmpint(gutil_tlvs_decode(&test->input, test->tags, NULL,
        test->flags), == ,test->result);

    if (vals) {
        g_assert_cmpint(gutil_tlvs_decode(&test->input, test->tags, vals,
            test->flags), == ,test->result);
    }

    if (test->result > 0) {
        for (i = 0; i < n; i++) {
            if (test->result & (1 << i)) {
                const GUtilData* expected = test->values + i;

                g_assert_cmpuint(vals[i].size, == ,expected->size);
                g_assert(!memcmp(vals[i].bytes, test->values[i].bytes,
                    expected->size));
            } else {
                g_assert(!vals[i].bytes);
                g_assert_cmpuint(vals[i].size, == ,0);
            }
        }
    }
    g_free(vals);
}

/*==========================================================================*
 * Common
 *==========================================================================*/

#define TESTS_WITH_DATA(x) \
    do { int i; for (i = 0; i < G_N_ELEMENTS(x##_data); i++) {    \
        g_test_add_data_func(x##_data[i].name, x##_data + i, x); \
    } } while (FALSE)

int main(int argc, char* argv[])
{
    g_test_init(&argc, &argv, NULL);
    test_init(&test_opt, argc, argv);
    g_test_add_func(TEST_("mbn/corner_cases"), test_mbn_corner_cases);
    g_test_add_func(TEST_("tlv/corner_cases"), test_tlv_corner_cases);
    g_test_add_func(TEST_("signed_mbn/decode2"), test_signed_mbn_decode2);
    g_test_add_func(TEST_("unsigned_mbn/decode2"), test_unsigned_mbn_decode2);
    TESTS_WITH_DATA(test_signed_mbn_size);
    TESTS_WITH_DATA(test_signed_mbn_encode);
    TESTS_WITH_DATA(test_signed_mbn_decode_ok);
    TESTS_WITH_DATA(test_signed_mbn_decode_fail);
    TESTS_WITH_DATA(test_unsigned_mbn_size);
    TESTS_WITH_DATA(test_unsigned_mbn_encode);
    TESTS_WITH_DATA(test_unsigned_mbn_decode_ok);
    TESTS_WITH_DATA(test_unsigned_mbn_decode_fail);
    TESTS_WITH_DATA(test_tlv_size);
    TESTS_WITH_DATA(test_tlv_encode);
    TESTS_WITH_DATA(test_tlv_decode_ok);
    TESTS_WITH_DATA(test_tlv_decode_fail);
    TESTS_WITH_DATA(test_tlvs_decode);
    return g_test_run();
}

/*
 * Local Variables:
 * mode: C
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
