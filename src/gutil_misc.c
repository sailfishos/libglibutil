/*
 * Copyright (C) 2016-2020 Jolla Ltd.
 * Copyright (C) 2016-2020 Slava Monich <slava.monich@jolla.com>
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

#include "gutil_misc.h"

#include <glib-object.h>

#include <ctype.h>
#include <limits.h>
#include <errno.h>

void
gutil_disconnect_handlers(
    gpointer instance,
    gulong *ids,
    int count)
{
    if (G_LIKELY(instance) && G_LIKELY(ids)) {
        int i;
        for (i=0; i<count; i++) {
            if (ids[i]) {
                g_signal_handler_disconnect(instance, ids[i]);
                ids[i] = 0;
            }
        }
    }
}

void*
gutil_hex2bin(
    const char* str,
    gssize len,
    void* data)
{
    if (str && data && len > 0 && !(len & 1)) {
        gssize i;
        guint8* ptr = data;
        for (i=0; i<len; i+=2) {
            static const guint8 hex[] = {
                0, 1, 2, 3, 4, 5, 6, 7,     /* 0x30..0x37 */
                8, 9, 0, 0, 0, 0, 0, 0,     /* 0x3a..0x3f */
                0,10,11,12,13,14,15, 0,     /* 0x40..0x47 */
                0, 0, 0, 0, 0, 0, 0, 0,     /* 0x4a..0x4f */
                0, 0, 0, 0, 0, 0, 0, 0,     /* 0x40..0x47 */
                0, 0, 0, 0, 0, 0, 0, 0,     /* 0x5a..0x5f */
                0,10,11,12,13,14,15         /* 0x60..0x66 */
            };
            const char x1 = str[i];
            const char x2 = str[i+1];
            if (isxdigit(x1) && isxdigit(x2)) {
                *ptr++ = (hex[x1-0x30] << 4) + hex[x2-0x30];
            } else {
                return NULL;
            }
        }
        return data;
    }
    return NULL;
}

GBytes*
gutil_hex2bytes(
    const char* str,
    gssize len)
{
    if (str) {
        if (len < 0) len = strlen(str);
        if (len > 0 && !(len & 1)) {
            void* data = g_malloc(len/2);
            if (gutil_hex2bin(str, len, data)) {
                return g_bytes_new_take(data, len/2);
            }
            g_free(data);
        }
    }
    return NULL;
}

/**
 * The caller makes sure that the destination buffer has at least
 * GUTIL_HEXDUMP_BUFSIZE bytes available. Returns the number of
 * bytes actually dumped (no more than GUTIL_HEXDUMP_MAXBYTES).
 *
 * Since 1.0.29
 */
guint
gutil_hexdump(
    char* buf,
    const void* data,
    guint len)
{
    static const char hex[] = "0123456789abcdef";
    const guint bytes_dumped = MIN(len, GUTIL_HEXDUMP_MAXBYTES);
    const guchar* bytes = data;
    char* ptr = buf;
    guint i;

    for (i=0; i<GUTIL_HEXDUMP_MAXBYTES; i++) {
        if (i > 0) {
            *ptr++ = ' ';
            if (i == 8) *ptr++ = ' ';
        }
        if (i < len) {
            const guchar b = bytes[i];
            *ptr++ = hex[(b >> 4) & 0xf];
            *ptr++ = hex[b & 0xf];
        } else {
            *ptr++ = ' ';
            *ptr++ = ' ';
        }
    }

    *ptr++ = ' ';
    *ptr++ = ' ';
    *ptr++ = ' ';
    *ptr++ = ' ';
    for (i=0; i<bytes_dumped; i++) {
        const char c = bytes[i];
        if (i == 8) *ptr++ = ' ';
        *ptr++ = isprint(c) ? c : '.';
    }

    *ptr++ = 0;
    return bytes_dumped;
}

/* Since 1.0.30 */
gboolean
gutil_parse_int(
    const char* str,
    int base,
    int* value)
{
    gboolean ok = FALSE;

    if (str && str[0]) {
        char* str2 = g_strstrip(g_strdup(str));
        char* end = str2;
        gint64 l;

        errno = 0;
        l = g_ascii_strtoll(str2, &end, base);
        ok = !*end && errno != ERANGE && l >= INT_MIN && l <= INT_MAX;
        if (ok && value) {
            *value = (int)l;
        }
        g_free(str2);
    }
    return ok;
}

/* since 1.0.31 */
gboolean
gutil_data_equal(
    const GUtilData* data1,
    const GUtilData* data2)
{
    if (data1 == data2) {
        return TRUE;
    } else if (!data1 || !data2) {
        return FALSE;
    } else if (data1->size == data2->size) {
        return !memcmp(data1->bytes, data2->bytes, data1->size);
    } else {
        return FALSE;
    }
}

const GUtilData*
gutil_data_from_string(
    GUtilData* data,
    const char* str)
{
    if (data) {
        if (str) {
            data->bytes = (const void*)str;
            data->size = strlen(str);
        } else {
            data->bytes = NULL;
            data->size = 0;
        }
    }
    return data;
}

const GUtilData*
gutil_data_from_bytes(
    GUtilData* data,
    GBytes* bytes)
{
    if (data) {
        if (bytes) {
            data->bytes = g_bytes_get_data(bytes, &data->size);
        } else {
            data->bytes = NULL;
            data->size = 0;
        }
    }
    return data;
}

