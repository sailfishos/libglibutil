/*
 * Copyright (C) 2016 Jolla Ltd.
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

#include "gutil_ring.h"
#include "gutil_log.h"

#define RET_OK       (0)
#define RET_ERR      (1)

typedef struct test_desc {
    const char* name;
    int (*run)(void);
} TestDesc;

/*==========================================================================*
 * Basic
 *==========================================================================*/

int
test_basic()
{
    int i, n = 5, ret = RET_OK;
    GUtilRing* r = gutil_ring_new();

    /* Test NULL tolerance */
    gutil_ring_ref(NULL);
    gutil_ring_unref(NULL);
    gutil_ring_set_free_func(NULL, NULL);
    gutil_ring_set_free_func(r, NULL);
    gutil_ring_set_free_func(NULL, g_free);
    gutil_ring_clear(NULL);
    gutil_ring_compact(NULL);
    gutil_ring_reserve(NULL, 0);
    if (gutil_ring_size(NULL) != 0 ||
        gutil_ring_can_put(NULL, 1) ||
        gutil_ring_put(NULL, NULL) ||
        gutil_ring_put_front(NULL, NULL) ||
        gutil_ring_get(NULL) ||
        gutil_ring_data_at(NULL, 0) ||
        gutil_ring_get_last(NULL) ||
        gutil_ring_drop(NULL, 1) ||
        gutil_ring_drop_last(NULL, 1) ||
        gutil_ring_flatten(NULL, NULL)) {
        GDEBUG("NULL test failed");
        ret = RET_ERR;
    }

    gutil_ring_ref(r);
    gutil_ring_unref(r);

    for (i=0; i<n && ret == RET_OK; i++) {
        if (!gutil_ring_can_put(r, 1) ||
            !gutil_ring_put(r, GINT_TO_POINTER(i))) {
            GDEBUG("Failed to put data");
            ret = RET_ERR;
        }
    }

    if (gutil_ring_data_at(r, -1) || gutil_ring_data_at(r, n)) {
        GDEBUG("Unexpected get with invalid index");
        ret = RET_ERR;
    }

    for (i=0; i<n && ret == RET_OK; i++) {
        if (gutil_ring_data_at(r, i) != GINT_TO_POINTER(i)) {
            GDEBUG("Data peek mismatch");
            ret = RET_ERR;
        }
    }

    for (i=0; i<n && ret == RET_OK; i++) {
        gutil_ring_flatten(r, NULL);
        gutil_ring_compact(r);
        if (gutil_ring_get(r) != GINT_TO_POINTER(i)) {
            GDEBUG("Data get mismatch");
            ret = RET_ERR;
        }
    }

    if (gutil_ring_get(r)) {
        GDEBUG("Data get mismatch 2");
        ret = RET_ERR;
    }

    gutil_ring_compact(r);
    gutil_ring_compact(r);
    gutil_ring_unref(r);
    return ret;
}

/*==========================================================================*
 * PutFront
 *==========================================================================*/

int
test_put_front()
{
    int i, n = 5, ret = RET_OK;
    GUtilRing* r = gutil_ring_new();

    for (i=0; i<n && ret == RET_OK; i++) {
        if (!gutil_ring_can_put(r, 1) ||
            !gutil_ring_put_front(r, GINT_TO_POINTER(n-i-1))) {
            GDEBUG("Failed to put data");
            ret = RET_ERR;
        }
    }

    for (i=0; i<n && ret == RET_OK; i++) {
        gutil_ring_compact(r);
        if (gutil_ring_get(r) != GINT_TO_POINTER(i)) {
            GDEBUG("Data get mismatch");
            ret = RET_ERR;
        }
    }

    /* The same but take it out with get_last */
    for (i=0; i<n && ret == RET_OK; i++) {
        if (!gutil_ring_can_put(r, 1) ||
            !gutil_ring_put_front(r, GINT_TO_POINTER(n-i-1))) {
            GDEBUG("Failed to put data 2");
            ret = RET_ERR;
        }
    }

    for (i=0; i<n && ret == RET_OK; i++) {
        gutil_ring_compact(r);
        if (gutil_ring_get_last(r) != GINT_TO_POINTER(n-i-1)) {
            GDEBUG("Data get mismatch 2");
            ret = RET_ERR;
        }
    }

    if (gutil_ring_get_last(r)) {
        GDEBUG("Data get mismatch 3");
        ret = RET_ERR;
    }

    gutil_ring_unref(r);
    return ret;
}

