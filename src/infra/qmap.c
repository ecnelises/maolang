/*
 * qmap.c
 * Qiu Chaofan, 2015/12/29
 *
 * Implementations of qmap_t type, using hash table.
 */

#include <string.h>
#include <assert.h>
#include "qmap.h"
#include "error.h"

qmap_t
qmap_create_sized(size_t persize, size_t num)
{
    qmap_t res = qalloc(sizeof(struct qmap_struct));
    res->pool = qalloc(num * sizeof(qmem_t));
    res->persize  = persize;
    res->totalnum = num;
    memset(res->pool, 0, num * sizeof(qmem_t));
    return res;
}

qmap_t
qmap_duplicate(const qmap_t item)
{
    qmap_t res;
    assert(item != NULL);
    res = qalloc(sizeof(struct qmap_struct));
    res->pool = qalloc(item->totalnum * sizeof(qmem_t));
    res->persize  = item->persize;
    res->totalnum = item->totalnum;
    memcpy(res->pool, item->pool, item->totalnum * sizeof(qmem_t));
    struct qmap_key_store_struct tmp;
    
    for (size_t i = 0; i < item->totalnum; ++i) {
        if ((item->pool)[i] != NULL) {
            (res->pool)[i] = qmem_create_sized(sizeof(struct qmap_key_store_struct), 1);
            for (qmem_iter_t j = qmem_iter_new((item->pool)[i]);
                 !qmem_iter_end(j); qmem_iter_forward(&j)) {
                tmp.key = qstr_duplicate(qmem_iter_getval(j, struct qmap_key_store_struct).key);
                tmp.data = qalloc(item->persize);
                memcpy(tmp.data, qmem_iter_getval(j, struct qmap_key_store_struct).data, item->persize);
                qmem_append((res->pool)[i], tmp, struct qmap_key_store_struct);
            }
        }
    }
    return res;
}

/*
 * This is a simple but useful hash algorithm found in the book
 * The C Programming Language.
 */
static size_t
qmap_hash(qstr_t key, size_t domain_max)
{
    size_t hash = 0;
    for (qstr_iter_t i = qstr_iter_new(key); qstr_iter_getval(i) != '\0' &&
         !qstr_iter_end(i);
         qstr_iter_forward(&i)) {
        hash = hash * 131 + qstr_iter_getval(i);
    }
    return hash % domain_max;
}

qmem_t
qmap_get_qmap_plus(qmap_t item, qstr_t key)
{
    size_t hashcode = qmap_hash(key, item->totalnum);
    if ((item->pool)[hashcode] == NULL) {
        return (item->pool)[hashcode] = qmem_create_sized(sizeof(struct qmap_key_store_struct), 1);
    }
    return (item->pool)[hashcode];
}

void *
qmap_find_iter_in_qmem(qmap_t item, qstr_t key)
{
    size_t hashcode = qmap_hash(key, item->totalnum);
    if ((item->pool)[hashcode] == NULL) {
        return NULL;
    }
    for (qmem_iter_t i = qmem_iter_new((item->pool)[hashcode]);
         !qmem_iter_end(i); qmem_iter_forward(&i)) {
        if (!qstr_comp(key, qmem_iter_getval(i, struct qmap_key_store_struct).key)) {
            return qmem_iter_getval(i, struct qmap_key_store_struct).data;
        }
    }
    return NULL;
}

void
qmap_delete_item(qmap_t item, qstr_t key)
{
    size_t hashcode = qmap_hash(key, item->totalnum);
    if ((item->pool)[hashcode] == NULL) {
        return;
    }
    for (qmem_iter_t i = qmem_iter_new((item->pool)[hashcode]);
         !qmem_iter_end(i); qmem_iter_forward(&i)) {
        if (!qstr_comp(key, qmem_iter_getval(i, struct qmap_key_store_struct).key)) {
            free(qmem_iter_getval(i, struct qmap_key_store_struct).key);
            free(qmem_iter_getval(i, struct qmap_key_store_struct).data);
            qmem_delete_item(i);
            if (qmem_len(i.source) == 0) {
                free(i.source);
                (item->pool)[hashcode] = NULL;
            }
            break;
        }
    }
}

void
qmap_free(qmap_t item)
{
    for (size_t i = 0; i < item->totalnum; ++i) {
        if ((item->pool)[i] != NULL) {
            for (qmem_iter_t j = qmem_iter_new((item->pool)[i]);
                 !qmem_iter_end(j); qmem_iter_forward(&j)) {
                qstr_free(qmem_iter_getval(j, struct qmap_key_store_struct).key);
                free(qmem_iter_getval(j, struct qmap_key_store_struct).data);
            }
            qmem_free((item->pool)[i]);
        }
    }
    free(item->pool);
    free(item);
}