gboolean
gutil_data_has_prefix(
    const GUtilData* data,
    const GUtilData* prefix) /* Since 1.0.38 */
{
    /*
     * Not that it was overly important, but let's postulate that
     * NULL begins with NULL, empty block begins with empty block
     * but NULL doesn't begin with empty block and empty block
     * doesn't begin with NULL.
     */
    if (G_LIKELY(data)) {
        return G_LIKELY(prefix) &&
            prefix->size <= data->size &&
            !memcmp(data->bytes, prefix->bytes, prefix->size);
    } else {
        return !prefix;
    }
}

gboolean
gutil_data_has_suffix(
    const GUtilData* data,
    const GUtilData* suffix) /* Since 1.0.38 */
{
    /*
     * Similarly to gutil_data_has_prefix, NULL ends with NULL, empty
     * block ends with empty block but NULL doesn't end with empty block
     * and empty block doesn't end with NULL.
     */
    if (G_LIKELY(data)) {
        return G_LIKELY(suffix) &&
            suffix->size <= data->size &&
            !memcmp(data->bytes + (data->size - suffix->size),
                suffix->bytes, suffix->size);
    } else {
        return !suffix;
    }
}

GBytes*
gutil_bytes_concat(
    GBytes* bytes1,
    ...) /* Since 1.0.37 */
{
    if (G_LIKELY(bytes1)) {
        va_list args;
        gsize size = g_bytes_get_size(bytes1);
        gsize total = size;
        guint non_empty_count;
        GBytes* non_empty;
        GBytes* b;

        if (!size) {
            non_empty_count = 0;
            non_empty = NULL;
        } else {
            non_empty_count = 1;
            non_empty = bytes1;
        }

        va_start(args, bytes1);
        b = va_arg(args, GBytes*);
        while (b) {
            size = g_bytes_get_size(b);
            total += size;
            if (size) {
                non_empty_count++;
                non_empty = b;
            }
            b = va_arg(args, GBytes*);
        }
        va_end(args);

        if (non_empty_count == 0) {
            /* All arrays are empty */
            return g_bytes_ref(bytes1);
        } else if (non_empty_count == 1) {
            /* Only one array is non-empty */
            return g_bytes_ref(non_empty);
        } else {
            /* We actually need to concatenate something */
            guint8* buf = g_malloc(total);
            guint8* dest;
            gsize size;
            const void* src = g_bytes_get_data(bytes1, &size);

            memcpy(buf, src, size);
            dest = buf + size;

            va_start(args, bytes1);
            b = va_arg(args, GBytes*);
            while (b) {
                src = g_bytes_get_data(b, &size);
                memcpy(dest, src, size);
                dest += size;
                b = va_arg(args, GBytes*);
            }
            va_end(args);

            return g_bytes_new_take(buf, total);
        }
    }
    return NULL;
}

GBytes*
gutil_bytes_xor(
    GBytes* bytes1,
    GBytes* bytes2) /* Since 1.0.37 */
{
    if (G_LIKELY(bytes1) && G_LIKELY(bytes2)) {
        gsize size1, size2;
        const guint8* data1 = g_bytes_get_data(bytes1, &size1);
        const guint8* data2 = g_bytes_get_data(bytes2, &size2);
        const gsize size = MIN(size1, size2);

        if (G_LIKELY(size)) {
            gsize i;
            guint8* xor_data = g_malloc(size);
            unsigned long* xor_long = (unsigned long*)xor_data;
            const unsigned long* long1 = (const unsigned long*)data1;
            const unsigned long* long2 = (const unsigned long*)data2;
            const guint8* tail1;
            const guint8* tail2;
            guint8* xor_tail;

            /* Optimize by XOR-ing entire words */
            for (i = 0; (i + sizeof(*long1)) <= size; i += sizeof(*long1)) {
                *xor_long++ = (*long1++) ^ (*long2++);
            }

            /* Finish the process byte-by-byte */
            tail1 = (const guint8*)long1;
            tail2 = (const guint8*)long2;
            xor_tail = (guint8*)xor_long;
            for(; i < size; i++) {
                *xor_tail++ = (*tail1++) ^ (*tail2++);
            }
            return g_bytes_new_take(xor_data, size);
        }
        return g_bytes_ref(size1 ? bytes2 : bytes1);
    }
    return NULL;
}

gboolean
gutil_bytes_equal(
    GBytes* bytes,
    const void* data,
    gsize size) /* Since 1.0.41 */
{
    if (bytes && data) {
        gsize bytes_size;
        const guint8* contents = g_bytes_get_data(bytes, &bytes_size);

        if (bytes_size == size) {
            return !bytes_size || !memcmp(contents, data, size);
        } else {
            return FALSE;
        }
    } else {
        /* NULLs are equal to each other but not to anything else */
        return !bytes && !data;
    }
}

gboolean
gutil_bytes_equal_data(
    GBytes* bytes,
    const GUtilData* data) /* Since 1.0.41 */
{
    if (bytes && data) {
        gsize bytes_size;
        const guint8* contents = g_bytes_get_data(bytes, &bytes_size);

        if (bytes_size == data->size) {
            return !bytes_size || !memcmp(contents, data->bytes, data->size);
        } else {
            return FALSE;
        }
    } else {
        /* NULLs are equal to each other but not to anything else */
        return !bytes && !data;
    }
}

/* Calculates the length of NULL-terminated array of pointers */
gsize
gutil_ptrv_length(
    gconstpointer ptrv) /* Since 1.0.50 */
{
    if (G_LIKELY(ptrv)) {
        gsize len = 0;
        const gconstpointer* ptr = ptrv;

        while (*ptr++) len++;
        return len;
    } else {
        return 0;
    }
}

/*
 * Local Variables:
 * mode: C
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
