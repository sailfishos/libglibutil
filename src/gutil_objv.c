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

#include "gutil_objv.h"
#include "gutil_misc.h"

GObject**
gutil_objv_new(
    GObject* obj1,
    ...) /* Since 1.0.72 */
{
    gsize len = 0;
    GObject** objv;
    GObject** ptr;

    if (obj1) {
        GObject* obj;
        va_list args;

        len++;
        va_start(args, obj1);
        while ((obj = va_arg(args, GObject*)) != NULL) {
            len++;
        }
        va_end(args);
    }

    ptr = objv = g_new(GObject*, len + 1);
    if (obj1) {
        GObject* obj;
        va_list args;

        g_object_ref(*ptr++ = obj1);
        va_start(args, obj1);
        while ((obj = va_arg(args, GObject*)) != NULL) {
            g_object_ref(*ptr++ = obj);
        }
        va_end(args);
    }
    *ptr = NULL;
    return objv;
}

void
gutil_objv_free(
    GObject** objv)
{
    if (objv) {
        GObject** ptr = objv;

        while (*ptr) g_object_unref(*ptr++);
        g_free(objv);
    }
}

GObject**
gutil_objv_copy(
    GObject* const* objv)
{
    if (objv) {
        GObject* const* ptr = objv;
        gsize n = 0;

        /* Count the objects and bump references at the same time */
        while (*ptr) {
            g_object_ref(*ptr++);
            n++;
        }
        return gutil_memdup(objv, sizeof(GObject*) * (n + 1));
    }
    return NULL;

}

GObject**
gutil_objv_add(
    GObject** objv,
    GObject* obj)
{
    if (obj) {
        gsize len = gutil_ptrv_length(objv);

        objv = g_renew(GObject*, objv, len + 2);
        g_object_ref(objv[len++] = obj);
        objv[len] = NULL;
    }
    return objv;
}

GObject**
gutil_objv_insert(
    GObject** objv,
    GObject* obj,
    gsize pos) /* Since 1.0.71 */
{
    if (obj) {
        gsize len = gutil_ptrv_length(objv);

        objv = g_renew(GObject*, objv, len + 2);
        if (pos >= len) {
            /* Insert as the last element */
            g_object_ref(objv[len++] = obj);
            objv[len] = NULL;
        } else {
            /* Insert somewhere in the middle (or at the very beginning) */
            memmove(objv + (pos + 1), objv + pos, sizeof(GObject*) *
                (len - pos + 1)); /* Move NULL too */
            g_object_ref(objv[pos] = obj);
        }
    }
    return objv;
}

GObject**
gutil_objv_append(
    GObject** objv,
    GObject* const* objs) /* Since 1.0.71 */
{
    const gsize len2 = gutil_ptrv_length(objs);

    if (len2 > 0) {
        const gsize len1 = gutil_ptrv_length(objv);
        GObject* const* src;
        GObject** dest;

        objv = g_renew(GObject*, objv, len1 + len2 + 1);
        dest = objv + len1;
        src = objs;
        while (*src) {
            *dest++ = g_object_ref(*src++);
        }
        *dest = NULL;
    }
    return objv;
}

static
gssize
gutil_objv_find_last_impl(
    GObject* const* objv,
    GObject* obj,
    gsize i /* exclisive */)
{
    while (i > 0) {
        if (objv[--i] == obj) {
            return i;
        }
    }
    return -1;
}

static
GObject**
gutil_objv_remove_impl(
    GObject** objv,
    gsize pos,
    gsize len)
{
    g_object_unref(objv[pos]);
    memmove(objv + pos, objv + (pos + 1), sizeof(GObject*) * (len - pos));
    return g_realloc(objv, sizeof(GObject*) * len);
}

GObject**
gutil_objv_remove(
    GObject** objv,
    GObject* obj,
    gboolean all)
{
    if (objv && obj) {
        const gssize pos = gutil_objv_find(objv, obj);

        if (pos >= 0) {
            gsize len = gutil_ptrv_length(objv);

            objv = gutil_objv_remove_impl(objv, pos, len);
            if (all) {
                gssize i, l;

                len--;
                l = len - pos;
                while ((i = gutil_objv_find_last_impl(objv + pos,
                    obj, l)) >= 0) {
                    objv = gutil_objv_remove_impl(objv, pos + i, len--);
                    l = i;
                }
            }
        }
    }
    return objv;
}

GObject**
gutil_objv_remove_at(
    GObject** objv,
    gsize pos)
{
    if (objv) {
        const gsize len = gutil_ptrv_length(objv);

        if (pos < len) {
            objv = gutil_objv_remove_impl(objv, pos, len);
        }
    }
    return objv;
}

GObject*
gutil_objv_at(
    GObject* const* objv,
    gsize pos)
{
    if (objv) {
        guint i = 0;

        while (objv[i] && i < pos) i++;
        if (i == pos) {
            /* We also end up here if i == len but that's OK */
            return objv[pos];
        }
    }
    return NULL;

}

gboolean
gutil_objv_equal(
    GObject* const* v1,
    GObject* const* v2)
{
    if (v1 == v2) {
        return TRUE;
    } else if (!v1) {
        return !v2[0];
    } else if (!v2) {
        return !v1[0];
    } else {
        gsize len = 0;

        while (v1[len] && v1[len] == v2[len]) len++;
        return !v1[len] && !v2[len];
    }
}

GObject*
gutil_objv_first(
    GObject* const* objv)
{
    return objv ? objv[0] : NULL;
}

GObject*
gutil_objv_last(
    GObject* const* objv)
{
    if (objv && objv[0]) {
        GObject* const* ptr = objv;

        while (ptr[1]) ptr++;
        return *ptr;
    }
    return NULL;
}

gssize
gutil_objv_find(
    GObject* const* objv,
    GObject* obj)
{
    if (objv && obj) {
        GObject* const* ptr;

        for (ptr = objv; *ptr; ptr++) {
            if (*ptr == obj) {
                return ptr - objv;
            }
        }
    }
    return -1;
}

gssize
gutil_objv_find_last(
    GObject* const* objv,
    GObject* obj)
{
    return (objv && obj) ?
        gutil_objv_find_last_impl(objv, obj, gutil_ptrv_length(objv)) :
        -1;
}

gboolean
gutil_objv_contains(
    GObject* const* objv,
    GObject* obj)
{
    if (objv && obj) {
        GObject* const* ptr;

        for (ptr = objv; *ptr; ptr++) {
            if (*ptr == obj) {
                return TRUE;
            }
        }
    }
    return FALSE;
}

/*
 * Local Variables:
 * mode: C
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