/*==========================================================================*
 * Drop
 *==========================================================================*/

int
test_drop()
{
    int i, n = 5, get = 3, drop = 3, ret = RET_OK;
    GUtilRing* r = gutil_ring_sized_new(0,n);

    /* [01234] */
    for (i=0; i<n; i++) {
        gutil_ring_put(r, GINT_TO_POINTER(i));
    }

    /* ...[34] */
    for (i=0; i<get && ret == RET_OK; i++) {
        if (gutil_ring_get(r) != GINT_TO_POINTER(i)) {
            GDEBUG("Data get mismatch");
            ret = RET_ERR;
        }
    }

    /* 567][34 */
    for (i=0; i<get; i++) {
        gutil_ring_put(r, GINT_TO_POINTER(n+i));
    }

    /* ..[67].. */
    if (ret == RET_OK) {
        if (gutil_ring_drop(r, drop) != drop) {
            GDEBUG("Drop mismatch");
            ret = RET_ERR;
        }
    }

    if (ret == RET_OK) {
        for (i=0; i<(n-drop) && ret == RET_OK; i++) {
            if (gutil_ring_get(r) != GINT_TO_POINTER(get+drop+i)) {
                GDEBUG("Data get mismatch 2");
                ret = RET_ERR;
            }
        }
    }

    /* Drop more than the size of the buffer (i.e. clear it) */
    for (i=0; i<n; i++) {
        gutil_ring_put(r, GINT_TO_POINTER(i));
    }

    if (gutil_ring_drop(r, 0) != 0 ||
        gutil_ring_drop(r, n+1) != n ||
        gutil_ring_drop(r, 1) != 0 ||
        gutil_ring_size(r) != 0) {
        GDEBUG("Drop mismatch 2");
        ret = RET_ERR;
    }

    gutil_ring_unref(r);
    return ret;
}

/*==========================================================================*
 * DropLast
 *==========================================================================*/

int
test_drop_last()
{
    int i, n = 5, get = 2, drop = 3, ret = RET_OK;
    GUtilRing* r = gutil_ring_sized_new(0,n);

    /* [01234] */
    for (i=0; i<n; i++) {
        gutil_ring_put(r, GINT_TO_POINTER(i));
    }

    /* ..[234] */
    for (i=0; i<get && ret == RET_OK; i++) {
        if (gutil_ring_get(r) != GINT_TO_POINTER(i)) {
            GDEBUG("Data get mismatch");
            ret = RET_ERR;
        }
    }

    /* 56][234 */
    for (i=0; i<get; i++) {
        gutil_ring_put(r, GINT_TO_POINTER(n+i));
    }

    /* ..[23]. */
    if (ret == RET_OK) {
        if (gutil_ring_drop_last(r, drop) != drop) {
            GDEBUG("Drop mismatch");
            ret = RET_ERR;
        }
    }

    if (ret == RET_OK) {
        gutil_ring_flatten(r, NULL);
        for (i=0; i<(n-drop) && ret == RET_OK; i++) {
            if (gutil_ring_get(r) != GINT_TO_POINTER(get+i)) {
                GDEBUG("Data get mismatch 2");
                ret = RET_ERR;
            }
        }
    }

    /* Drop more than the size of the buffer (i.e. clear it) */
    for (i=0; i<n; i++) {
        gutil_ring_put(r, GINT_TO_POINTER(i));
    }

    if (gutil_ring_drop_last(r, 0) != 0 ||
        gutil_ring_drop_last(r, n+1) != n ||
        gutil_ring_drop_last(r, 1) != 0 ||
        gutil_ring_size(r) != 0) {
        GDEBUG("Drop mismatch 2");
        ret = RET_ERR;
    }

    gutil_ring_unref(r);
    return ret;
}

/*==========================================================================*
 * Limit
 *==========================================================================*/

