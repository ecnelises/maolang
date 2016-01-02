/*
 * runtime.h
 * Qiu Chaofan, 2015/12/27
 *
 * Defines object, variable structure and functions.
 * Every variable has a corresponding object. Its type
 * and value are stored in the object.
 */

#ifndef MAOLANG_RUNTIME_H_
#define MAOLANG_RUNTIME_H_

#include <stdbool.h>
#include <assert.h>
#include "infra/qmemory.h"
#include "infra/qstring.h"
#include "infra/qmap.h"

#define MAO_OBJ_CONFLICT 0  /* 000 */
#define MAO_OBJ_INT      1  /* 001 */
#define MAO_OBJ_DOUBLE   3  /* 011 */

struct mobject_struct {
    int type;
    union {
        int    ival;
        double dval;
    };
};

typedef struct mobject_struct * mobj;

struct mvar_struct {
    int  id;
    mobj vobj;
};

typedef struct mvar_struct * mvar;

mvar mao_register_variable(int type, qstr_t var_name);
mobj mao_get_variable_obj(qstr_t name);

#define OBJ_INIT_INT    1
#define OBJ_INIT_DOUBLE 2

mobj mao_obj_new(int init_type, ...);
void print_obj(mobj item, FILE *fp);

/*
 * basic arithmetic operators:
 * + - * /
 */
mobj mao_obj_add(mobj o1, mobj o2);
mobj mao_obj_sub(mobj o1, mobj o2);
mobj mao_obj_mul(mobj o1, mobj o2);
mobj mao_obj_div(mobj o1, mobj o2);

/*
 * assignment operators:
 * = += -= *= /=
 */
mobj mao_obj_assign(mobj dst, mobj src);
mobj mao_obj_adde(mobj dst, mobj src);
mobj mao_obj_sube(mobj dst, mobj src);
mobj mao_obj_mule(mobj dst, mobj src);
mobj mao_obj_dive(mobj dst, mobj src);
mobj mao_obj_sign(mobj item, bool negative);

extern qmem_t global_memory_list;
extern qmap_t variable_list;

#define global_memory_register(address) qmem_append(global_memory_list, address, void*)
void global_memory_clean(void);

#endif //MAOLANG_RUNTIME_H_

