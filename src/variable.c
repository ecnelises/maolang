/*
 * variable.c
 * Qiu Chaofan, 2015/12/31
 */

#include "infra/qmemory.h"
#include "infra/qstring.h"
#include "infra/qmap.h"
#include "runtime.h"
#include "error.h"

mvar
mao_register_variable(int type, qstr_t var_name)
{
    static int var_id_list = 1;
    if (qmap_element_exist(variable_list, var_name)) {
        add_err_queue("Redefinition of variable.\n");
        return NULL;
    }
    mvar res = qalloc(sizeof(struct mvar_struct));
    res->id = var_id_list++;
    res->vobj = qalloc(sizeof(struct mobject_struct));
    res->vobj->type = type;
    
    if (type == MAO_OBJ_INT) {
        res->vobj->ival = 0;
    } else if (type == MAO_OBJ_DOUBLE) {
        res->vobj->dval = 0.0;
    }

    qmap_add(variable_list, var_name, res, mvar);
    return res;
}

mobj
mao_get_variable_obj(qstr_t name)
{
    if (!qmap_element_exist(variable_list, name)) {
        return NULL;
    }
    return qmap_fetch(variable_list, name, mvar)->vobj;
}
