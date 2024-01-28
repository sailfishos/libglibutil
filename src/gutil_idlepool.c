/*
 * Copyright (C) 2016-2024 Slava Monich <slava@monich.com>
 * Copyright (C) 2016-2020 Jolla Ltd.
 *
 * You may use this file under the terms of the BSD license as follows:
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

#if __GNUC__ >= 4
#pragma GCC visibility push(default)
#endif

typedef struct gutil_idle_pool_item GUtilIdlePoolItem;

struct gutil_idle_pool_item {
    GUtilIdlePoolItem* next;
    gpointer pointer;
    GDestroyNotify destroy;
};

struct gutil_idle_pool {
    gint ref_count;
    guint idle_id;
    GThread* thread;
    GUtilIdlePoolItem* first;
    GUtilIdlePoolItem* last;
    GUtilIdlePool** shared;
};

GUtilIdlePool*
gutil_idle_pool_new()
{
    GUtilIdlePool* self = g_slice_new0(GUtilIdlePool);

    /* No need to take a ref as we never dereference this pointer */
    self->thread = g_thread_self();
    g_atomic_int_set(&self->ref_count, 1);
    return self;
}

static
void
gutil_idle_pool_unref_1(
    gpointer pool)
{
    gutil_idle_pool_unref(pool);
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
            gutil_idle_pool_add(pool, pool, gutil_idle_pool_unref_1);
            *shared = pool;
            return pool;
        }
    } else {
        return gutil_idle_pool_new();
    }
}

/*
 * Thread specific GUtilIdlePool gets created on demand and remains alive
 * until the thread exits or gutil_idle_pool_release_default is called. The
 * thread should have a GMainContext or else strange things may happen (see
 * the comments in gutil_idle_pool_idle).
 */
GUtilIdlePool*
gutil_idle_pool_get_default() /* Since 1.0.76 */
{
    static GPrivate thread_pool = G_PRIVATE_INIT(gutil_idle_pool_unref_1);
    GUtilIdlePool* pool = g_private_get(&thread_pool);

    if (!pool) {
        pool = gutil_idle_pool_new();
        g_private_set(&thread_pool, pool);
    }
    return pool;
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
    if (G_LIKELY(self)) {
        gutil_idle_pool_drain(self);
        gutil_idle_pool_unref(self);
    }
}

void
gutil_idle_pool_drain(
    GUtilIdlePool* self)
{
    /* If no pool is specified, use the default one for this thread */
    if (!self) {
        self = gutil_idle_pool_get_default();
    }
    while (self->first) {
        GUtilIdlePoolItem* items = self->first;
        GUtilIdlePoolItem* item = items;

        self->first = self->last = NULL;
        while (item) {
            item->destroy(item->pointer);
            item = item->next;
        }
        g_slice_free_chain(GUtilIdlePoolItem, items, next);
    }
    if (self->idle_id) {
        const guint id = self->idle_id;

        self->idle_id = 0;
        g_source_remove(id);
    }
}

static
gboolean
gutil_idle_pool_idle(
    gpointer user_data)
{
    GUtilIdlePool* self = user_data;

    self->idle_id = 0;

    /*
     * gutil_idle_pool_idle may be invoked on a wrong (main) thread if
     * something is added to a thread specific pool (e.g. the one created
     * by gutil_idle_pool_get_default) before the thread specific GMainContext
     * has been pushed to the context stack. In this case we must not touch
     * the pool because the access to the pool items is not synchronized and
     * we generally want to destroy items on the same thread which has created
     * them. Hence the check for the current thread.
     *
     * In the worst case (if the thread specific GMainContext never gets
     * created) this callback will keep getting invoked but the items won't
     * be actually deallocated until the thread exits. And the pool (together
     * with the items it contains) may actually end up being destroyed on the
     * main thread.
     *
     * The bottom line is that it's better not to use GUtilIdlePool without
     * GMainContext.
     */
    if (self->thread == g_thread_self()) {
        gutil_idle_pool_drain(self);
    } else {
        GDEBUG("gutil_idle_pool_idle is invoked on a wrong thread");
    }
    return G_SOURCE_REMOVE;
}

