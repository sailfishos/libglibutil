/*
 * Copyright (C) 2016-2017 Jolla Ltd.
 * Contact: Slava Monich <slava.monich@jolla.com>
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
 *   3. Neither the name of Jolla Ltd nor the names of its contributors may
 *      be used to endorse or promote products derived from this software
 *      without specific prior written permission.
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

#ifndef GUTIL_RING_H
#define GUTIL_RING_H

#include "gutil_types.h"

G_BEGIN_DECLS

/*
 * GLib-style "first in, first out" ring buffer.
 *
 * Note that `get' functions don't invoke the free function on the
 * elements they remove from the ring buffer. The ring buffer only
 * frees the elements it removes internally in gutil_ring_clear,
 * gutil_ring_drop functions and the last gutil_ring_unref.
 */

#define GUTIL_RING_UNLIMITED_SIZE (-1)

GUtilRing*
gutil_ring_new(void);

GUtilRing*
gutil_ring_sized_new(
    gint reserved_size,
    gint max_size);

GUtilRing*
gutil_ring_new_full(
    gint reserved_size,
    gint max_size,
    GDestroyNotify free_func);

GUtilRing*
gutil_ring_ref(
    GUtilRing* ring);

void
gutil_ring_unref(
    GUtilRing* ring);

void
gutil_ring_set_free_func(
    GUtilRing* ring,
    GDestroyNotify free_func);

gint
gutil_ring_max_size(
    GUtilRing* ring);

void
gutil_ring_set_max_size(
    GUtilRing* ring,
    gint max_size);

gint
gutil_ring_size(
    GUtilRing* ring);

void
gutil_ring_clear(
    GUtilRing* ring);

void
gutil_ring_compact(
    GUtilRing* ring);

gboolean
gutil_ring_reserve(
    GUtilRing* ring,
    gint reserved_size);

gboolean
gutil_ring_can_put(
    GUtilRing* ring,
    gint num_elements);

gboolean
gutil_ring_put(
    GUtilRing* ring,
    gpointer data);

gboolean
gutil_ring_put_front(
    GUtilRing* ring,
    gpointer data);

gpointer
gutil_ring_get(
    GUtilRing* ring);

gpointer
gutil_ring_get_last(
    GUtilRing* ring);

gint
gutil_ring_drop(
    GUtilRing* ring,
    gint count);

gint
gutil_ring_drop_last(
    GUtilRing* ring,
    gint count);

gpointer
gutil_ring_data_at(
    GUtilRing* ring,
    gint pos);

gpointer*
gutil_ring_flatten(
    GUtilRing* ring,
    gint* size);

G_END_DECLS

#endif /* GUTIL_RING_H */

/*
 * Local Variables:
 * mode: C
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
