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

struct qmap_struct {
    void      *data;
    size_t  persize;
    size_t totalnum;
};

typedef qmap_struct * qmap_t;

#define QMAP_LEN_DEFAULT 512
#define qmap_create(type) (qmap_create_sized(sizeof(qmem_t), QMAP_LEN_DEFAULT))

qmap_t qmap_create_sized(size_t persize, size_t num);
qmap_t qmap_duplicate(const qmap_t item);

bool qmap_element_exist(qmap_t item, qstr_t key);
/*
 * TODO: need rewrite!
 */
#define qmap_fetch(item, key, type) \
    (qmem_iter_getval(qmap_find_iter_in_qmem(item, key), type))

qmem_iter_t qmap_find_iter_in_qmem(qmap_t item, qstr_t key);

void qmap_delete_item(qmap_t item, qstr_t key);
void qmap_free(qmap_t item);

#endif //MAOLANG_QMAP_H_
