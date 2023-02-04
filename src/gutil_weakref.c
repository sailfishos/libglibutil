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

#include "gutil_weakref.h"
#include "gutil_log.h"
#include "gutil_macros.h"

#include <glib-object.h>

/*
 * Ref-countable weak reference can be used to avoid calling g_weak_ref_set()
 * too often because it grabs global weak_locations_lock for exclusive access.
 * Note that g_weak_ref_set() is also invoked internally by g_weak_ref_init()
 * and g_weak_ref_clear().
 *
 * g_weak_ref_get() on the other hand only acquires weak_locations_lock
 * for read-only access which is less of a bottleneck in a multi-threaded
 * environment. And it's generally significantly simpler and faster than
 * g_weak_ref_set().
 *
 * Since 1.0.68
 */

struct gutil_weakref {
    gint ref_count;
    GWeakRef weak_ref;
};

GUtilWeakRef*
gutil_weakref_new(
    gpointer obj)
{
    GUtilWeakRef* self = g_slice_new(GUtilWeakRef);

    g_atomic_int_set(&self->ref_count, 1);
    g_weak_ref_init(&self->weak_ref, obj);
    return self;
}

GUtilWeakRef*
gutil_weakref_ref(
    GUtilWeakRef* self)
{
    if (G_LIKELY(self)) {
        GASSERT(self->ref_count > 0);
        g_atomic_int_inc(&self->ref_count);
    }
    return self;
}

void
gutil_weakref_unref(
    GUtilWeakRef* self)
{
    if (G_LIKELY(self)) {
        GASSERT(self->ref_count > 0);
        if (g_atomic_int_dec_and_test(&self->ref_count)) {
            g_weak_ref_clear(&self->weak_ref);
            gutil_slice_free(self);
        }
    }
}

gpointer
gutil_weakref_get(
    GUtilWeakRef* self)
{
    return G_LIKELY(self) ? g_weak_ref_get(&self->weak_ref) : NULL;
}

void
gutil_weakref_set(
    GUtilWeakRef* self,
    gpointer obj)
{
    if (G_LIKELY(self)) {
        g_weak_ref_set(&self->weak_ref, obj);
    }
}

/*
 * Local Variables:
 * mode: C
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
