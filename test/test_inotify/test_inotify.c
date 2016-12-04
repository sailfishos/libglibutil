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

#include "test_common.h"

#include "gutil_inotify.h"
#include "gutil_macros.h"
#include "gutil_log.h"

#include <sys/inotify.h>

#define TEST_TIMEOUT (10) /* seconds */
#define TMP_DIR_TEMPLATE "test_inotify_XXXXXX"

static TestOpt test_opt;

typedef struct test_inotify {
    const char* name;
    GMainLoop* loop;
    GUtilInotifyWatch* watch1;
    GUtilInotifyWatch* watch2;
    char* dir1;
    char* dir2;
    guint timeout_id;
    gboolean array_free_count;
    gboolean ok;
} TestInotify;

static
gboolean
test_inotify_timeout(
    gpointer param)
{
    TestInotify* test = param;
    GERR("%s TIMEOUT", test->name);
    test->timeout_id = 0;
    g_main_loop_quit(test->loop);
    return G_SOURCE_REMOVE;
}

static
void
test_inotify_common(
    TestInotify* test,
    gboolean (*init)(TestInotify* test),
    void (*deinit)(TestInotify* test))
{
    const int mask = IN_ALL_EVENTS | IN_ONLYDIR | IN_EXCL_UNLINK;

    test->ok = FALSE;
    test->dir1 = g_dir_make_tmp(TMP_DIR_TEMPLATE, NULL);
    test->dir2 = g_dir_make_tmp(TMP_DIR_TEMPLATE, NULL);
    test->watch1 = gutil_inotify_watch_new(test->dir1, mask);
    test->watch2 = gutil_inotify_watch_new(test->dir2, mask);

    GDEBUG("%s: directory 1: %s", test->name, test->dir1);
    GDEBUG("%s: directory 2: %s", test->name, test->dir2);

    /* Initialize the event loop */
    test->loop = g_main_loop_new(NULL, TRUE);
    if (!(test_opt.flags & TEST_FLAG_DEBUG)) {
        test->timeout_id = g_timeout_add_seconds(TEST_TIMEOUT,
            test_inotify_timeout, test);
    }

    if (init) {
        init(test);
    }

    /* Run the event loop */
    g_main_loop_run(test->loop);

    if (deinit) {
        deinit(test);
    }

    g_assert(test->ok);
    if (!(test_opt.flags & TEST_FLAG_DEBUG)) {
        g_assert(test->timeout_id);
        g_source_remove(test->timeout_id);
    }

    g_main_loop_unref(test->loop);
    gutil_inotify_watch_destroy(test->watch1);
    gutil_inotify_watch_destroy(test->watch2);
    remove(test->dir1);
    remove(test->dir2);
    g_free(test->dir1);
    g_free(test->dir2);
}

/*==========================================================================*
 * Basic
 *==========================================================================*/

static
void
test_inotify_basic(
    void)
{
    char* dir = g_dir_make_tmp(TMP_DIR_TEMPLATE, NULL);
    GUtilInotifyWatch* watch = gutil_inotify_watch_new(dir, IN_ALL_EVENTS);

    /* These have no effect, just testing NULL-telerance */
    g_assert(!gutil_inotify_watch_new(NULL, 0));
    g_assert(!gutil_inotify_watch_ref(NULL));
    gutil_inotify_watch_unref(NULL);
    gutil_inotify_watch_destroy(NULL);
    gutil_inotify_watch_remove_handler(NULL, 0);
    gutil_inotify_watch_remove_handler(watch, 0);

    g_assert(!gutil_inotify_watch_callback_new(NULL, 0, NULL, NULL));
    g_assert(!gutil_inotify_watch_add_handler(NULL, NULL, NULL));
    g_assert(!gutil_inotify_watch_add_handler(watch, NULL, NULL));

    gutil_inotify_watch_destroy(watch);

    /* Remove the directory and try to watch it. That should fail */
    remove(dir);
    g_assert(!gutil_inotify_watch_new(dir, IN_ALL_EVENTS));
    g_free(dir);
}

/*==========================================================================*
 * Move
 *==========================================================================*/

typedef struct test_inotify_move {
    TestInotify test;
    const char* fname;
    char* dest;
    gulong id1;
    gulong id2;
    int from;
} TestInotifyMove;

static
void
test_inotify_move_from(
    GUtilInotifyWatch* watch,
    guint mask,
    guint cookie,
    const char* name,
    void* arg)
{
    TestInotifyMove* move = arg;
    if ((mask & IN_MOVED_FROM) && !g_strcmp0(move->fname, name)) {
        GDEBUG("%s moved from %s", name, move->test.dir1);
        move->from++;
    }
}

static
void
test_inotify_move_to(
    GUtilInotifyWatch* watch,
    guint mask,
    guint cookie,
    const char* name,
    void* arg)
{
    TestInotifyMove* move = arg;
    if ((mask & IN_MOVED_TO) && !g_strcmp0(move->fname, name)) {
        GDEBUG("%s moved to %s", name, move->test.dir2);
        if (move->from == 1) {
            move->test.ok = TRUE;
            g_main_loop_quit(move->test.loop);
        }
    }
}

