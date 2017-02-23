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

#include "test_common.h"

#include "gutil_ring.h"

static TestOpt test_opt;

/*==========================================================================*
 * Basic
 *==========================================================================*/

static
void
test_basic(
    void)
{
    int i, n = 5;
    GUtilRing* r = gutil_ring_new();

    /* Test NULL tolerance */
    g_assert(!gutil_ring_ref(NULL));
    gutil_ring_unref(NULL);
    gutil_ring_set_free_func(NULL, NULL);
    gutil_ring_set_free_func(r, NULL);
    gutil_ring_set_free_func(NULL, g_free);
    gutil_ring_clear(NULL);
    gutil_ring_compact(NULL);
    gutil_ring_reserve(NULL, 0);
    g_assert(!gutil_ring_size(NULL));
    g_assert(!gutil_ring_can_put(NULL, 1));
    g_assert(!gutil_ring_put(NULL, NULL));
    g_assert(!gutil_ring_put_front(NULL, NULL));
    g_assert(!gutil_ring_get(NULL));
    g_assert(!gutil_ring_data_at(NULL, 0));
    g_assert(!gutil_ring_get_last(NULL));
    g_assert(!gutil_ring_drop(NULL, 1));
    g_assert(!gutil_ring_drop_last(NULL, 1));
    g_assert(!gutil_ring_flatten(NULL, NULL));

    g_assert(gutil_ring_ref(r) == r);
    gutil_ring_unref(r);

    /* Put some data in */
    for (i=0; i<n; i++) {
        g_assert(gutil_ring_can_put(r, 1));
        g_assert(gutil_ring_put(r, GINT_TO_POINTER(i)));
    }

    /* Access at invalid index */
    g_assert(!gutil_ring_data_at(r, -1));
    g_assert(!gutil_ring_data_at(r, n));

    /* Peek the data */
    for (i=0; i<n; i++) {
        g_assert(gutil_ring_data_at(r, i) == GINT_TO_POINTER(i));
    }

    /* Get the data */
    for (i=0; i<n; i++) {
        gutil_ring_flatten(r, NULL);
        gutil_ring_compact(r);
        g_assert(gutil_ring_get(r) == GINT_TO_POINTER(i));
    }

    /* There should be nothing left */
    g_assert(!gutil_ring_get(r));

    gutil_ring_compact(r);
    gutil_ring_compact(r);
    gutil_ring_clear(r);
    g_assert(!gutil_ring_flatten(r, &i));
    g_assert(!i);

    gutil_ring_unref(r);
}

/*==========================================================================*
 * PutFront
 *==========================================================================*/

static
void
test_put_front(
    void)
{
    int i, n = 5;
    GUtilRing* r = gutil_ring_new();

    for (i=0; i<n; i++) {
        g_assert(gutil_ring_can_put(r, 1));
        g_assert(gutil_ring_put_front(r, GINT_TO_POINTER(n-i-1)));
    }

    for (i=0; i<n; i++) {
        gutil_ring_compact(r);
        g_assert(gutil_ring_get(r) == GINT_TO_POINTER(i));
    }

    /* The same but take it out with get_last */
    for (i=0; i<n; i++) {
        g_assert(gutil_ring_can_put(r, 1));
        g_assert(gutil_ring_put_front(r, GINT_TO_POINTER(n-i-1)));
    }

    for (i=0; i<n; i++) {
        gutil_ring_compact(r);
        g_assert(gutil_ring_get_last(r) == GINT_TO_POINTER(n-i-1));
    }

    g_assert(!gutil_ring_get_last(r));
    gutil_ring_unref(r);
}

/*==========================================================================*
 * Drop
 *==========================================================================*/

static
void
test_drop(
    void)
{
    int i, n = 5, get = 3, drop = 3;
    GUtilRing* r = gutil_ring_sized_new(0,n);

    /* [01234] */
    for (i=0; i<n; i++) {
        gutil_ring_put(r, GINT_TO_POINTER(i));
    }

    /* ...[34] */
    for (i=0; i<get; i++) {
        g_assert(gutil_ring_get(r) == GINT_TO_POINTER(i));
    }

    /* 567][34 */
    for (i=0; i<get; i++) {
        gutil_ring_put(r, GINT_TO_POINTER(n+i));
    }

    /* ..[67].. */
    g_assert(gutil_ring_drop(r, drop) == drop);
    for (i=0; i<(n-drop); i++) {
        g_assert(gutil_ring_get(r) == GINT_TO_POINTER(get+drop+i));
    }

    /* Drop more than the size of the buffer (i.e. clear it) */
    for (i=0; i<n; i++) {
        gutil_ring_put(r, GINT_TO_POINTER(i));
    }

    g_assert(!gutil_ring_drop(r, 0));
    g_assert(gutil_ring_drop(r, n+1) == n);
    g_assert(!gutil_ring_drop(r, 1));
    g_assert(!gutil_ring_size(r));

    gutil_ring_unref(r);
}

