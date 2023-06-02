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

#ifndef GUTIL_OBJV_H
#define GUTIL_OBJV_H

#include "gutil_types.h"

#include <glib-object.h>

/*
 * Operations on NULL-terminated array of references to GObjects.
 *
 * Since 1.0.70
 */

G_BEGIN_DECLS

void
gutil_objv_free(
    GObject** objv);

GObject**
gutil_objv_copy(
    GObject* const* objv)
    G_GNUC_WARN_UNUSED_RESULT;

GObject**
gutil_objv_add(
    GObject** objv,
    GObject* obj)
    G_GNUC_WARN_UNUSED_RESULT;

GObject**
gutil_objv_insert(
    GObject** objv,
    GObject* obj,
    gsize pos) /* Since 1.0.71 */
    G_GNUC_WARN_UNUSED_RESULT;

GObject**
gutil_objv_append(
    GObject** objv,
    GObject* const* objs) /* Since 1.0.71 */
    G_GNUC_WARN_UNUSED_RESULT;

GObject**
gutil_objv_remove(
    GObject** objv,
    GObject* obj,
    gboolean all)
    G_GNUC_WARN_UNUSED_RESULT;

GObject**
gutil_objv_remove_at(
    GObject** objv,
    gsize pos)
    G_GNUC_WARN_UNUSED_RESULT;

GObject*
gutil_objv_at(
    GObject* const* objv,
    gsize pos);

gboolean
gutil_objv_equal(
    GObject* const* objv1,
    GObject* const* objv2);

GObject*
gutil_objv_first(
    GObject* const* objv);

GObject*
gutil_objv_last(
    GObject* const* objv);

gssize
gutil_objv_find(
    GObject* const* objv,
    GObject* obj);

gssize
gutil_objv_find_last(
    GObject* const* objv,
    GObject* obj);

gboolean
gutil_objv_contains(
    GObject* const* objv,
    GObject* obj);

G_END_DECLS

#endif /* GUTIL_OBJV_H */

/*
 * Local Variables:
 * mode: C
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
