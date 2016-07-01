/*
 * qmap.c
 * Qiu Chaofan, 2015/12/29
 *
 * Implementations of qmap_t type, using hash table.
 */

#include <string.h>
#include "qmap.h"
#include "syst/error.h"

qmap_t
qmap_create_sized(size_t persize, size_t num)
{
    qmap_t res = malloc(sizeof(struct qmap_struct));
    if (res == NULL || (res->data = malloc(persize * num)) == NULL) {
        handle_error(ERR_MEM_ALLOC_FAIL);
        return NULL;
    }
    res->persize  = persize;
    res->totalnum = num;
    memset(res->data, 0, persize * num);
    return res;
}

qmap_t
qmap_duplicate(const qmap_t item)
{
    qmap_t res;
    size_t size;
    if (item == NULL) {
        handle_error(ERR_MEM_ACCESS_NULL);
        return NULL;
    }
    size = item->persize * item->totalnum;
    res = malloc(sizeof(struct qmap_struct));
    if (res == NULL || (res->data = malloc(size)) == NULL) {
        handle_error(ERR_MEM_ALLOC_FAIL);
        return NULL;
    }
    res->persize  = item->persize;
    res->totalnum = item->totalnum;
    /* TODO: each element is a qmem_t! */
    memcpy(res->data, item->data, size);
    return res;
}

bool
qmap_element_exist(qmap_t item, qstr_t key)
{
}