/*==========================================================================*
 * DropLast
 *==========================================================================*/

static
void
test_drop_last(
    void)
{
    int i, n = 5, get = 2, drop = 3;
    GUtilRing* r = gutil_ring_sized_new(0,n);

    /* [01234] */
    for (i=0; i<n; i++) {
        gutil_ring_put(r, GINT_TO_POINTER(i));
    }

    /* ..[234] */
    for (i=0; i<get; i++) {
        g_assert(gutil_ring_get(r) == GINT_TO_POINTER(i));
    }

    /* 56][234 */
    for (i=0; i<get; i++) {
        gutil_ring_put(r, GINT_TO_POINTER(n+i));
    }

    /* ..[23]. */
    g_assert(gutil_ring_drop_last(r, drop) == drop);
    gutil_ring_flatten(r, NULL);
    for (i=0; i<(n-drop); i++) {
        g_assert(gutil_ring_get(r) == GINT_TO_POINTER(get+i));
    }

    /* Drop more than the size of the buffer (i.e. clear it) */
    for (i=0; i<n; i++) {
        gutil_ring_put(r, GINT_TO_POINTER(i));
    }

    g_assert(!gutil_ring_drop_last(r, 0));
    g_assert(gutil_ring_drop_last(r, n+1) == n);
    g_assert(!gutil_ring_drop_last(r, 1));
    g_assert(!gutil_ring_size(r));

    gutil_ring_unref(r);
}

/*==========================================================================*
 * Limit
 *==========================================================================*/

static
void
test_limit(
    void)
{
    int i, limit = 5, extra = 2;
    gint size;
    gpointer* data;
    GUtilRing* r = gutil_ring_sized_new(2, limit);

    g_assert(gutil_ring_reserve(r, limit));
    g_assert(!gutil_ring_reserve(r, limit+1));

    for (i=0; i<limit; i++) {
        g_assert(gutil_ring_can_put(r, 1));
        g_assert(gutil_ring_put(r, GINT_TO_POINTER(i)));
    }

    g_assert(!gutil_ring_can_put(r, 1));
    g_assert(gutil_ring_get_last(r) == GINT_TO_POINTER(limit-1));
    g_assert(gutil_ring_get_last(r) == GINT_TO_POINTER(limit-2));
    g_assert(gutil_ring_put(r, GINT_TO_POINTER(limit-2)));
    g_assert(gutil_ring_put(r, GINT_TO_POINTER(limit-1)));

    for (i=0; i<extra; i++) {
        g_assert(gutil_ring_get(r) == GINT_TO_POINTER(i));
        g_assert(gutil_ring_can_put(r, 1));
        g_assert(gutil_ring_put(r, GINT_TO_POINTER(i+limit)));
    }

    g_assert(gutil_ring_size(r) == limit);
    data = gutil_ring_flatten(r, &size);
    g_assert(data);
    g_assert(size == limit);

    for (i=0; i<size; i++) {
        g_assert(data[i] == GINT_TO_POINTER(i+extra));
    }

    g_assert(gutil_ring_get_last(r) == GINT_TO_POINTER(size+extra-1));
    gutil_ring_compact(r);
    data = gutil_ring_flatten(r, &size);
    g_assert(data);
    g_assert(size == limit-1);
    for (i=0; i<size; i++) {
        g_assert(data[i] == GINT_TO_POINTER(i+extra));
    }

    gutil_ring_clear(r);

    for (i=0; i<limit; i++) {
        gutil_ring_put(r, GINT_TO_POINTER(i));
    }
    
    for (i=0; i<limit; i++) {
        g_assert(gutil_ring_get(r) == GINT_TO_POINTER(i));
    }

    gutil_ring_unref(r);
}

/*==========================================================================*
 * MaxSize
 *==========================================================================*/

