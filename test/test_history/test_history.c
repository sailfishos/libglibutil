/*
 * Copyright (C) 2017 Jolla Ltd.
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

#include "gutil_history.h"

static TestOpt test_opt;
static gint64 test_history_time;

static
gint64
test_history_time_func(void)
{
    return test_history_time;
}

/*==========================================================================*
 * NULL tolerance
 *==========================================================================*/

static
void
test_history_null(
    void)
{
    gutil_int_history_unref(NULL);
    gutil_int_history_clear(NULL);
    g_assert(!gutil_int_history_ref(NULL));
    g_assert(!gutil_int_history_size(NULL));
    g_assert(!gutil_int_history_interval(NULL));
    g_assert(!gutil_int_history_add(NULL, 1));
    g_assert(!gutil_int_history_median(NULL, 0));
    g_assert(!gutil_int_history_new(0, 0));
    g_assert(!gutil_int_history_new(1, 0));
    g_assert(!gutil_int_history_new(0, 1));
}

/*==========================================================================*
 * Basic
 *==========================================================================*/

static
void
test_history_basic(
    void)
{
    /* The default time functon returns real time, not usable for testing */
    GUtilIntHistory* h = gutil_int_history_new(1, 1);
    g_assert(gutil_int_history_size(h) == 0);
    g_assert(gutil_int_history_interval(h) == 0);
    gutil_int_history_unref(gutil_int_history_ref(h));
    gutil_int_history_unref(h);
}

/*==========================================================================*
 * Clear
 *==========================================================================*/

static
void
test_history_clear(
    void)
{
    GUtilIntHistory* h = gutil_int_history_new_full(2, 2,
        test_history_time_func);
    g_assert(gutil_int_history_median(h, 0) == 0);
    test_history_time = 2;
    g_assert(gutil_int_history_add(h, 1) == 1);
    /* Time hasn't changed, 1 is replaced with 2 */
    g_assert(gutil_int_history_add(h, 2) == 2);
    /* Time goes back, 2 is replaced with 3 */
    test_history_time -=1;
    g_assert(gutil_int_history_add(h, 3) == 3);
    test_history_time += 2;
    g_assert(gutil_int_history_add(h, 5) == 4);
    g_assert(gutil_int_history_size(h) == 2);
    g_assert(gutil_int_history_median(h, 0) == 4);
    g_assert(gutil_int_history_interval(h) == 1);
    gutil_int_history_clear(h);
    g_assert(!gutil_int_history_size(h));
    g_assert(!gutil_int_history_interval(h));
    gutil_int_history_unref(h);
}

/*==========================================================================*
 * Median
 *==========================================================================*/

static
void
test_history_median(
    void)
{
    GUtilIntHistory* h = gutil_int_history_new_full(2, 2,
        test_history_time_func);
    test_history_time = 1;
    g_assert(gutil_int_history_add(h, 1) == 1);
    test_history_time += 1;
    g_assert(gutil_int_history_add(h, 5) == 3);
    test_history_time += 1;
    /* Still the same, both ends of the interval are inclusive */
    g_assert(gutil_int_history_median(h, 0) == 3);
    test_history_time += 1;
    /* Now one entry expires */
    g_assert(gutil_int_history_median(h, 0) == 5);
    test_history_time += 1;
    /* The last one expires too */
    g_assert(gutil_int_history_median(h, 0) == 0);
    gutil_int_history_unref(h);
}

/*==========================================================================*
 * Size
 *==========================================================================*/

static
void
test_history_size(
    void)
{
    GUtilIntHistory* h = gutil_int_history_new_full(2, 2,
        test_history_time_func);
    test_history_time = 1;
    g_assert(gutil_int_history_add(h, 1) == 1);
    test_history_time += 1;
    g_assert(gutil_int_history_add(h, 5) == 3);
    g_assert(gutil_int_history_size(h) == 2);
    test_history_time += 1;
    /* Still the same, both ends of the interval are inclusive */
    g_assert(gutil_int_history_size(h) == 2);
    test_history_time += 1;
    /* Now one entry expires */
    g_assert(gutil_int_history_size(h) == 1);
    test_history_time += 1;
    /* The last one expires too */
    g_assert(gutil_int_history_size(h) == 0);
    gutil_int_history_unref(h);
}

/*==========================================================================*
 * Interval
 *==========================================================================*/

