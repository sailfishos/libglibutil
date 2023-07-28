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

#include "gutil_datapack.h"

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
    gint64 value)
{
    guint n = 1, msc = (value & 0x7f); /* Most Significant Chunk */

    value >>= 7;
    if (value < 0) {
        while (value != -1) {
            msc = (value & 0x7f);
            value >>= 7;
            n++;
        }
        if (!(msc & 0x40)) {
            n++; /* Sign bit didn't fit */
        }
    } else {
        while (value) {
            msc = (value & 0x7f);
            value >>= 7;
            n++;
        }
        if (msc & 0x40) {
            n++; /* Sign bit didn't fit */
        }
    }
    return n;
}

guint
gutil_signed_mbn_encode(
    gpointer buf,
    gint64 value)
{
    return gutil_signed_mbn_encode2(buf, value,
        gutil_signed_mbn_size(value));
}

guint
gutil_signed_mbn_encode2(
    gpointer buf,
    gint64 value,
    guint size)
{
    guchar* ptr = buf;

    /* Zero or too large size must be a programming error.  */
    if (size > 0) {
        guint n = size;

        ptr[--n] = (guchar)(value & 0x7f);
        value >>= 7;
        while (n) {
            ptr[--n] = (guchar)(value | 0x80);
            value >>= 7;
        }

        if (value < 0) {
            const guint maxbits = sizeof(value) * 8;

            /* Special treatment of negative numbers */
            if (size * 7 > maxbits) {
                /* Extend the sign bit */
                ptr[0] |= ~((1 << (maxbits - (size - 1) * 7)) - 1);
            }
        }
    }
    return size;
}

gboolean
gutil_signed_mbn_decode(
    GUtilRange* in,
    gint64* out)
{
    if (in->ptr < in->end) {
        guchar last = in->ptr[0];

        if (last & 0x80) {
            const int maxbits = sizeof(*out) * 8;
            const guchar msc = last; /* Most Significant Chunk */
            int nbits = 7;
            guint off = 1;
            guint64 value = (last & 0x7f);

            while ((in->ptr + off) < in->end &&
                   ((last = in->ptr[off++]) & 0x80)) {
                value = (value << 7) | (last & 0x7f);
                if ((nbits + 7) <= maxbits) {
                    nbits += 7;
                } else {
                    /* Too many bytes */
                    return FALSE;
                }
            }

            if (!(last & 0x80)) {
                value = (value << 7) | last;
                if (msc & 0x40) {
                    /* Negative number */
                    if ((nbits + 7) < maxbits) {
                        /* Extend the sign bit */
                        if (out) {
                            *out = value | ~((G_GINT64_CONSTANT(1) <<
                                (nbits + 7)) - 1);
                        }
                        in->ptr += off;
                        return TRUE;
                    } else {
                        /* Unused bits must be set to 1 */
                        if ((msc | ((1 << (maxbits - nbits)) - 1)) == 0xff) {
                            if (out) {
                                *out = value;
                            }
                            in->ptr += off;
                            return TRUE;
                        }
                    }
                } else {
                    /* Positive number, unused bits (if any) must be zeroed */
                    /* Unused bits must be zeroed */
                    if ((nbits + 7) < maxbits ||
                        (msc & ~((1 << (maxbits - nbits)) - 1)) == 0x80) {
                        if (out) {
                            *out = value;
                        }
                        in->ptr += off;
                        return TRUE;
                    }
                }
            }
            /* Broken sequence */
        } else {
            /* Single byte, very common case */
            if (out) {
                if ((*out = *in->ptr++) & 0x40) {
                    /* Small negative number */
                    *out |= (G_GINT64_CONSTANT(-1) ^ 0x7f);
                }
            } else {
                in->ptr++;
            }
            return TRUE;
        }
    }
    return FALSE;
}

gboolean
gutil_signed_mbn_decode2(
    const GUtilData* data,
    gint64* out)
{
    if (data && data->size) {
        GUtilRange in;
        gint64 value;

        in.end = (in.ptr = data->bytes) + data->size;
        if (gutil_signed_mbn_decode(&in, &value) && in.ptr == in.end) {
            if (out) {
                *out = value;
            }
            return TRUE;
        }
    }
    return FALSE;
}

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
    guint64 value)
{
    guint n;

    value >>= 7;
    for (n = 1; value; n++) {
        value >>= 7;
    }
    return n;
}

guint
gutil_unsigned_mbn_encode(
    gpointer buf,
    guint64 value)
{
    return gutil_unsigned_mbn_encode2(buf, value,
        gutil_unsigned_mbn_size(value));
}

guint
gutil_unsigned_mbn_encode2(
    gpointer buf,
    guint64 value,
    guint size)
{
    guchar* ptr = buf;

    /* Zero or too large size must be a programming error.  */
    if (size > 0) {
        guint n = size;

        ptr[--n] = (guchar)(value & 0x7f);
        value >>= 7;
        while (n) {
            ptr[--n] = (guchar)(value | 0x80);
            value >>= 7;
        }
    }
    return size;
}

