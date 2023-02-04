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

#ifndef GUTIL_WEAKREF_H
#define GUTIL_WEAKREF_H

#include "gutil_types.h"

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

G_BEGIN_DECLS

GUtilWeakRef*
gutil_weakref_new(
    gpointer obj);

GUtilWeakRef*
gutil_weakref_ref(
    GUtilWeakRef* ref);

void
gutil_weakref_unref(
    GUtilWeakRef* ref);

gpointer
gutil_weakref_get(
    GUtilWeakRef* ref);

void
gutil_weakref_set(
    GUtilWeakRef* ref,
    gpointer obj);

G_END_DECLS

#endif /* GUTIL_WEAKREF_H */

/*
 * Local Variables:
 * mode: C
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
