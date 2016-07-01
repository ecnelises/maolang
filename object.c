/*
 * object.c
 * Qiu Chaofan, 2015/12/28
 */

#include "runtime.h"
#include "syst/error.h"

#define CHECK_ARG(x, y) \
    if ((x) == NULL || (y) == NULL) { \
        handle_error(ERR_MEM_ACCESS_NULL); \
        return NULL; \
    }
#define CHECK_ALLOC(x) \
    if ((x) == NULL) { \
        handle_error(ERR_MEM_ALLOC_FAIL); \
        return NULL; \
    }

mobj mao_obj_add(const mobj o1, const mobj o2)
{
    CHECK_ARG(o1, o2);
    mobj res = malloc(sizeof(struct mobject_struct));
    CHECK_ALLOC(res);
    /* consistent or 'map and list' */
    if ((o1->type) & (o2->type)) {
        /* basic type */
        if (!(((o1->type) | (o2->type)) & 48)) {
            res->type = (o1->type) | (o2->type);
        }
    }
    return NULL;
}
