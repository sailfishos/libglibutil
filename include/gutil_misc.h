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

#ifndef GUTIL_MISC_H
#define GUTIL_MISC_H

#include "gutil_types.h"

G_BEGIN_DECLS

void
gutil_disconnect_handlers(
    gpointer instance,
    gulong* ids,
    int count);

void*
gutil_object_ref(
    void* object); /* Since 1.0.71 */

void
gutil_object_unref(
    void* object); /* Since 1.0.71 */

void*
gutil_hex2bin(
    const char* str,
    gssize len,
    void* data);

char*
gutil_bin2hex(
    const void* data,
    gsize len,
    gboolean upper_case) /* Since 1.0.71 */
    G_GNUC_WARN_UNUSED_RESULT;

char*
gutil_data2hex(
    const GUtilData* data,
    gboolean upper_case) /* Since 1.0.71 */
    G_GNUC_WARN_UNUSED_RESULT;

GBytes*
gutil_hex2bytes(
    const char* str,
    gssize len);

#define GUTIL_HEXDUMP_BUFSIZE  (70)
#define GUTIL_HEXDUMP_MAXBYTES (16)

guint
gutil_hexdump(
    char* buf,
    const void* data,
    guint size); /* Since 1.0.29 */

gboolean
gutil_parse_int(
    const char* str,
    int base,
    int* value); /* Since 1.0.30 */

gboolean
gutil_parse_uint(
    const char* str,
    int base,
    unsigned int* value); /* Since 1.0.53 */

gboolean
gutil_parse_int64(
    const char* str,
    int base,
    gint64* value); /* Since 1.0.56 */

gboolean
gutil_parse_uint64(
    const char* str,
    int base,
    guint64* value); /* Since 1.0.56 */

gboolean
gutil_data_equal(
    const GUtilData* data1,
    const GUtilData* data2); /* Since 1.0.31 */

gboolean
gutil_data_has_prefix(
    const GUtilData* data,
    const GUtilData* prefix); /* Since 1.0.38 */

gboolean
gutil_data_has_suffix(
    const GUtilData* data,
    const GUtilData* suffix); /* Since 1.0.38 */

const GUtilData*
gutil_data_from_string(
    GUtilData* data,
    const char* str); /* Since 1.0.31 */

const GUtilData*
gutil_data_from_bytes(
    GUtilData* data,
    GBytes* bytes); /* Since 1.0.31 */

GUtilData*
gutil_data_new(
    const void* bytes,
    guint len) /* Since 1.0.72 */
    G_GNUC_WARN_UNUSED_RESULT;

GUtilData*
gutil_data_copy(
    const GUtilData* data) /* Since 1.0.72 */
    G_GNUC_WARN_UNUSED_RESULT;

GBytes*
gutil_bytes_concat(
    GBytes* bytes,
    ...) /* Since 1.0.37 */
    G_GNUC_WARN_UNUSED_RESULT
    G_GNUC_NULL_TERMINATED;

GBytes*
gutil_bytes_xor(
    GBytes* bytes1,
    GBytes* bytes2); /* Since 1.0.37 */

gboolean
gutil_bytes_equal(
    GBytes* bytes,
    const void* data,
    gsize size); /* Since 1.0.41 */

gboolean
gutil_bytes_equal_data(
    GBytes* bytes,
    const GUtilData* data); /* Since 1.0.41 */

gboolean
gutil_bytes_has_prefix(
    GBytes* bytes,
    const void* data,
    gsize size); /* Since 1.0.63 */

gboolean
gutil_bytes_has_suffix(
    GBytes* bytes,
    const void* data,
    gsize size); /* Since 1.0.63 */

gsize
gutil_ptrv_length(
    const void* ptrv); /* Since 1.0.50 */

gboolean
gutil_ptrv_is_empty(
    const void* ptrv); /* Since 1.0.71 */

void
gutil_ptrv_free(
    void** ptrv); /* Since 1.0.51 */

void*
gutil_memdup(
    const void* ptr,
    gsize size); /* Since 1.0.52 */

gsize
gutil_strlen0(
    const char* str); /* Since 1.0.62 */

gsize
gutil_range_init_with_bytes(
    GUtilRange* range,
    GBytes* bytes); /* Since 1.0.55 */

gboolean
gutil_range_has_prefix(
    const GUtilRange* range,
    const GUtilData* prefix); /* Since 1.0.55 */

gboolean
gutil_range_skip_prefix(
    GUtilRange* range,
    const GUtilData* prefix); /* Since 1.0.55 */

G_END_DECLS

#endif /* GUTIL_MISC_H */

/*
 * Local Variables:
 * mode: C
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
