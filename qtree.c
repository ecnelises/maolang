/*
 * qtree.c
 * Qiu Chaofan, 2015/12/16
 */

#include <stdlib.h>
#include "qtree.h"

qtree_t
qtree_create_sized(size_t persize, size_t per_blk_size)
{
    qtree_t res = malloc(sizeof(struct qtree_node));
    if (res == NULL) {
        handle_error(ERR_MEM_ALLOC_FAIL);
        return NULL;
    }
    
    res->parent = NULL;
    res->childsptr = qmem_create_sized(persize, per_blk_size);
    res->data = malloc(persize);
    
    return res;
}

void
qtree_free(qtree_t item)
{
    if (item == NULL) {
        handle_error(ERR_MEM_ACCESS_NULL);
        return;
    }
    
    for (qmem_iter_t eachchild = qmem_iter_new(item->childsptr);
         !qmem_iter_end(eachchild);
         qmem_iter_forward(&eachchild)) {
        qtree_free(qmem_iter_getval(eachchild, qtree_t));
    }
    qmem_free(item->childsptr);
    free(item->data);
    free(item);
}