static
void
test_max_size(
    void)
{
    int i;
    const int n = 5;
    GUtilRing* r = gutil_ring_sized_new(0, -2);

    g_assert(!gutil_ring_max_size(NULL));
    g_assert(gutil_ring_max_size(r) == GUTIL_RING_UNLIMITED_SIZE);

    gutil_ring_set_max_size(NULL, n); /* This one shouldn't crash */
    gutil_ring_set_max_size(r, n);
    for (i=0; i<n; i++) {
        g_assert(gutil_ring_put(r, GINT_TO_POINTER(i)));
    }

    /* The buffer is full, the next put should fail */
    g_assert(!gutil_ring_put(r, GINT_TO_POINTER(i)));
    g_assert(gutil_ring_size(r) == n);
    gutil_ring_set_max_size(r, n);
    g_assert(gutil_ring_size(r) == n);

    /* Allow more space */
    gutil_ring_set_max_size(r, 2*n);
    for (i=0; i<n; i++) {
        g_assert(gutil_ring_put(r, GINT_TO_POINTER(i+n)));
    }

    /* The buffer is full again */
    g_assert(!gutil_ring_put(r, GINT_TO_POINTER(i)));
    g_assert(gutil_ring_size(r) == 2*n);

    /* Shrink it */
    gutil_ring_set_max_size(r, n);
    g_assert(gutil_ring_size(r) == n);
    for (i=0; i<n; i++) {
        g_assert(gutil_ring_get(r) == GINT_TO_POINTER(i+n));
    }
    g_assert(!gutil_ring_size(r));

    /* Negative == unlimited */
    gutil_ring_set_max_size(r, -2);
    g_assert(gutil_ring_max_size(r) == GUTIL_RING_UNLIMITED_SIZE);
    gutil_ring_unref(r);
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

static
void
test_free(
    void)
{
    int data[5];
    const int n = G_N_ELEMENTS(data);
    const int drop = 2;
    int i;
    GUtilRing* r = gutil_ring_new();

    gutil_ring_set_free_func(r, test_free_func);
    memset(data, 0, sizeof(data));
    for (i=0; i<n; i++) {
        gutil_ring_put(r, data + i);
    }

    /* Clear it twice */
    gutil_ring_clear(r);
    gutil_ring_clear(r);

    /* Make sure that test_free_func has been called */
    for (i=0; i<n; i++) {
        g_assert(data[i] == 1);
    }

    for (i=0; i<n; i++) {
        gutil_ring_put(r, data + i);
    }
    gutil_ring_get(r);
    gutil_ring_unref(r);

    /* test_free_func shouldn't be invoked for the element we retreived */
    g_assert(data[0] == 1);
    for (i=1; i<n; i++) {
        g_assert(data[i] == 2);
    }

    r = gutil_ring_new();
    gutil_ring_set_free_func(r, test_free_func);

    memset(data, 0, sizeof(data));
    for (i=0; i<n; i++) {
        gutil_ring_put(r, data + i);
    }

    g_assert(gutil_ring_drop(r, drop) == drop);
    g_assert(gutil_ring_drop_last(r, drop) == drop);
    g_assert(gutil_ring_size(r) == (n - 2*drop));

    for (i=drop; i<(n-drop); i++) {
        g_assert(gutil_ring_get(r) == data + i);
    }

    for (i=0; i<n; i++) {
        if (i < drop) {
            /* Invoked by drop */
            g_assert(data[i] == 1);
        } else if (i >= (n-drop)) {
            /* Invoked by drop last */
            g_assert(data[i] == 1);
        } else {
            /* Not invoked for those we have retreived */
            g_assert(data[i] == 0);
        }
    }

    gutil_ring_unref(r);
}

/*==========================================================================*
 * Common
 *==========================================================================*/

#define TEST_PREFIX "/ring/"

int main(int argc, char* argv[])
{
    g_test_init(&argc, &argv, NULL);
    g_test_add_func(TEST_PREFIX "basic", test_basic);
    g_test_add_func(TEST_PREFIX "put_front", test_put_front);
    g_test_add_func(TEST_PREFIX "drop", test_drop);
    g_test_add_func(TEST_PREFIX "drop_last", test_drop_last);
    g_test_add_func(TEST_PREFIX "max_size", test_max_size);
    g_test_add_func(TEST_PREFIX "limit", test_limit);
    g_test_add_func(TEST_PREFIX "free", test_free);
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
