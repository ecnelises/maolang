/*
 * qtree.h
 * Qiu Chaofan, 2015/12/15
 *
 * Interfaces for tree data structure.
 *
 * type: qtree_t
 */

#ifndef MAOLANG_QTREE_H_
#define MAOLANG_QTREE_H_

#include <string.h>
#include "qmemory.h"

/* Another challenge for structure design is also generics. */
struct qtree_node {
    struct qtree_node *parent;
    qmem_t childsptr;
    void *data;
};

typedef struct qtree_node * qtree;
typedef qtree  qtree_t;

#define QTREE_CHILDN_DEFAULT 4
#define qtree_create(type)     (qtree_create_sized(sizeof(type), QTREE_CHILDN_DEFAULT))

qtree_t qtree_create_sized(size_t persize, size_t per_blk_size);

#define qtree_add_child(dst, item) \
    do { \
        qmem_append(dst->childsptr, item, qtree_t); \
    } while(0)

#define qtree_add_child_data(dst, dataptr, type) \
    do { \
        qtree_t _res = qtree_create(type); \
        memcpy(_res->data, dataptr, sizeof(type)); \
        qmem_append(dst->childsptr, _res, qtree_t); \
        _res->parent = dst; \
    } while (0)

#define qtree_data(dst, type) \
    (((type*)(dst->data))[0])

void qtree_free(qtree_t item);

#endif //MAOLANG_QTREE_H_

