/*
 * qmap.h
 * Qiu Chaofan, 2015/12/29
 *
 * Definition of qmap_t type, associative array.
 */

#ifndef MAOLANG_QMAP_H_
#define MAOLANG_QMAP_H_

#include <stdbool.h>
#include "qmemory.h"
#include "qstring.h"

struct qmap_key_store_struct {
    qstr_t     key;
    void     *data;
};

struct qmap_struct {
    qmem_t    *pool;
    size_t  persize;
    size_t totalnum;
};

typedef struct qmap_struct * qmap_t;

#define QMAP_LEN_DEFAULT 512
#define qmap_create(type) (qmap_create_sized(sizeof(qmem_t), QMAP_LEN_DEFAULT))

qmap_t qmap_create_sized(size_t persize, size_t num);
qmap_t qmap_duplicate(const qmap_t item);
void *qmap_find_iter_in_qmem(qmap_t item, qstr_t key);

#define qmap_element_exist(item, key) \
    (qmap_find_iter_in_qmem(item, key) != NULL)

#define qmap_fetch(item, key, type) \
    (((type*)(qmap_find_iter_in_qmem(item, key)))[0])

#define qmap_add(item, _key, value, type) \
    do { \
        qmem_t rpos = qmap_get_qmap_plus(item, _key); \
        struct qmap_key_store_struct _tmp; \
        _tmp.key = qstr_duplicate(_key); \
        _tmp.data = qalloc(sizeof(item->persize)); \
        ((type*)(_tmp.data))[0] = value; \
        qmem_append(rpos, _tmp, struct qmap_key_store_struct); \
    } while (0)

qmem_t qmap_get_qmap_plus(qmap_t item, qstr_t key);

void qmap_delete_item(qmap_t item, qstr_t key);
void qmap_free(qmap_t item);

#endif //MAOLANG_QMAP_H_
