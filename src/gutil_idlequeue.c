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

#include "gutil_idlequeue.h"
#include "gutil_log.h"

typedef struct gutil_idle_queue_item GUtilIdleQueueItem;

struct gutil_idle_queue_item {
    GUtilIdleQueueItem* next;
    GUtilIdleQueue* queue;
    GUtilIdleQueueTag tag;
    gpointer data;
    GUtilIdleFunc run;
    GFreeFunc destroy;
    guint source_id;
};

struct gutil_idle_queue {
    gint ref_count;
    GUtilIdleQueueItem* first;
    GUtilIdleQueueItem* last;
};

static
gboolean
gutil_idle_queue_item_run(
    gpointer data)
{
    GUtilIdleQueueItem* item = data;
    GUtilIdleQueue* q = item->queue;
    /* We assume that items are executed in the same order in which they
     * were added to the queue */
    GASSERT(q->first == item);
    GASSERT(item->source_id);
    /* Remove it from the list */
    q->first = item->next;
    if (!q->first) {
        q->last = NULL;
    }
    item->next = NULL;
    item->source_id = 0;
    item->queue = NULL;
    /* Invoke the external callback */
    if (item->run) {
        item->run(item->data);
    }
    return G_SOURCE_REMOVE;
}

static
void
gutil_idle_queue_item_destroy(
    gpointer data)
{
    GUtilIdleQueueItem* item = data;
    GASSERT(!item->source_id);
    GASSERT(!item->queue);
    if (item->destroy) {
        item->destroy(item->data);
    }
    g_slice_free(GUtilIdleQueueItem, item);
}

GUtilIdleQueue*
gutil_idle_queue_new()
{
    GUtilIdleQueue* q = g_slice_new0(GUtilIdleQueue);
    q->ref_count = 1;
    return q;
}

void
gutil_idle_queue_free(
    GUtilIdleQueue* q)
{
    gutil_idle_queue_cancel_all(q);
    gutil_idle_queue_unref(q);
}

GUtilIdleQueue*
gutil_idle_queue_ref(
    GUtilIdleQueue* q)
{
    if (G_LIKELY(q)) {
        GASSERT(q->ref_count > 0);
        g_atomic_int_inc(&q->ref_count);
    }
    return q;
}

void
gutil_idle_queue_unref(
    GUtilIdleQueue* q)
{
    if (G_LIKELY(q)) {
        GASSERT(q->ref_count > 0);
        if (g_atomic_int_dec_and_test(&q->ref_count)) {
            gutil_idle_queue_cancel_all(q);
            g_slice_free(GUtilIdleQueue, q);
        }
    }
}

void
gutil_idle_queue_add(
    GUtilIdleQueue* q,
    GUtilIdleFunc run,
    gpointer data)
{
    gutil_idle_queue_add_tag_full(q, 0, run, data, NULL);
}

void
gutil_idle_queue_add_full(
    GUtilIdleQueue* q,
    GUtilIdleFunc run,
    gpointer data,
    GFreeFunc free)
{
    gutil_idle_queue_add_tag_full(q, 0, run, data, free);
}

void
gutil_idle_queue_add_tag(
    GUtilIdleQueue* q,
    GUtilIdleQueueTag tag,
    GUtilIdleFunc run,
    gpointer data)
{
    gutil_idle_queue_add_tag_full(q, tag, run, data, NULL);
}

void
gutil_idle_queue_add_tag_full(
    GUtilIdleQueue* q,
    GUtilIdleQueueTag tag,
    GUtilIdleFunc run,
    gpointer data,
    GFreeFunc destroy)
{
    if (G_LIKELY(q)) {
        GUtilIdleQueueItem* item = g_slice_new0(GUtilIdleQueueItem);

        /* Fill the item */
        item->queue = q;
        item->tag = tag;
        item->run = run;
        item->destroy = destroy;
        item->data = data;

        /* Add it to the queue */
        if (q->last) {
            GASSERT(q->first);
            q->last->next = item;
        } else {
            GASSERT(!q->first);
            q->first = item;
        }
        q->last = item;

        /* Schedule the callback */
        item->source_id = g_idle_add_full(G_PRIORITY_DEFAULT_IDLE,
            gutil_idle_queue_item_run, item, gutil_idle_queue_item_destroy);
    } else if (destroy) {
        destroy(data);
    }
}

gboolean
gutil_idle_queue_contains_tag(
    GUtilIdleQueue* q,
    GUtilIdleQueueTag tag)
{
    if (G_LIKELY(q)) {
        const GUtilIdleQueueItem* item;
        for (item = q->first; item; item = item->next) {
            if (item->tag == tag) {
                return TRUE;
            }
        }
    }
    return FALSE;
}

void
gutil_idle_queue_cancel_first(
    GUtilIdleQueue* q)
{
    GUtilIdleQueueItem* item = q->first;
    guint source_id = item->source_id;
    q->first = item->next;
    if (!q->first) {
        q->last = NULL;
    }
    item->queue = NULL;
    item->source_id = 0;
    g_source_remove(source_id);
}

gboolean
gutil_idle_queue_cancel_tag(
    GUtilIdleQueue* q,
    GUtilIdleQueueTag tag)
{
    if (G_LIKELY(q) && q->first) {
        GUtilIdleQueueItem* item = q->first;
        if (item->tag == tag) {
            gutil_idle_queue_cancel_first(q);
            return TRUE;
        } else {
            GUtilIdleQueueItem* prev = item;
            for (item = item->next; item; prev = item, item = item->next) {
                if (item->tag == tag) {
                    guint source_id = item->source_id;
                    if (item->next) {
                        prev->next = item->next;
                    } else {
                        GASSERT(q->last == item);
                        prev->next = NULL;
                        q->last = prev;
                    }
                    item->queue = NULL;
                    item->source_id = 0;
                    g_source_remove(source_id);
                    return TRUE;
                }
            }
        }
    }
    return FALSE;
}

void
gutil_idle_queue_cancel_all(
    GUtilIdleQueue* q)
{
    if (G_LIKELY(q)) {
        while (q->first) {
            gutil_idle_queue_cancel_first(q);
        }
    }
}

/*
 * Local Variables:
 * mode: C
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
