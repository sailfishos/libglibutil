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

#include <gutil_idlepool.h>
#include <gutil_macros.h>
#include <gutil_log.h>

#include <glib-object.h>

typedef struct gutil_idle_pool_item GUtilIdlePoolItem;

struct gutil_idle_pool_item {
    GUtilIdlePoolItem* next;
    gpointer pointer;
    GDestroyNotify destroy;
};

struct gutil_idle_pool {
    gint ref_count;
    guint idle_id;
    GUtilIdlePoolItem* first;
    GUtilIdlePoolItem* last;
    GUtilIdlePool** shared;
};

GUtilIdlePool*
gutil_idle_pool_new()
{
    GUtilIdlePool* self = g_slice_new0(GUtilIdlePool);
    g_atomic_int_set(&self->ref_count, 1);
    return self;
}

GUtilIdlePool*
gutil_idle_pool_get(
    GUtilIdlePool** shared)
{
    if (shared) {
        if (*shared) {
            /* The object already exists */
            return *shared;
        } else {
            GUtilIdlePool* pool = gutil_idle_pool_new();
            pool->shared = shared;
            /* New shared object will destroy itself if the caller
             * doesn't reference it. */
            gutil_idle_pool_add(pool, pool, (GDestroyNotify)
                gutil_idle_pool_unref);
            *shared = pool;
            return pool;
        }
    } else {
        return gutil_idle_pool_new();
    }
}

GUtilIdlePool*
gutil_idle_pool_ref(
    GUtilIdlePool* self)
{
    if (G_LIKELY(self)) {
        GASSERT(self->ref_count > 0);
        g_atomic_int_inc(&self->ref_count);
    }
    return self;
}

void
gutil_idle_pool_unref(
    GUtilIdlePool* self)
{
    if (G_LIKELY(self)) {
        GASSERT(self->ref_count > 0);
        if (g_atomic_int_dec_and_test(&self->ref_count)) {
            /* Clear pointer to the shared instance */
            if (self->shared) *(self->shared) = NULL;
            gutil_idle_pool_drain(self);
            gutil_slice_free(self);
        }
    }
}

void
gutil_idle_pool_destroy(
    GUtilIdlePool* self) /* Since 1.0.34 */
{
    gutil_idle_pool_drain(self);
    gutil_idle_pool_unref(self);
}

void
gutil_idle_pool_drain(
    GUtilIdlePool* self)
{
    if (G_LIKELY(self)) {
        GUtilIdlePoolItem* items = self->first;
        while (items) {
            GUtilIdlePoolItem* item = items;
            self->first = self->last = NULL;
            while (item) {
                item->destroy(item->pointer);
                item = item->next;
            }
            g_slice_free_chain(GUtilIdlePoolItem, items, next);
            items = self->first;
        }
        if (self->idle_id) {
            g_source_remove(self->idle_id);
            self->idle_id = 0;
        }
    }
}

static
gboolean
gutil_idle_pool_idle(
    gpointer user_data)
{
    GUtilIdlePool* self = user_data;
    self->idle_id = 0;
    gutil_idle_pool_ref(self);
    gutil_idle_pool_drain(self);
    gutil_idle_pool_unref(self);
    return G_SOURCE_REMOVE;
}

void
gutil_idle_pool_add(
    GUtilIdlePool* self,
    gpointer pointer,
    GDestroyNotify destroy)
{
    if (G_LIKELY(self) && G_LIKELY(destroy)) {
        GUtilIdlePoolItem* item = g_slice_new(GUtilIdlePoolItem);
        item->next = NULL;
        item->pointer = pointer;
        item->destroy = destroy;
        if (self->last) {
            self->last->next = item;
        } else {
            GASSERT(!self->first);
            self->first = item;
        }
        self->last = item;
        if (!self->idle_id) {
            self->idle_id = g_idle_add(gutil_idle_pool_idle, self);
        }
    }
}

void
gutil_idle_pool_add_strv(
    GUtilIdlePool* self,
    char** strv) /* Since 1.0.32 */
{
    if (G_LIKELY(strv)) {
        gutil_idle_pool_add(self, strv, (GDestroyNotify) g_strfreev);
    }
}

void
gutil_idle_pool_add_object(
    GUtilIdlePool* self,
    gpointer object)
{
    if (G_LIKELY(object)) {
        gutil_idle_pool_add(self, G_OBJECT(object), g_object_unref);
    }
}

void
gutil_idle_pool_add_variant(
    GUtilIdlePool* self,
    GVariant* variant)
{
    if (G_LIKELY(variant)) {
        gutil_idle_pool_add(self, variant, (GDestroyNotify)g_variant_unref);
    }
}

void
gutil_idle_pool_add_ptr_array(
    GUtilIdlePool* self,
    GPtrArray* array)
{
    if (G_LIKELY(array)) {
        gutil_idle_pool_add(self, array, (GDestroyNotify)g_ptr_array_unref);
    }
}

void
gutil_idle_pool_add_bytes(
    GUtilIdlePool* self,
    GBytes* bytes) /* Since 1.0.34 */
{
    if (G_LIKELY(bytes)) {
        gutil_idle_pool_add(self, bytes, (GDestroyNotify)g_bytes_unref);
    }
}

void
gutil_idle_pool_add_object_ref(
    GUtilIdlePool* self,
    gpointer object)
{
    if (G_LIKELY(self) && G_LIKELY(object)) {
        gutil_idle_pool_add_object(self, g_object_ref(object));
    }
}

void
gutil_idle_pool_add_variant_ref(
    GUtilIdlePool* self,
    GVariant* variant)
{
    if (G_LIKELY(self) && G_LIKELY(variant)) {
        gutil_idle_pool_add_variant(self, g_variant_ref(variant));
    }
}

void
gutil_idle_pool_add_ptr_array_ref(
    GUtilIdlePool* self,
    GPtrArray* array)
{
    if (G_LIKELY(self) && G_LIKELY(array)) {
        gutil_idle_pool_add_ptr_array(self, g_ptr_array_ref(array));
    }
}

void
gutil_idle_pool_add_bytes_ref(
    GUtilIdlePool* self,
    GBytes* bytes) /* Since 1.0.34 */
{
    if (G_LIKELY(self) && G_LIKELY(bytes)) {
        gutil_idle_pool_add_bytes(self, g_bytes_ref(bytes));
    }
}

/*
 * Local Variables:
 * mode: C
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
