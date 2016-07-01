/*
 * runtime.h
 * Qiu Chaofan, 2015/12/27
 */

#ifndef MAOLANG_RUNTIME_H_
#define MAOLANG_RUNTIME_H_

#include <stdbool.h>
#include "infra/qmemory.h"
#include "infra/qstring.h"
#include "infra/qmap.h"
#include "infra/qtree.h"

#define MAO_OBJ_BOOL    1   /* 000001 */
#define MAO_OBJ_INT     3   /* 000011 */
#define MAO_OBJ_DOUBLE  7   /* 000111 */
#define MAO_OBJ_STRING  8   /* 001000 */
#define MAO_OBJ_LIST    31  /* 011111 */
#define MAO_OBJ_MAP     47  /* 101111 */

struct mvar_struct {
    int scope_id;
    int id;
    qstr_t name;
    struct mobject *vobj;
};

typedef struct mvar_struct * mvar;

/*
 * Type representation in mobject:
 *
 * 000001 bool
 * 000011 int
 * 000111 double
 * 001000 string
 * 011111 list
 * 101111 map
 *
 * This kind of representation is easy for implicit type cast.
 * For these types, we do the 'and' operation, if the result is
 * non-zero, that means the two types are consistent, at least
 * for basic types. How? See an example:
 *
 * b/a;
 *
 * In the expression 'b/a', assuming that type of b is double and
 * that of a is int. 011 & 111 = 011 != 0, so the two variables
 * can do division. And the result type is the two do 'or', so that
 * is 011 & 111 = 111, namely double.
 *
 * For container types, namely list and map, they're consistent with any
 * basic types. But a map cannot append with a list nor a list cannot
 * append with a map. A 'string map' can operates with 'string' (append).
 */
struct mobject_struct {
    int refcnt;
    int type;
    union {
        bool   bval;
        int    ival;
        double dval;
        qstr_t sval;
        qmem_t lstv;
        qmap_t mapv;
    } v;
};

typedef struct mobject_struct * mobj;

/*
 * Each variable in mao language has a scope, 0 by default, which
 * means it's global.
 */
mvar mao_register_variable(int type, qstr_t var_name, mobj assignment);
int mao_unregister_variable_byscope(int scope_id);
int mao_unregister_variable_byname(const qstr_t var_name);

mobj mao_get_variable(qstr_t name);
mobj mao_get_constant(qtree_t src);

/*
 * basic arithmetic operators:
 * + - * / mod ^
 */
mobj mao_obj_add(const mobj o1, const mobj o2);
mobj mao_obj_sub(const mobj o1, const mobj o2);
mobj mao_obj_mul(const mobj o1, const mobj o2);
mobj mao_obj_div(const mobj o1, const mobj o2);
mobj mao_obj_mod(const mobj o1, const mobj o2);
mobj mao_obj_exp(const mobj o1, const mobj o2);

/*
 * assignment operators:
 * = += -= *= /= %= ^=
 */
const mobj mao_obj_assign(mobj dst, const mobj src);
const mobj mao_obj_adde(mobj dst, const mobj src);
const mobj mao_obj_sube(mobj dst, const mobj src);
const mobj mao_obj_mule(mobj dst, const mobj src);
const mobj mao_obj_dive(mobj dst, const mobj src);
const mobj mao_obj_mode(mobj dst, const mobj src);
const mobj mao_obj_expe(mobj dst, const mobj src);

/*
 * comparison operators:
 * < > <= >= == !=
 */
mobj mao_obj_comp(const mobj o1, const mobj o2, int op);

/*
 * logical and sign operators:
 * and or not + -
 */
mobj mao_obj_and(const mobj o1, const mobj o2);
mobj mao_obj_or(const mobj o1, const mobj o2);
mobj mao_obj_not(const mobj item);
mobj mao_obj_sign(const mobj item, int op);

/*
 * list/map access operators:
 * []
 */
mobj mao_get_element(const mobj src, const mobj key);

/*
 * rank operator
 */
mobj mao_obj_rank(const mobj container, const mobj item);

static inline
fetch_val(mobj src);

#endif //MAOLANG_RUNTIME_H_
