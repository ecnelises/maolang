/*
 * qmemory.h
 * Qiu Chaofan, 2015/11/21
 *
 * Interfaces for dynamic memory allocating and releasing.
 * I defined qmem_t(qmemory) type, which is a pointer to qmemory_struct.
 *
 * Implementation of qmemory is powered by block-list.
 * As a list, qmemory has several nodes which pointing at last and next,
 * but each node has a block containing more than one element.
 */

#ifndef MAOLANG_QMEMORY_H_
#define MAOLANG_QMEMORY_H_

#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include "error.h"

struct qmem_node {
    struct qmem_node *last;
    struct qmem_node *next;
    void                *v;
};

struct qmemory_struct {
    struct qmem_node *head;	    /* head block of list */
    struct qmem_node *tail;	    /* tail block of list */
    size_t          blknum;	    /* number of blocks */
    size_t          blklen;	    /* length of each block */
    size_t         persize;	    /* size of each element */
    size_t       unwritten;	    /* first position unwritten */
};

/*
 * The following `iterator` is for an easier reading method.
 * current_read_blk is pointing at the 'current' pointed block.
 * current_read_seek is the 'current' position at current_read_blk
 */
struct qmemory_iterator {
    struct qmemory_struct      *source;
    struct qmem_node *current_read_blk;
    size_t           current_read_seek;
};

typedef struct qmemory_struct *qmemory;
typedef qmemory qmem_t;
typedef struct qmemory_iterator qmemory_iterator;
typedef qmemory_iterator qmem_iter_t;

#define QMEM_LEN_DEFAULT 16
#define qmem_create(type) (qmem_create_sized(sizeof(type), QMEM_LEN_DEFAULT))

qmem_t qmem_create_sized(size_t persize, size_t per_blk_size);
qmem_t qmem_duplicate(const qmem_t item);

/*
 * Using macros instead of functions is for generics.
 * We can pass a type parameter for the object to be appended.
 * And 'while(0)' is to avoid the occasion that
 *
 * if ()
 *     qmem_append(xxx);
 * else
 *     some code;
 *
 * If we use '{}' here, the macro will be expanded as
 *
 * if ()
 *     { };
 * else
 *     some code;
 *
 * And the compiler will give an error.
 */
#define qmem_append(dst, item, type) \
    do { \
        struct qmem_node * new_tail; \
        if ((dst->unwritten) > (dst->blklen) - 1) { \
            new_tail = qalloc(sizeof(struct qmem_node)); \
            new_tail->v = qalloc(sizeof(type)*(dst->blklen)); \
            memset(new_tail->v, 0, sizeof(type)*(dst->blklen));\
            if (dst->blknum == 0) { \
                new_tail->last = NULL; \
                new_tail->next = NULL; \
                dst->head = new_tail; \
                dst->tail = new_tail; \
            } else { \
                dst->tail->next = new_tail; \
                new_tail->last = dst->tail; \
                new_tail->next = NULL; \
                dst->tail = new_tail; \
            } \
            dst->blknum += 1; \
            dst->unwritten = 0; \
        } \
        ((type *)(dst->tail->v))[(dst->unwritten)++] = item; \
    } while(0)

#define qmem_replace(dst, place, item, type) \
    do { \
        struct qmem_node *itr; \
        if (dst == NULL) { handle_error(ERR_MEM_ACCESS_NULL); break; } \
        if (dst->blknum == 0) { handle_error(ERR_MEM_REPLACE_OUT); break; } \
        if (place > ((dst->blknum - 1) * (dst->blklen) + (dst->unwritten) - 1)) { handle_error(ERR_MEM_REPLACE_OUT); break; } \
        itr = dst->head; \
        for (int i = 0; i < (dst->blknum) - 1; ++i) { itr = itr->next; } \
        ((type*)(itr->v))[place % (dst->blklen)] = item; \
    } while (0)

void qmem_lessen(qmem_t item, size_t dest_len);

#define qmem_clear(item) \
    do { \
        qmem_lessen(item, 0); \
    } while (0)

#define qmem_free(item) \
    do { \
        qmem_clear(item); \
        free(item); \
    } while(0)

#define qmem_empty(item) \
    ((item) != NULL && (item)->blknum == 0)

static inline size_t
qmem_len(const qmem_t item)
{
    if (qmem_empty(item)) {
        return 0;
    }
    return (item->blknum - 1) * item->blklen + item->unwritten;
}

#define qmem_iter_eq(x, y) \
    ((x).source == (y).source && (x).current_read_blk == (y).current_read_blk && \
     (x).current_read_seek == (y).current_read_seek)

qmem_iter_t qmem_iter_new(const qmem_t item);

static inline bool
qmem_iter_end(const qmem_iter_t item)
{
    if ((item.source)->blknum == 0 || item.current_read_blk == NULL) {
        return true;
    }
    if (item.current_read_blk == (item.source)->tail &&
        item.current_read_seek >= (item.source)->unwritten) {
        return true;
    }
    return false;
}

void qmem_iter_backward(qmem_iter_t * item);
void qmem_iter_forward(qmem_iter_t * item);
#define qmem_iter_getval(item, type) \
    (((type*)(((item).current_read_blk)->v))[(item).current_read_seek])

/*
 * Only for lists (when blklen == 1)
 */
void qmem_delete_item(qmem_iter_t pos);

#endif				// MAOLANG_QMEMORY_H_