int
test_limit()
{
    int i, limit = 5, extra = 2, ret = RET_OK;
    GUtilRing* r = gutil_ring_sized_new(2, limit);

    if (!gutil_ring_reserve(r, limit) ||
        gutil_ring_reserve(r, limit+1)) {
        GDEBUG("Unexpected reserve bahavior");
        ret = RET_ERR;
    }

    for (i=0; i<limit && ret == RET_OK; i++) {
        if (!gutil_ring_can_put(r, 1) ||
            !gutil_ring_put(r, GINT_TO_POINTER(i))) {
            GDEBUG("Failed to put data");
            ret = RET_ERR;
        }
    }

    if (gutil_ring_can_put(r, 1)) {
        ret = RET_ERR;
    } else {
        if (gutil_ring_get_last(r) != GINT_TO_POINTER(limit-1) ||
            gutil_ring_get_last(r) != GINT_TO_POINTER(limit-2) ||
            !gutil_ring_put(r, GINT_TO_POINTER(limit-2)) ||
            !gutil_ring_put(r, GINT_TO_POINTER(limit-1))) {
            GDEBUG("Data get last mismatch");
        }

        for (i=0; i<extra && ret == RET_OK; i++) {
            if (gutil_ring_get(r) != GINT_TO_POINTER(i)) {
                GDEBUG("Data get mismatch");
                ret = RET_ERR;
            }
            if (!gutil_ring_can_put(r, 1) ||
                !gutil_ring_put(r, GINT_TO_POINTER(i+limit))) {
                GDEBUG("Failed to put additional data");
                ret = RET_ERR;
            }
        }

        if (gutil_ring_size(r) != limit) {
            GDEBUG("Size check failed");
            ret = RET_ERR;
        } else {
            gint size;
            gpointer* data = gutil_ring_flatten(r, &size);
            if (size == limit) {
                for (i=0; i<size && ret == RET_OK; i++) {
                    if (data[i] != GINT_TO_POINTER(i+extra)) {
                        GDEBUG("Flattened data mismatch");
                        ret = RET_ERR;
                    }
                }
                if (gutil_ring_get_last(r) != GINT_TO_POINTER(size+extra-1)) {
                    GDEBUG("Get last data mismatch");
                    ret = RET_ERR;
                } else {
                    gutil_ring_compact(r);
                    data = gutil_ring_flatten(r, &size);
                    if (size == limit-1) {
                        for (i=0; i<size && ret == RET_OK; i++) {
                            if (data[i] != GINT_TO_POINTER(i+extra)) {
                                GDEBUG("Flattened data mismatch");
                                ret = RET_ERR;
                            }
                        }
                        gutil_ring_clear(r);
                    }
                }
            } else {
                GDEBUG("Flattened data size check failed");
                ret = RET_ERR;
            }
        }
    }

    if (ret == RET_OK) {
        for (i=0; i<limit; i++) {
            gutil_ring_put(r, GINT_TO_POINTER(i));
        }
        for (i=0; i<limit && ret == RET_OK; i++) {
            if (gutil_ring_get(r) != GINT_TO_POINTER(i)) {
                GDEBUG("Data get mismatch 2");
                ret = RET_ERR;
            }
        }
    }

    gutil_ring_unref(r);
    return ret;
}

/*==========================================================================*
 * Free
 *==========================================================================*/

static
void
test_free_func(
    gpointer ptr)
{
    int* data = ptr;
    (*data)++;
}

