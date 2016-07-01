/*
 * qmemory.c Qiu Chaofan, 2015/11/21
 * 
 * Implementations of functions related to qmemory declared in qmemory.h
 */

#include <string.h>
#include <stdlib.h>
#include "qmemory.h"

qmem_t 
qmem_create_sized(size_t persize, size_t per_blk_size)
{
    qmem_t        res = malloc(sizeof(struct qmemory_struct));

    if (res == NULL) {
        handle_error(ERR_MEM_ALLOC_FAIL);
        return res;
    }
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
    if (item == NULL) {
        handle_error(ERR_MEM_ACCESS_NULL);
        return NULL;
    }
    qmem_t        res = malloc(sizeof(struct qmemory_struct));

    if (res == NULL) {
        handle_error(ERR_MEM_ALLOC_FAIL);
        return res;
    }
    res->blknum = item->blknum;
    res->blklen = item->blklen;
    res->persize = item->persize;
    res->unwritten = res->blklen;

    struct qmem_node *p = NULL, *index = NULL;
    struct qmem_node *dp = item->head;

    for (size_t i = 0; i < item->blknum && dp != NULL; ++i) {
        index = malloc(sizeof(struct qmem_node));
        if (index == NULL) {
            handle_error(ERR_MEM_ALLOC_FAIL);
        }
        index->v = malloc(res->persize * res->blklen);
        if (index->v == NULL) {
            handle_error(ERR_MEM_ALLOC_FAIL);
        }
        memcpy(index->v, dp->v, res->persize * res->blklen);
        index->last = p;
        if (p != NULL) {
            p->next = index;
            index->next = NULL;
        } else {
            res->head = index;
        }
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
    size_t         dst_blknum;
    struct qmem_node     *itr;
    struct qmem_node *itr_tmp;

    if (item == NULL) {
        handle_error(ERR_MEM_ACCESS_NULL);
        return;
    }
    itr = item->head;

    if (dst_len > (item->blknum - 1) * (item->blklen) || item->blknum == 0) {
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

    if (item == NULL) {
        handle_error(ERR_MEM_ACCESS_NULL);
        return (qmem_iter_t) {
            NULL, NULL, 0
        };
    }
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