gboolean
gutil_unsigned_mbn_decode(
    GUtilRange* in,
    guint64* out)
{
    if (in->ptr < in->end) {
        guchar last = in->ptr[0];

        if (last & 0x80) {
            const int maxbits = sizeof(*out) * 8;
            const guchar msc = last; /* Most Significant Chunk */
            int nbits = 7;
            guint off = 1;
            guint64 value = (last & 0x7f);

            while ((in->ptr + off) < in->end &&
                   ((last = in->ptr[off++]) & 0x80)) {
                value = (value << 7) | (last & 0x7f);
                if ((nbits + 7) <= maxbits) {
                    nbits += 7;
                } else {
                    /* Too many bytes */
                    return FALSE;
                }
            }

            if (!(last & 0x80)) {
                value = (value << 7) | last;
                /* Unused bits must be zeroed */
                if ((nbits + 7) < maxbits ||
                    (msc & ~((1 << (maxbits - nbits)) - 1)) == 0x80) {
                    if (out) {
                        *out = value;
                    }
                    in->ptr += off;
                    return TRUE;
                }
            }
            /* Broken sequence */
        } else {
            /* Single byte, very common case */
            if (out) {
                *out = *in->ptr++;
            } else {
                in->ptr++;
            }
            return TRUE;
        }
    }
    return FALSE;
}

gboolean
gutil_unsigned_mbn_decode2(
    const GUtilData* data,
    guint64* out)
{
    if (data && data->size) {
        GUtilRange in;
        guint64 value;

        in.end = (in.ptr = data->bytes) + data->size;
        if (gutil_unsigned_mbn_decode(&in, &value) && in.ptr == in.end) {
            if (out) {
                *out = value;
            }
            return TRUE;
        }
    }
    return FALSE;
}

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

guint
gutil_tlv_size(
    guint tag,
    gsize len)
{
    return gutil_unsigned_mbn_size(tag) + gutil_unsigned_mbn_size(len) + len;
}

gsize
gutil_tlv_encode(
    gpointer buf,
    guint tag,
    const GUtilData* val)
{
    guchar* ptr = buf;
    const guint tag_size = gutil_unsigned_mbn_encode(ptr, tag);

    ptr += tag_size;
    if (val && val->size) {
        const guint len_size = gutil_unsigned_mbn_encode(ptr, val->size);

        memcpy(ptr + len_size, val->bytes, val->size);
        return tag_size + len_size + val->size;
    } else {
        ptr[0] = 0;
        return tag_size + 1;
    }
}

guint
gutil_tlv_decode(
    GUtilRange* in,
    GUtilData* val)
{
    GUtilRange tmp = *in;
    guint64 tag, len;

    if (gutil_unsigned_mbn_decode(&tmp, &tag) && tag <= INT_MAX &&
        gutil_unsigned_mbn_decode(&tmp, &len) && (tmp.end - tmp.ptr) >= len) {
        if (val) {
            val->bytes = tmp.ptr;
            val->size = (gsize) len;
        }
        in->ptr = tmp.ptr + len;
        return (guint) tag;
    }
    return 0;
}

int
gutil_tlvs_decode(
    const GUtilData* in,
    const guint* tags,
    GUtilData* vals,
    GUTIL_TLVS_DECODE_FLAGS flags)
{
    /* No more than first 31 tags from the tags array are considered */
    const int max_tags = sizeof(int) * 8 - 1;
    int mask = 0;
    GUtilRange range;
    GUtilData val;
    guint tag;

    range.end = (range.ptr = in->bytes) + in->size;

    if (vals) {
        int n = 0;
        const guint* tag = tags;

        /* Clear the output values */
        while (*tag++) n++;
        memset(vals, 0, sizeof(vals[0]) * n);
    }

    while ((tag = gutil_tlv_decode(&range, &val)) != 0) {
        int i, bit = 0;

        for (i = 0; tags[i] && i < max_tags; i++) {
            if (tags[i] == tag) {
                bit = 1 << i;
                break;
            }
        }

        if (bit) {
            if (mask & bit) {
                return GUTIL_TLVS_DECODE_DUPLICATE_TAG;
            } else {
                mask |= bit;
                if (vals) {
                    vals[i] = val;
                }
            }
        } else if (!(flags & GUTIL_TLVS_DECODE_FLAG_SKIP_UNKNOWN_TAGS)) {
            return GUTIL_TLVS_DECODE_UNKNOWN_TAG;
        }
    }

    return (range.ptr == range.end) ? mask : GUTIL_TLVS_DECODE_ERROR;
}

/*
 * Local Variables:
 * mode: C
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
