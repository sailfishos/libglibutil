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

#include "gutil_misc.h"

#include <glib-object.h>

#include <ctype.h>
#include <errno.h>
#include <limits.h>

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
gutil_object_ref(
    void* object) /* Since 1.0.71 */
{
    /* Just a NULL-tolerant version of g_object_ref() */
    if (object) {
        g_object_ref(object);
    }
    return object;
}

void
gutil_object_unref(
    void* object) /* Since 1.0.71 */
{
    /* Just a NULL-tolerant version of g_object_unref */
    if (object) {
        g_object_unref(object);
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

            if (g_ascii_isxdigit(x1) && g_ascii_isxdigit(x2)) {
                *ptr++ = (hex[x1-0x30] << 4) + hex[x2-0x30];
            } else {
                return NULL;
            }
        }
        return data;
    }
    return NULL;
}

char*
gutil_bin2hex(
    const void* data,
    gsize len,
    gboolean upper_case) /* Since 1.0.71 */
{
    static const char hex[] = "0123456789abcdef";
    static const char HEX[] = "0123456789ABCDEF";
    const char* map = upper_case ? HEX : hex;
    const guchar* ptr = data;
    const guchar* end = ptr + len;
    char* out = g_malloc(2 * len + 1);
    char* dest = out;

    while (ptr < end) {
        const guchar b = *ptr++;

        *dest++ = map[(b >> 4) & 0xf];
        *dest++ = map[b & 0xf];
    }

    *dest = 0;
    return out;
}

char*
gutil_data2hex(
    const GUtilData* data,
    gboolean upper_case) /* Since 1.0.71 */
{
    return data ? gutil_bin2hex(data->bytes, data->size, upper_case) : NULL;
}

