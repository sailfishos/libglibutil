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

#ifndef GUTIL_DATAPACK_H
#define GUTIL_DATAPACK_H

#include "gutil_types.h"

/*
 * Utilities for packing and unpacking simple data types and structures.
 *
 * Since 1.0.69
 */

G_BEGIN_DECLS

/*
 * Compact 7 bits per byte representation of a signed integer with
 * redundant sign bits removed. Most significant bit is the continuation
 * flag. Bit 0x40 in the first (most significant) byte is the sign bit.
 * Unused bits in the most significant chunk must be filled with the
 * sign bit.
 *
 * For positive numbers it's slightly less efficient than encoding unsigned
 * numbers because of one extra sign bit (e.g. unsigned number 65 can be
 * squeezed into a single byte but the signed one takes 2 bytes).
 *
 * Examples:
 *
 *  33  (0x0000000000000021) => 00100001 (0x21)
 * -33  (0xffffffffffffffdf) => 01011111 (0x5f)
 *  65  (0x0000000000000041) => 10000000 01000001 (0x80 0x41)
 * -65  (0xffffffffffffff3f) => 11111111 00111111 (0xff 0x3f)
 *  129 (0x0000000000000081) => 10000001 00000001 (0x81 0x01)
 * -129 (0xffffffffffffff7f) => 11111110 01111110 (0xfe 0x7f)
 */

guint
gutil_signed_mbn_size(
    gint64 value);

guint
gutil_signed_mbn_encode(
    gpointer buf,
    gint64 value);

guint
gutil_signed_mbn_encode2(
    gpointer buf,
    gint64 value,
    guint value_size);

gboolean
gutil_signed_mbn_decode(
    GUtilRange* in,
    gint64* out);

gboolean
gutil_signed_mbn_decode2(
    const GUtilData* in,
    gint64* out);

/*
 * Compact 7 bits per byte representation of an unsigned integer with
 * redundant zero bits removed. Most significant bit is the continuation
 * flag. Unused bits in the first byte (most significant chunk) must be
 * filled with zeros.
 *
 * Examples:
 *
 * 33  (0x0000000000000021) => 00100001 (0x21)
 * 65  (0x0000000000000041) => 01000001 (0x41)
 * 129 (0x0000000000000081) => 10000001 00000001 (0x81 0x01)
 */

guint
gutil_unsigned_mbn_size(
    guint64 value);

guint
gutil_unsigned_mbn_encode(
    gpointer buf,
    guint64 value);

guint
gutil_unsigned_mbn_encode2(
    gpointer buf,
    guint64 value,
    guint value_size);

gboolean
gutil_unsigned_mbn_decode(
    GUtilRange* in,
    guint64* out);

gboolean
gutil_unsigned_mbn_decode2(
    const GUtilData* in,
    guint64* out);

/*
 * TLV is a convenient and extendible way to pack various kinds of data
 * into a single memory block. Each entry gets encoded as follows:
 *
 * +---
 * |  T (unsigned MBN) : tag (non-zero)
 * +---
 * |  L (unsigned MBN) : length (zero if there's no data)
 * +---
 * |  V (L bytes) : tag specific data (optional)
 * +---
 *
 * To make API even more convenient, these utilities restrict the tag
 * value to INT_MAX which should be enough in most real life situations.
 * Also, these utilities assume that tags are non-zero. gutil_tlv_decode()
 * returns zero ifno TLV can be pulled out of the input data, and the
 * tags array passed to gutil_tlvs_decode() is zero terminated.
 */

typedef enum gutil_tlvs_decode_flags {
    GUTIL_TLVS_DECODE_NO_FLAGS = 0,
    GUTIL_TLVS_DECODE_FLAG_SKIP_UNKNOWN_TAGS = 0x1
} GUTIL_TLVS_DECODE_FLAGS;

guint
gutil_tlv_size(
    guint tag,
    gsize len);

gsize
gutil_tlv_encode(
    gpointer buf,
    guint tag,
    const GUtilData* val);

guint
gutil_tlv_decode(
    GUtilRange* in,
    GUtilData* val);

#define GUTIL_TLVS_DECODE_ERROR (-1)
#define GUTIL_TLVS_DECODE_DUPLICATE_TAG (-2)
#define GUTIL_TLVS_DECODE_UNKNOWN_TAG (-3)

int
gutil_tlvs_decode(
    const GUtilData* in,
    const guint* tags,
    GUtilData* vals,
    GUTIL_TLVS_DECODE_FLAGS flags);

G_END_DECLS

#endif /* GUTIL_DATAPACK_H */

/*
 * Local Variables:
 * mode: C
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
