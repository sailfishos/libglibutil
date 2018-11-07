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

#ifndef GUTIL_IDLEPOOL_H
#define GUTIL_IDLEPOOL_H

#include "gutil_types.h"

G_BEGIN_DECLS

/*
 * This is a glib equivalent of NSAutoreleasePool. Its purpose is to
 * temporarily hold a reference, until the next idle callback or until
 * the pool is drained manually with gutil_idle_pool_drain().
 *
 * Note that the following functions don't add references, they hold
 * the references which you have created:
 *
 * gutil_idle_pool_add_object()
 * gutil_idle_pool_add_variant()
 * gutil_idle_pool_add_ptr_array()
 * gutil_idle_pool_add_bytes()
 *
 * The following functions, however, do add the reference and hold it
 * until the pool is drained:
 *
 * gutil_idle_pool_add_object_ref()
 * gutil_idle_pool_add_variant_ref()
 * gutil_idle_pool_add_ptr_array_ref()
 * gutil_idle_pool_add_bytes_ref()
 */

GUtilIdlePool*
gutil_idle_pool_new(void);

GUtilIdlePool*
gutil_idle_pool_get(
    GUtilIdlePool** shared);

GUtilIdlePool*
gutil_idle_pool_ref(
    GUtilIdlePool* pool);

void
gutil_idle_pool_unref(
    GUtilIdlePool* pool);

void
gutil_idle_pool_destroy(
    GUtilIdlePool* pool); /* Since 1.0.34 */

void
gutil_idle_pool_drain(
    GUtilIdlePool* pool);

void
gutil_idle_pool_add(
    GUtilIdlePool* pool,
    gpointer pointer,
    GDestroyNotify destroy);

void
gutil_idle_pool_add_strv(
    GUtilIdlePool* pool,
    char** strv); /* Since 1.0.32 */

void
gutil_idle_pool_add_object(
    GUtilIdlePool* pool,
    gpointer object);

void
gutil_idle_pool_add_variant(
    GUtilIdlePool* pool,
    GVariant* variant);

void
gutil_idle_pool_add_ptr_array(
    GUtilIdlePool* pool,
    GPtrArray* array);

void
gutil_idle_pool_add_bytes(
    GUtilIdlePool* pool,
    GBytes* bytes); /* Since 1.0.34 */

void
gutil_idle_pool_add_object_ref(
    GUtilIdlePool* pool,
    gpointer object);

void
gutil_idle_pool_add_variant_ref(
    GUtilIdlePool* pool,
    GVariant* variant);

void
gutil_idle_pool_add_ptr_array_ref(
    GUtilIdlePool* pool,
    GPtrArray* array);

void
gutil_idle_pool_add_bytes_ref(
    GUtilIdlePool* pool,
    GBytes* bytes); /* Since 1.0.34 */

G_END_DECLS

#endif /* GUTIL_IDLEPOOL_H */

/*
 * Local Variables:
 * mode: C
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
