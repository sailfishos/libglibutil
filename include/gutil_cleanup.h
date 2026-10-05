/*
 * SPDX-FileCopyrightText: 2026 Jolla Mobile Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Since 1.0.83
 */

#ifndef GUTIL_CLEANUP_H
#define GUTIL_CLEANUP_H

#include "gutil_types.h"

/*
 * GUtilCleanup is a collection of things to be destroyed.
 */

G_BEGIN_DECLS

GUtilCleanup*
gutil_cleanup_new(
    void);

gpointer
gutil_cleanup_add(
    GUtilCleanup* cleanup,
    GDestroyNotify destroy,
    gpointer pointer);

gpointer
gutil_cleanup_add_ptr(
    GUtilCleanup* cleanup,
    gpointer ptr);

char**
gutil_cleanup_add_strv(
    GUtilCleanup* cleanup,
    char** strv);

gpointer
gutil_cleanup_add_object(
    GUtilCleanup* cleanup,
    gpointer object);

gpointer
gutil_cleanup_add_object_ref(
    GUtilCleanup* cleanup,
    gpointer object);

GVariant*
gutil_cleanup_add_variant(
    GUtilCleanup* cleanup,
    GVariant* variant);

GVariant*
gutil_cleanup_add_variant_ref(
    GUtilCleanup* cleanup,
    GVariant* variant);

GBytes*
gutil_cleanup_add_bytes(
    GUtilCleanup* cleanup,
    GBytes* bytes);

GBytes*
gutil_cleanup_add_bytes_ref(
    GUtilCleanup* cleanup,
    GBytes* bytes);

gboolean
gutil_cleanup_drop(
    GUtilCleanup* cleanup,
    gpointer pointer);

void
gutil_cleanup_clear(
    GUtilCleanup* cleanup);

void
gutil_cleanup_free(
    GUtilCleanup* cleanup);

G_END_DECLS

#endif /* GUTIL_CLEANUP_H */

/*
 * Local Variables:
 * mode: C
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
