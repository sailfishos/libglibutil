/*
 * SPDX-FileCopyrightText: 2026 Jolla Mobile Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Since 1.0.83
 */

#include <gutil_cleanup.h>
#include <gutil_misc.h>

#include <glib-object.h>

#if __GNUC__ >= 4
#pragma GCC visibility push(default)
#endif

typedef struct gutil_cleanup_item {
    GDestroyNotify destroy;
    gpointer pointer;
} GUtilCleanupItem;

/*
 * This is basically a GArray providing better type safety at compile time.
 */
struct gutil_cleanup {
    GUtilCleanupItem* items;
    guint count;
};

G_STATIC_ASSERT(sizeof(GUtilCleanup) == sizeof(GArray));
#define ELEMENT_SIZE (sizeof(GUtilCleanupItem))

static
void
gutil_cleanup_destroy_item(
    gpointer data)
{
    GUtilCleanupItem* item = data;

    item->destroy(item->pointer);
}

static
void
gutil_cleanup_strv_free(
    gpointer strv)
{
    g_strfreev(strv);
}

static
void
gutil_cleanup_variant_unref(
    gpointer variant)
{
    g_variant_unref(variant);
}

static
void
gutil_cleanup_bytes_unref(
    gpointer bytes)
{
    g_bytes_unref(bytes);
}

GUtilCleanup*
gutil_cleanup_new()
{
    GArray* array = g_array_sized_new(FALSE, FALSE, ELEMENT_SIZE, 0);

    g_array_set_clear_func(array, gutil_cleanup_destroy_item);
    return (GUtilCleanup*)array;
}

gpointer
gutil_cleanup_add(
    GUtilCleanup* self,
    GDestroyNotify destroy,
    gpointer pointer)
{
    if (G_LIKELY(self) && G_LIKELY(destroy) && G_LIKELY(pointer)) {
        GUtilCleanupItem item;

        item.destroy = destroy;
        item.pointer = pointer;
        g_array_append_vals((GArray*)self, &item, 1);
        return pointer;
    }
    return NULL;
}

gpointer
gutil_cleanup_add_ptr(
    GUtilCleanup* self,
    gpointer ptr)
{
    return gutil_cleanup_add(self, g_free, ptr);
}

char**
gutil_cleanup_add_strv(
    GUtilCleanup* self,
    char** strv)
{
    return gutil_cleanup_add(self, gutil_cleanup_strv_free, strv);
}

gpointer
gutil_cleanup_add_object(
    GUtilCleanup* self,
    gpointer object)
{
    return G_LIKELY(object) ?
        gutil_cleanup_add(self, g_object_unref, G_OBJECT(object)) :
        NULL;
}

gpointer
gutil_cleanup_add_object_ref(
    GUtilCleanup* self,
    gpointer object)
{
    return gutil_cleanup_add_object(self, gutil_object_ref(object));
}

GVariant*
gutil_cleanup_add_variant(
    GUtilCleanup* self,
    GVariant* variant)
{
    return G_LIKELY(variant) ?
        gutil_cleanup_add(self, gutil_cleanup_variant_unref, variant) :
        NULL;
}

GVariant*
gutil_cleanup_add_variant_ref(
    GUtilCleanup* self,
    GVariant* variant)
{
    return G_LIKELY(variant) ?
        gutil_cleanup_add_variant(self, g_variant_ref_sink(variant)) :
        NULL;
}

GBytes*
gutil_cleanup_add_bytes(
    GUtilCleanup* self,
    GBytes* bytes)
{
    return G_LIKELY(bytes) ?
        gutil_cleanup_add(self, gutil_cleanup_bytes_unref, bytes) :
        NULL;
}

GBytes*
gutil_cleanup_add_bytes_ref(
    GUtilCleanup* self,
    GBytes* bytes)
{
    return G_LIKELY(bytes) ?
        gutil_cleanup_add_bytes(self, g_bytes_ref(bytes)) :
        NULL;
}

gboolean
gutil_cleanup_drop(
    GUtilCleanup* self,
    gpointer pointer)
{
    if (G_LIKELY(self) && G_LIKELY(pointer)) {
        guint i;

        for (i = 0; i < self->count; i++) {
            if (self->items[i].pointer == pointer) {
                g_array_remove_index((GArray*)self, i);
                return TRUE;
            }
        }
    }
    return FALSE;
}

void
gutil_cleanup_clear(
    GUtilCleanup* self)
{
    if (G_LIKELY(self)) {
        g_array_set_size((GArray*)self, 0);
    }
}

void
gutil_cleanup_free(
    GUtilCleanup* self)
{
    if (G_LIKELY(self)) {
        g_array_free((GArray*)self, TRUE);
    }
}

/*
 * Local Variables:
 * mode: C
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
