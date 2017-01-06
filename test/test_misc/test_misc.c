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
 *   3. Neither the name of the Jolla Ltd nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
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

#include "test_common.h"

#include "gutil_misc.h"
#include "gutil_idlepool.h"
#include "gutil_log.h"

static TestOpt test_opt;

/*==========================================================================*
 * Basic
 *==========================================================================*/

static
void
test_notify(
    GObject* object,
    GParamSpec* spec,
    gpointer data)
{
}

static
void
test_basic(
    void)
{
    gulong id[2];
    GObject* obj = g_object_new(GUTIL_IDLE_POOL_TYPE, NULL);

    /* These have no effect */
    gutil_disconnect_handlers(NULL, id, G_N_ELEMENTS(id));
    gutil_disconnect_handlers(obj, NULL, 0);

    id[0] = g_signal_connect(obj, "notify", G_CALLBACK(test_notify), NULL);
    id[1] = g_signal_connect(obj, "notify", G_CALLBACK(test_notify), NULL);
    GASSERT(id[0] && id[1]);

    /* gutil_disconnect_handlers zeros the ids */
    gutil_disconnect_handlers(obj, id, G_N_ELEMENTS(id));
    g_assert(!id[0]);
    g_assert(!id[1]);

    /* Second time has no effect */
    gutil_disconnect_handlers(obj, id, G_N_ELEMENTS(id));
    g_assert(!id[0]);
    g_assert(!id[1]);

    g_object_unref(obj);
}

/*==========================================================================*
 * Common
 *==========================================================================*/

#define TEST_PREFIX "/misc/"

int main(int argc, char* argv[])
{
    G_GNUC_BEGIN_IGNORE_DEPRECATIONS;
    g_type_init();
    G_GNUC_END_IGNORE_DEPRECATIONS;
    g_test_init(&argc, &argv, NULL);
    g_test_add_func(TEST_PREFIX "basic", test_basic);
    test_init(&test_opt, argc, argv);
    return g_test_run();
}

/*
 * Local Variables:
 * mode: C
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