static
gboolean
test_inotify_move_init(
    TestInotify* test)
{
    TestInotifyMove* move = G_CAST(test, TestInotifyMove, test);
    char* src;

    move->fname = "test";
    src = g_strconcat(test->dir1, G_DIR_SEPARATOR_S, move->fname, NULL);
    move->dest = g_strconcat(test->dir2, G_DIR_SEPARATOR_S, move->fname, NULL);
    move->id1 = gutil_inotify_watch_add_handler(test->watch1,
        test_inotify_move_from, move);
    move->id2 = gutil_inotify_watch_add_handler(test->watch2,
        test_inotify_move_to, move);
    g_file_set_contents(src, "contents", -1, NULL);
    GDEBUG("%s -> %s", src, move->dest);
    rename(src, move->dest);
    g_free(src);
    return TRUE;
}

static
void
test_inotify_move_deinit(
    TestInotify* test)
{
    TestInotifyMove* move = G_CAST(test, TestInotifyMove, test);
    gutil_inotify_watch_remove_handler(test->watch1, move->id1);
    gutil_inotify_watch_remove_handler(test->watch2, move->id2);
    remove(move->dest);
    g_free(move->dest);
}

static
void
test_inotify_move(
    void)
{
    TestInotifyMove move;
    memset(&move, 0, sizeof(move));
    move.test.name = "move";
    test_inotify_common(&move.test,
        test_inotify_move_init,
        test_inotify_move_deinit);
}

/*==========================================================================*
 * Callback
 *==========================================================================*/

typedef struct test_inotify_callback {
    TestInotify test;
    const char* fname;
    char* dest;
    GUtilInotifyWatchCallback* cb1;
    GUtilInotifyWatchCallback* cb2;
    int from;
} TestInotifyCallback;

static
void
test_inotify_callback_from(
    GUtilInotifyWatch* watch,
    guint mask,
    guint cookie,
    const char* name,
    void* arg)
{
    TestInotifyCallback* cb = arg;
    if ((mask & IN_MOVED_FROM) && !g_strcmp0(cb->fname, name)) {
        GDEBUG("%s moved from %s", name, cb->test.dir1);
        cb->from++;
    }
}

static
void
test_inotify_callback_to(
    GUtilInotifyWatch* watch,
    guint mask,
    guint cookie,
    const char* name,
    void* arg)
{
    TestInotifyCallback* cb = arg;
    if ((mask & IN_MOVED_TO) && !g_strcmp0(cb->fname, name)) {
        GDEBUG("%s moved to %s", name, cb->test.dir2);
        if (cb->from == 1) {
            cb->test.ok = TRUE;
            g_main_loop_quit(cb->test.loop);
        }
    }
}

static
gboolean
test_inotify_callback_init(
    TestInotify* test)
{
    TestInotifyCallback* cb = G_CAST(test, TestInotifyCallback, test);
    char* src;
    cb->fname = "test";
    src = g_strconcat(test->dir1, G_DIR_SEPARATOR_S, cb->fname, NULL);
    cb->dest = g_strconcat(test->dir2, G_DIR_SEPARATOR_S, cb->fname, NULL);
    cb->cb1 = gutil_inotify_watch_callback_new(test->dir1, IN_MOVED_FROM,
        test_inotify_callback_from, cb);
    cb->cb2 = gutil_inotify_watch_callback_new(test->dir2, IN_MOVED_TO,
        test_inotify_callback_to, cb);
    g_file_set_contents(src, "contents", -1, NULL);
    GDEBUG("%s -> %s", src, cb->dest);
    rename(src, cb->dest);
    g_free(src);
    return TRUE;
}

static
void
test_inotify_callback_deinit(
    TestInotify* test)
{
    TestInotifyCallback* cb = G_CAST(test, TestInotifyCallback, test);
    gutil_inotify_watch_callback_free(cb->cb1);
    gutil_inotify_watch_callback_free(cb->cb2);
    gutil_inotify_watch_callback_free(NULL);
    remove(cb->dest);
    g_free(cb->dest);
}

static
void
test_inotify_callback(
    void)
{
    TestInotifyCallback callback;
    memset(&callback, 0, sizeof(callback));
    callback.test.name = "callback";
    test_inotify_common(&callback.test,
        test_inotify_callback_init,
        test_inotify_callback_deinit);
}

/*==========================================================================*
 * Common
 *==========================================================================*/

#define TEST_PREFIX "/inotify/"

int main(int argc, char* argv[])
{
    g_test_init(&argc, &argv, NULL);
    g_test_add_func(TEST_PREFIX "basic", test_inotify_basic);
    g_test_add_func(TEST_PREFIX "move", test_inotify_move);
    g_test_add_func(TEST_PREFIX "callback", test_inotify_callback);
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