GBytes*
gutil_hex2bytes(
    const char* str,
    gssize len)
{
    if (str) {
        if (len < 0) len = strlen(str);
        if (len > 0 && !(len & 1)) {
            const gsize n = len/2;
            void* data = g_malloc(n);

            if (gutil_hex2bin(str, len, data)) {
                return g_bytes_new_take(data, n);
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

static
const char*
gutil_strstrip(
    const char* str,
    char** tmp)
{
    /* Caller makes sure that str isn't NULL */
    const gsize len = strlen(str);

    if (g_ascii_isspace(str[0]) || g_ascii_isspace(str[len - 1])) {
        /* Need to modify the original string */
        return (*tmp = g_strstrip(gutil_memdup(str, len + 1)));
    } else {
        /* The original string is fine as is */
        return str;
    }
}

gboolean
gutil_parse_int(
    const char* str,
    int base,
    int* value) /* Since 1.0.30 */
{
    gint64 ll;

    if (gutil_parse_int64(str, base, &ll) && ll >= INT_MIN && ll <= INT_MAX) {
        if (value) {
            *value = (int)ll;
        }
        return TRUE;
    }
    return FALSE;
}

gboolean
gutil_parse_uint(
    const char* str,
    int base,
    unsigned int* value) /* Since 1.0.53 */
{
    guint64 ull;

    if (gutil_parse_uint64(str, base, &ull) && ull <= UINT_MAX) {
        if (value) {
            *value = (unsigned int)ull;
        }
        return TRUE;
    }
    return FALSE;
}

gboolean
gutil_parse_int64(
    const char* str,
    int base,
    gint64* value) /* Since 1.0.56 */
{
    gboolean ok = FALSE;

    if (str && *str) {
        char* tmp = NULL;
        char* end = NULL;
        const char* stripped = gutil_strstrip(str, &tmp);
        gint64 ll;

        errno = 0;
        ll = g_ascii_strtoll(stripped, &end, base);
        if (end && !*end &&
            !((ll == G_MAXINT64 || ll == G_MININT64) && errno == ERANGE) &&
            !(ll == 0 && errno == EINVAL)) {
            if (value) {
                *value = ll;
            }
            ok = TRUE;
        }
        g_free(tmp);
    }
    return ok;
}

gboolean
gutil_parse_uint64(
    const char* str,
    int base,
    guint64* value) /* Since 1.0.56 */
{
    gboolean ok = FALSE;

    /*
     * Sorry, we don't accept minus as a part of an unsigned number
     * (unlike strtoul)
     */
    if (str && *str && *str != '-') {
        char* tmp = NULL;
        const char* stripped = gutil_strstrip(str, &tmp);

        if (*stripped != '-') {
            char* end = NULL;
            guint64 ull;

            errno = 0;
            ull = g_ascii_strtoull(stripped, &end, base);
            if (end && !*end &&
                !(ull == G_MAXUINT64 && errno == ERANGE) &&
                !(ull == 0 && errno == EINVAL)) {
                if (value) {
                    *value = ull;
                }
                ok = TRUE;
            }
        }
        g_free(tmp);
    }
    return ok;
}

gboolean
gutil_data_equal(
    const GUtilData* data1,
    const GUtilData* data2) /* Since 1.0.31 */
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
    const char* str) /* Since 1.0.31 */
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
    GBytes* bytes) /* Since 1.0.31 */
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

GUtilData*
gutil_data_new(
    const void* bytes,
    guint len) /* Since 1.0.72 */
{
    /*
     * The whole thing is allocated from a single memory block and
     * has to be freed with a single g_free()
     */
    const gsize total = len + sizeof(GUtilData);
    GUtilData* data = g_malloc(total);

    if (bytes) {
        void* contents = (void*)(data + 1);

        data->bytes = contents;
        data->size = len;
        memcpy(contents, bytes, len);
    } else {
        memset(data, 0, sizeof(*data));
    }
    return data;
}

GUtilData*
gutil_data_copy(
    const GUtilData* data) /* Since 1.0.72 */
{
    /*
     * The whole thing is allocated from a single memory block and
     * has to be freed with a single g_free()
     */
    return data ? gutil_data_new(data->bytes, data->size) : NULL;
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

gboolean
gutil_bytes_has_prefix(
    GBytes* bytes,
    const void* data,
    gsize size) /* Since 1.0.63 */
{
    if (!bytes) {
        /* NULL GBytes has neither prefix nor suffix, even an empty one */
        return FALSE;
    } else if (!size) {
        /*
         * That's largely a philosophical question - can anything have
         * an empty prefix? Let's assume that the answer is yes. And
         * then if anything can have such a prefix, everything has it.
         * Right? Except for NULL GBytes which doesn't have anything
         * as said earlier.
         */
        return TRUE;
    } else {
        gsize bytes_size;
        const guint8* contents = g_bytes_get_data(bytes, &bytes_size);

        return (bytes_size >= size) && !memcmp(contents, data, size);
    }
}

gboolean
gutil_bytes_has_suffix(
    GBytes* bytes,
    const void* data,
    gsize size) /* Since 1.0.63 */
{
    /* Treat an empty suffix the same way as an empty prefix */
    if (!bytes) {
        return FALSE;
    } else if (!size) {
        return TRUE;
    } else {
        gsize bytes_size;
        const guint8* contents = g_bytes_get_data(bytes, &bytes_size);

        return (bytes_size >= size) &&
            !memcmp(contents + (bytes_size - size), data, size);
    }
}

/* Calculates the length of NULL-terminated array of pointers */
gsize
gutil_ptrv_length(
    const void* ptrv) /* Since 1.0.50 */
{
    if (ptrv) {
        gsize len = 0;
        const gconstpointer* ptr = ptrv;

        while (*ptr++) len++;
        return len;
    } else {
        return 0;
    }
}

gboolean
gutil_ptrv_is_empty(
    const void* ptrv) /* Since 1.0.71 */
{
    return !ptrv || !((gconstpointer*)ptrv)[0];
}

/* Frees NULL-terminated array of pointers and whatever they're pointing to. */
void
gutil_ptrv_free(
    void** ptrv) /* Since 1.0.51 */
{
    if (G_LIKELY(ptrv)) {
        void** ptr = ptrv;

        while (*ptr) g_free(*ptr++);
        g_free(ptrv);
    }
}

/* Similar to g_memdup but takes gsize as the number of bytes to copy */
void*
gutil_memdup(
    const void* ptr,
    gsize size) /* Since 1.0.52 */
{
    if (G_LIKELY(ptr) && G_LIKELY(size)) {
        void* copy = g_malloc(size);

        memcpy(copy, ptr, size);
        return copy;
    } else {
        return NULL;
    }
}

/* NULL-tolerant version of strlen */
gsize
gutil_strlen0(
    const char* str) /* Since 1.0.62 */
{
    return str ? strlen(str) : 0;
}

gsize
gutil_range_init_with_bytes(
    GUtilRange* range,
    GBytes* bytes) /* Since 1.0.55 */
{
    gsize size = 0;

    if (G_LIKELY(range)) {
        if (G_LIKELY(bytes)) {
            range->ptr = (const guint8*) g_bytes_get_data(bytes, &size);
            range->end = range->ptr + size;
        } else {
            memset(range, 0, sizeof(*range));
        }
    }
    return size;
}

gboolean
gutil_range_has_prefix(
    const GUtilRange* range,
    const GUtilData* prefix) /* Since 1.0.55 */
{
    if (G_LIKELY(range) && G_LIKELY(range->ptr) && G_LIKELY(prefix)) {
        if (range->end > range->ptr) {
            return (gsize)(range->end - range->ptr) >= prefix->size &&
                !memcmp(range->ptr, prefix->bytes, prefix->size);
        } else {
            return !prefix->size;
        }
    }
    return FALSE;
}

gboolean
gutil_range_skip_prefix(
    GUtilRange* range,
    const GUtilData* prefix) /* Since 1.0.55 */
{
    if (gutil_range_has_prefix(range, prefix)) {
        range->ptr += prefix->size;
        return TRUE;
    }
    return FALSE;
}

/*
 * Local Variables:
 * mode: C
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
