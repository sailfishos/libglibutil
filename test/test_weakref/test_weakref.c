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

#include "test_common.h"

#include "gutil_weakref.h"

#include <glib-object.h>

static TestOpt test_opt;

/*==========================================================================*
 * null
 *==========================================================================*/

static
void
test_null()
{
    gutil_weakref_unref(NULL);
    gutil_weakref_set(NULL, NULL);
    g_assert(!gutil_weakref_ref(NULL));
    g_assert(!gutil_weakref_get(NULL));
}

/*==========================================================================*
 * basic
 *==========================================================================*/

static
void
test_basic()
{
    GObject* obj = g_object_new(TEST_OBJECT_TYPE, NULL);
    GUtilWeakRef* ref = gutil_weakref_new(obj);

    g_assert(gutil_weakref_ref(ref) == ref);
    gutil_weakref_unref(ref);
    g_assert(gutil_weakref_get(ref) == obj);
    g_object_unref(obj);
    gutil_weakref_unref(ref);

    ref = gutil_weakref_new(NULL);
    g_assert(!gutil_weakref_get(ref));
    gutil_weakref_set(ref, obj);
    g_assert(gutil_weakref_get(ref) == obj);
    g_object_unref(obj);
    g_object_unref(obj); /* This actually deallocates the object */
    g_assert(!gutil_weakref_get(ref));
    gutil_weakref_unref(ref);
}

/*==========================================================================*
 * Common
 *==========================================================================*/

#define TEST_(name) "/weakref/" name

int main(int argc, char* argv[])
{
    G_GNUC_BEGIN_IGNORE_DEPRECATIONS;
    g_type_init();
    G_GNUC_END_IGNORE_DEPRECATIONS;
    g_test_init(&argc, &argv, NULL);
    test_init(&test_opt, argc, argv);
    g_test_add_func(TEST_("null"), test_null);
    g_test_add_func(TEST_("basic"), test_basic);
    return g_test_run();
}

/*
 * Local Variables:
 * mode: C
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