static
void
test_history_interval(
    void)
{
    GUtilIntHistory* h = gutil_int_history_new_full(2, 3,
        test_history_time_func);
    test_history_time = 1;
    g_assert(gutil_int_history_add(h, 1) == 1);
    test_history_time += 1;
    g_assert(gutil_int_history_add(h, 5) == 3);
    g_assert(gutil_int_history_interval(h) == 1);
    test_history_time += 1;
    /* Both ends of the interval are inclusive */
    g_assert(gutil_int_history_interval(h) == 2);
    test_history_time += 2;
    /* Now one entry with timestamp 1 expires, the one remaining has
     * timestamp 2, the current time is 5 so the interval becomes 3 */
    g_assert(gutil_int_history_interval(h) == 3);
    test_history_time += 1;
    /* The last one expires too */
    g_assert(gutil_int_history_interval(h) == 0);
    gutil_int_history_unref(h);
}

/*==========================================================================*
 * Data
 *==========================================================================*/

typedef struct test_history_sample {
    gint64 time;
    int value;
    int median;
} TestHistorySample;

typedef struct test_history_data {
    int max_size;
    gint64 max_interval;
    const TestHistorySample* samples;
    int count;
    guint size;
    gint64 interval;
} TestHistoryData;

static
void
test_history_data(
    gconstpointer test_data)
{
    int i;
    const TestHistoryData* data = test_data;
    GUtilIntHistory* h = gutil_int_history_new_full(data->max_size,
        data->max_interval, test_history_time_func);
    for (i=0; i<data->count; i++) {
        const TestHistorySample* sample = data->samples + i;
        test_history_time = sample->time;
        g_assert(gutil_int_history_add(h, sample->value) == sample->median);
    }
    g_assert(gutil_int_history_size(h) == data->size);
    g_assert(gutil_int_history_interval(h) == data->interval);
    test_history_time += data->max_interval + 1;
    g_assert(!gutil_int_history_size(h));
    g_assert(!gutil_int_history_interval(h));
    gutil_int_history_unref(h);
}

/*==========================================================================*
 * Common
 *==========================================================================*/
static const TestHistorySample samples1[] = {{1, 1, 1}};
static const TestHistoryData data1 = {
    1, 1, samples1, G_N_ELEMENTS(samples1), 1, 0
};

static const TestHistorySample samples2[] = {{1, 1, 1}, {3, 2, 2}};
static const TestHistoryData data2 = {
    1, 1, samples2, G_N_ELEMENTS(samples2), 1, 0
};

static const TestHistorySample samples3[] = {
    {1, 2, 2}, {3, 2, 2}, {4, 8, 3}
};
static const TestHistoryData data3 = {
    3, 4, samples3, G_N_ELEMENTS(samples3), 3, 3
};

static const TestHistorySample samples4[] = {
    {1, 2, 2}, {3, 2, 2}, {4, 8, 5}
};
static const TestHistoryData data4 = {
    3, 2, samples4, G_N_ELEMENTS(samples4), 2, 1
};

static const TestHistorySample samples5[] = {
    {1, 2, 2}, {3, 2, 2}, {4, 8, 5}
};
static const TestHistoryData data5 = {
    2, 4, samples5, G_N_ELEMENTS(samples5), 2, 1
};

#define TEST_PREFIX "/history/"

int main(int argc, char* argv[])
{
    g_test_init(&argc, &argv, NULL);
    g_test_add_func(TEST_PREFIX "null", test_history_null);
    g_test_add_func(TEST_PREFIX "basic", test_history_basic);
    g_test_add_func(TEST_PREFIX "clear", test_history_clear);
    g_test_add_func(TEST_PREFIX "median", test_history_median);
    g_test_add_func(TEST_PREFIX "size", test_history_size);
    g_test_add_func(TEST_PREFIX "interval", test_history_interval);
    g_test_add_data_func(TEST_PREFIX "data1", &data1, test_history_data);
    g_test_add_data_func(TEST_PREFIX "data2", &data2, test_history_data);
    g_test_add_data_func(TEST_PREFIX "data3", &data3, test_history_data);
    g_test_add_data_func(TEST_PREFIX "data4", &data4, test_history_data);
    g_test_add_data_func(TEST_PREFIX "data5", &data5, test_history_data);
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
