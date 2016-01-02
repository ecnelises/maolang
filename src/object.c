/*
 * object.c
 * Qiu Chaofan, 2015/12/28
 *
 * Calculation of Mao is powered by mao-object.
 */

#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <stdarg.h>
#include "infra/qmemory.h"
#include "infra/qmap.h"
#include "infra/qstring.h"
#include "lex.h"
#include "runtime.h"
#include "error.h"

/* Select member by its type */
#define TYPE_SELECT(x) \
((x)->type == MAO_OBJ_INT ? (x)->ival : (x)->dval)

/* Assign member by its type */
#define TYPE_ASSIGN(item, x) \
(item->type == MAO_OBJ_INT ? (item->ival = (x)) : (item->dval = (x)))

/*
 *   INT  | DOUBLE = DOUBLE
 *   INT  |   INT  = INT
 * DOUBLE | DOUBLE = DOUBLE
 */
#define MAO_GET_TYPE(x, y) ((x) | (y))

#define MAO_NUL(x, y) (y)
#define MAO_ADD(x, y) ((x) + (y))
#define MAO_SUB(x, y) ((x) - (y))
#define MAO_MUL(x, y) ((x) * (y))
#define MAO_DIV(x, y) ((x) / (y))

/*
 * All functions of operation is of the same form.
 * So I use macro to simplify code.
 */

#define make_operation_functions(name, op) \
    mobj \
    mao_obj_##name(mobj o1, mobj o2) \
    { \
        assert(o1 != NULL); \
        assert(o2 != NULL); \
        mobj res = qalloc(sizeof(struct mobject_struct)); \
        global_memory_register(res); \
        res->type = MAO_GET_TYPE(o1->type, o2->type); \
        (TYPE_ASSIGN(res, op(TYPE_SELECT(o1), TYPE_SELECT(o2)))); \
        return res; \
    }

make_operation_functions(add, MAO_ADD)
make_operation_functions(sub, MAO_SUB)
make_operation_functions(mul, MAO_MUL)
make_operation_functions(div, MAO_DIV)

#define make_assignment_functions(name, op) \
    mobj \
    mao_obj_##name(mobj dst, mobj src) \
    { \
        assert(dst != NULL); \
        assert(src != NULL); \
        TYPE_ASSIGN(dst, op(TYPE_SELECT(dst), TYPE_SELECT(src))); \
        return dst; \
    }

make_assignment_functions(assign, MAO_NUL)
make_assignment_functions(adde, MAO_ADD)
make_assignment_functions(sube, MAO_SUB)
make_assignment_functions(mule, MAO_MUL)
make_assignment_functions(dive, MAO_DIV)


/*
 * Calculate sign operator: '-' or '+'
 */
mobj
mao_obj_sign(mobj item, bool negative)
{
    assert(item != NULL);
    
    if (!negative) {
        return item;
    }
    mobj res = qalloc(sizeof(struct mobject_struct));
    global_memory_register(res);
    switch (item->type) {
        case MAO_OBJ_INT:
            res->type = MAO_OBJ_INT;
            res->ival = -(item->ival);
            break;
        case MAO_OBJ_DOUBLE:
            res->type = MAO_OBJ_DOUBLE;
            res->dval = -(item->ival);
            break;
        default:
            res->type = MAO_OBJ_CONFLICT;
            break;
    }
    return res;
}

/*
 * Create a new object and choosing init type.
 */
mobj
mao_obj_new(int init_type, ...)
{
    assert(init_type == OBJ_INIT_INT || init_type == OBJ_INIT_DOUBLE);
    mobj res = qalloc(sizeof(struct mobject_struct));
    global_memory_register(res);
    va_list ap;
    va_start(ap, init_type);
    if (init_type == OBJ_INIT_INT) {
        res->type = MAO_OBJ_INT;
        res->ival = va_arg(ap, int);
    } else if (init_type == OBJ_INIT_DOUBLE) {
        res->type = MAO_OBJ_DOUBLE;
        res->dval = va_arg(ap, double);
    }
    va_end(ap);
    return res;
}

void
print_obj(mobj item, FILE *fp)
{
    assert(item != NULL);
    if (item->type == MAO_OBJ_INT) {
        fprintf(fp, "%d\n", item->ival);
    } else if (item->type == MAO_OBJ_DOUBLE) {
        fprintf(fp, "%.6lf\n", item->dval);
    }
}

/*
 * Clean the temporary list.
 */
void
global_memory_clean(void)
{
    for (qmem_iter_t iter = qmem_iter_new(global_memory_list);
         !qmem_iter_end(iter); qmem_iter_forward(&iter)) {
        free(qmem_iter_getval(iter, void*));
    }
    qmem_clear(global_memory_list);
}