int
test_free()
{
    int data[5];
    const int n = G_N_ELEMENTS(data);
    const int drop = 2;
    int i, ret = RET_OK;
    GUtilRing* r = gutil_ring_new();

    gutil_ring_set_free_func(r, test_free_func);
    memset(data, 0, sizeof(data));
    for (i=0; i<n; i++) {
        gutil_ring_put(r, data + i);
    }

    /* Clear it twice */
    gutil_ring_clear(r);
    gutil_ring_clear(r);

    for (i=0; i<n && ret == RET_OK; i++) {
        if (data[i] != 1) {
            GDEBUG("Free function not called");
            ret = RET_ERR;
        }
    }

    for (i=0; i<n; i++) {
        gutil_ring_put(r, data + i);
    }
    gutil_ring_get(r);
    gutil_ring_unref(r);

    if (data[0] != 1) {
        GDEBUG("Free function called unexpectedly");
        ret = RET_ERR;
    } else {
        for (i=1; i<n; i++) {
            if (data[i] != 2) {
                GDEBUG("Free function not called by unref");
                ret = RET_ERR;
            }
        }
    }

    if (ret == RET_OK) {
        r = gutil_ring_new();
        gutil_ring_set_free_func(r, test_free_func);

        memset(data, 0, sizeof(data));
        for (i=0; i<n; i++) {
            gutil_ring_put(r, data + i);
        }

        if (gutil_ring_drop(r, drop) != drop ||
            gutil_ring_drop_last(r, drop) != drop ||
            gutil_ring_size(r) != (n - 2*drop)) {
            GDEBUG("Drop failed");
            ret = RET_ERR;
        }

        for (i=drop; i<(n-drop); i++) {
            if (gutil_ring_get(r) != data + i) {
                GDEBUG("Unexpected get");
                ret = RET_ERR;
            }
        }

        for (i=0; i<n; i++) {
            if (i < drop) {
                if (data[i] != 1) {
                    GDEBUG("Free function not called by drop");
                    ret = RET_ERR;
                }
            } else if (i >= (n-drop)) {
                if (data[i] != 1) {
                    GDEBUG("Free function not called by drop last");
                    ret = RET_ERR;
                }
            } else {
                if (data[i] != 0) {
                    GDEBUG("Free function called unexpectedly");
                    ret = RET_ERR;
                }
            }
        }

        gutil_ring_unref(r);
    }

    return ret;
}

/*==========================================================================*
 * Common
 *==========================================================================*/

static const TestDesc all_tests[] = {
    {
        "Basic",
        test_basic
    },{
        "PutFront",
        test_put_front
    },{
        "Drop",
        test_drop
    },{
        "DropLast",
        test_drop_last
    },{
        "Limit",
        test_limit
    },{
        "Free",
        test_free
    }
};

static
int
test_run_one(
    const TestDesc* desc)
{
    int ret = desc->run();
    GINFO("%s: %s", (ret == RET_OK) ? "OK" : "FAILED", desc->name);
    return ret;
}

static
int
test_run(
    const char* name)
{
    int i, ret;
    if (name) {
        const TestDesc* found = NULL;
        for (i=0, ret = RET_ERR; i<G_N_ELEMENTS(all_tests); i++) {
            const TestDesc* test = all_tests + i;
            if (!strcmp(test->name, name)) {
                ret = test_run_one(test);
                found = test;
                break;
            }
        }
        if (!found) GERR("No such test: %s", name);
    } else {
        for (i=0, ret = RET_OK; i<G_N_ELEMENTS(all_tests); i++) {
            int test_status = test_run_one(all_tests + i);
            if (ret == RET_OK && test_status != RET_OK) ret = test_status;
        }
    }
    return ret;
}

int main(int argc, char* argv[])
{
    int ret = RET_ERR;
    gboolean verbose = FALSE;
    GError* error = NULL;
    GOptionContext* options;
    GOptionEntry entries[] = {
        { "verbose", 'v', 0, G_OPTION_ARG_NONE, &verbose,
          "Enable verbose output", NULL },
        { NULL }
    };

    options = g_option_context_new("[TEST]");
    g_option_context_add_main_entries(options, entries, NULL);
    if (g_option_context_parse(options, &argc, &argv, &error)) {
        gutil_log_timestamp = FALSE;
        if (verbose) {
            gutil_log_default.level = GLOG_LEVEL_VERBOSE;
        }

        if (argc < 2) {
            ret = test_run(NULL);
        } else {
            int i;
            for (i=1, ret = RET_OK; i<argc; i++) {
                int test_status =  test_run(argv[i]);
                if (ret == RET_OK && test_status != RET_OK) ret = test_status;
            }
        }
    } else {
        fprintf(stderr, "%s\n", GERRMSG(error));
        g_error_free(error);
        ret = RET_ERR;
    }
    g_option_context_free(options);
    return ret;
}

/*
 * Local Variables:
 * mode: C
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