gpointer /* Return value since 1.0.76 */
gutil_idle_pool_add(
    GUtilIdlePool* self,
    gpointer pointer,
    GDestroyNotify destroy)
{
    if (G_LIKELY(destroy)) {
        GUtilIdlePoolItem* item = g_slice_new(GUtilIdlePoolItem);

        item->next = NULL;
        item->pointer = pointer;
        item->destroy = destroy;

        /* If no pool is specified, use the default one for this thread */
        if (!self) {
            self = gutil_idle_pool_get_default();
        }
        if (self->last) {
            self->last->next = item;
        } else {
            GASSERT(!self->first);
            self->first = item;
        }
        self->last = item;
        if (!self->idle_id) {
            GSource* src = g_idle_source_new();
            GMainContext* context = g_main_context_get_thread_default();

            g_source_set_priority(src, G_PRIORITY_DEFAULT_IDLE);
            g_source_set_callback(src, gutil_idle_pool_idle,
                gutil_idle_pool_ref(self), gutil_idle_pool_unref_1);
            self->idle_id = g_source_attach(src, context ? context :
                g_main_context_default());
            g_source_unref(src);
        }
    }
    return pointer;
}

static
void
gutil_idle_pool_strv_free(
    gpointer strv)
{
    g_strfreev(strv);
}

char** /* Return value since 1.0.76 */
gutil_idle_pool_add_strv(
    GUtilIdlePool* self,
    char** strv) /* Since 1.0.32 */
{
    if (G_LIKELY(strv)) {
        gutil_idle_pool_add(self, strv, gutil_idle_pool_strv_free);
    }
    return strv;
}

gpointer /* Return value since 1.0.76 */
gutil_idle_pool_add_object(
    GUtilIdlePool* self,
    gpointer object)
{
    if (G_LIKELY(object)) {
        gutil_idle_pool_add(self, G_OBJECT(object), g_object_unref);
    }
    return object;
}

static
void
gutil_idle_pool_variant_unref(
    gpointer var)
{
    g_variant_unref(var);
}

GVariant* /* Returns value since 1.0.76 */
gutil_idle_pool_add_variant(
    GUtilIdlePool* self,
    GVariant* variant)
{
    if (G_LIKELY(variant)) {
        gutil_idle_pool_add(self, variant, gutil_idle_pool_variant_unref);
    }
    return variant;
}

static
void
gutil_idle_pool_ptr_array_unref(
    gpointer array)
{
    g_ptr_array_unref(array);
}

GPtrArray* /* Return value since 1.0.76 */
gutil_idle_pool_add_ptr_array(
    GUtilIdlePool* self,
    GPtrArray* array)
{
    if (G_LIKELY(array)) {
        gutil_idle_pool_add(self, array, gutil_idle_pool_ptr_array_unref);
    }
    return array;
}

static
void
gutil_idle_pool_bytes_unref(
    gpointer bytes)
{
    g_bytes_unref(bytes);
}

GBytes* /* Return value since 1.0.76 */
gutil_idle_pool_add_bytes(
    GUtilIdlePool* self,
    GBytes* bytes) /* Since 1.0.34 */
{
    return G_LIKELY(bytes) ?
        gutil_idle_pool_add(self, bytes, gutil_idle_pool_bytes_unref) :
        NULL;
}

gpointer /* Return value since 1.0.76 */
gutil_idle_pool_add_object_ref(
    GUtilIdlePool* self,
    gpointer object)
{
    return G_LIKELY(object) ?
        gutil_idle_pool_add_object(self, g_object_ref(object)) :
        NULL;
}

GVariant* /* Returns value since 1.0.76 */
gutil_idle_pool_add_variant_ref(
    GUtilIdlePool* self,
    GVariant* variant)
{
    return G_LIKELY(variant) ?
        gutil_idle_pool_add_variant(self, g_variant_ref(variant)) :
        NULL;
}

GPtrArray* /* Return value since 1.0.76 */
gutil_idle_pool_add_ptr_array_ref(
    GUtilIdlePool* self,
    GPtrArray* array)
{
    return G_LIKELY(array) ?
        gutil_idle_pool_add_ptr_array(self, g_ptr_array_ref(array)) :
        NULL;
}

GBytes* /* Return value since 1.0.76 */
gutil_idle_pool_add_bytes_ref(
    GUtilIdlePool* self,
    GBytes* bytes) /* Since 1.0.34 */
{
    return G_LIKELY(bytes) ?
        gutil_idle_pool_add_bytes(self, g_bytes_ref(bytes)) :
        NULL;
}

/*
 * Local Variables:
 * mode: C
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
