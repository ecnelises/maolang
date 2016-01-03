/*
 * qmemory.c Qiu Chaofan, 2015/11/21
 *
 * Implementations of functions related to qmemory declared in qmemory.h
 */

#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "qmemory.h"

qmem_t
qmem_create_sized(size_t persize, size_t per_blk_size)
{
    qmem_t        res = qalloc(sizeof(struct qmemory_struct));

    res->blklen = per_blk_size;
    res->blknum = 0;
    res->persize = persize;
    res->unwritten = res->blklen;
    res->head = NULL;
    res->tail = NULL;
    
    return res;
}

qmem_t
qmem_duplicate(const qmem_t item)
{
    assert(item != NULL);
    qmem_t res = qalloc(sizeof(struct qmemory_struct));

    res->blknum = item->blknum;
    res->blklen = item->blklen;
    res->persize = item->persize;
    res->unwritten = res->blklen;
    
    struct qmem_node *p = NULL, *index = NULL;
    struct qmem_node *dp = item->head;
    
    for (size_t i = 0; i < item->blknum && dp != NULL; ++i) {
        index = qalloc(sizeof(struct qmem_node));
        index->v = qalloc(res->persize * res->blklen);

        memcpy(index->v, dp->v, res->persize * res->blklen);
        index->last = p;
        if (p != NULL) {
            p->next = index;
        } else {
            res->head = index;
        }
        index->next = NULL;
        dp = dp->next;
        p = index;
        index = index->next;
    }
    res->tail = p;
    res->unwritten = item->unwritten;
    
    return res;
}

void
qmem_lessen(qmem_t item, size_t dst_len)
{
    assert(item != NULL);
    size_t         dst_blknum;
    struct qmem_node     *itr;
    struct qmem_node *itr_tmp;
    
    itr = item->head;
    
    if (item->blknum == 0 || dst_len > (item->blknum - 1) * (item->blklen)) {
        item->unwritten = dst_len % item->blklen;
        return;
    }
    dst_blknum =
    (dst_len / (item->blklen)) + (dst_len % (item->blklen) ==
                                  0 ? 0 : 1);
    /*
     * Shrink to least blocks able to contain elements at the number of
     * dst_len
     */
    
    for (int i = 0; i < dst_blknum; ++i) {
        itr = itr->next;
    }
    
    item->blknum = dst_blknum;
    if (dst_blknum == 0) {
        item->head = NULL;
        item->tail = NULL;
    } else {
        itr->last->next = NULL;
        item->tail = itr->last;
    }
    
    while (itr != NULL) {
        itr_tmp = itr;
        itr = itr->next;
        free(itr_tmp->v);
        free(itr_tmp);
    }
    
    if (dst_len == 0 || dst_len % item->blklen == 0) {
        item->unwritten = item->blklen;
    } else {
        item->unwritten = dst_len % item->blklen;
    }
}

qmem_iter_t
qmem_iter_new(const qmem_t item)
{
    qmem_iter_t    res;
    
    assert(item != NULL);
    res.current_read_blk = item->head;
    res.current_read_seek = 0;
    res.source = item;
    
    return res;
}

void
qmem_iter_forward(qmem_iter_t * item)
{
    if (qmem_iter_end(*item)) {
        return;
    }
    ++(item->current_read_seek);
    if (item->current_read_seek > item->source->blklen - 1) {
        item->current_read_seek = 0;
        item->current_read_blk = item->current_read_blk->next;
    }
}

void
qmem_iter_backward(qmem_iter_t * item)
{
    if (item->current_read_blk == NULL) {
        item->current_read_blk = item->source->tail;
        item->current_read_seek = item->source->blklen - 1;
        return;
    }
    if (item->current_read_seek == 0) {
        item->current_read_seek = item->source->blklen - 1;
        item->current_read_blk = item->current_read_blk->last;
    } else {
        --(item->current_read_seek);
    }
}

void
qmem_delete_item(qmem_iter_t pos)
{
    qmem_t item = pos.source;
    if (item->blklen != 1 || item->blknum == 0) {
        return;
    }
    if (qmem_iter_end(pos)) {
        return;
    }
    struct qmem_node *tmp = pos.current_read_blk;
    if (tmp->last != NULL) {
        tmp->last->next = tmp->next;
    }
    if (tmp->next != NULL) {
        tmp->next->last = tmp->last;
    }
    free(tmp->v);
    free(tmp);
    --(item->blknum);
}
