/*
 * exec.c
 * Qiu Chaofan, 2015/12/26
 */

#include "infra/qstring.h"
#include "infra/qtree.h"
#include "syst/error.h"
#include "parse.h"
#include "runtime.h"
#include "exec.h"

#define NODE_TREE(x) (qmem_iter_getval((x), qtree_t))
#define NODE_TOK(x)  (qtree_data(qmem_iter_getval((x), qtree_t), grt_info))

static int  exec_single_stmt(qtree_t ctree);
static int  exec_dcl(qtree_t src);
static int  exec_blk(qtree_t blk);
static mobj exec_expr(qtree_t anatree);

int
mao_exec(qmem_t anatree)
{
    int end_status = 0;
    for (qmem_iter_t i = qmem_iter_new(anatree); !qmem_iter_end(i);
            qmem_iter_forward(&i)) {
        if (end_status = exec_single_stmt(NODE_TREE(i))) {
            add_err_queue("Executing failed.\n");
            break;
        }
    }
    return end_status;
}

static int
exec_single_stmt(qtree_t ctree)
{
    qmem_iter_t       cnode = qmem_iter_new(ctree->childsptr, qtree_t);
    int              status = 0;
    int   loop_dcl_scope_id = -1;

    switch (qtree_data(ctree, grt_info).type) {

    case GRT_DECLARATION:
        exec_dcl(NODE_TREE(cnode));
        break;

    case GRT_FOR:
        if (NODE_TOK(cnode).type == GRT_DECLARATION) {
            loop_dcl_scope_id = exec_dcl(NODE_TREE(cnode))；
            qmem_iter_forward(&cnode);
        } else {
            exec_expr(NODE_TREE(cnode));
            qmem_iter_forward(&cnode);
        }
        qtree_t cond   = NODE_TREE(cnode);
        qmem_iter_forward(&cnode);
        qtree_t action = NODE_TREE(cnode);
        qmem_iter_forward(&cnode);
        for (; fetch_val(exec_expr(cond)); exec_expr(action)) {
            exec_blk(NODE_TREE(cnode));
        }
        if (loop_dcl_scope_id != -1) {
            mao_unregister_var_scope(loop_dcl_scope_id);
        }
        break;

    case GRT_FOREACH:
        if (NODE_TOK(cnode).type == GRT_DECLARATION) {
            loop_dcl_scope_id = exec_dcl(NODE_TREE(cnode));
            qmem_iter_forward(&cnode);
        } else {
            qtree_t loop_var = NODE_TREE(cnode);
            qmem_iter_forward(&cnode);
        }
        if (mao_setvar(loop_var, NODE_TREE(cnode))) {
            qmem_iter_forward(&cnode);
            do {
                exec_blk(NODE_TREE(cnode));
            } while (mao_setvar(loop_var, NULL));
        }
        if (loop_dcl_scope_id != -1) {
            mao_unregister_var_scope(loop_dcl_scope_id);
        }
        break;

    case GRT_DOWHILE:
        qtree_t cond = NODE_TREE(cnode);
        qmem_iter_forward(&cnode);
        exec_blk(NODE_TREE(cnode));
    case GRT_WHILE:
        while (fetch_val(exec_expr(cond))) {
            exec_blk(NODE_TREE(cnode));
        }
        break;

    case GRT_IF:
        while (!qmem_iter_end(cnode)) {
            if (NODE_TOK(cnode).type == GRT_EXPRESSION) {
                if (fetch_val(exec_expr(NODE_TREE(cnode)))) {
                    qmem_iter_forward(&cnode);
                    exec_blk(NODE_TREE(cnode));
                    qmem_iter_forward(&cnode);
                    break;
                } else {
                    qmem_iter_forward(&cnode);
                    qmem_iter_forward(&cnode);
                }
            /* When reaching the 'else' clause. */
            } else if (NODE_TOK(cnode).type == GRT_BLOCK) {
                exec_blk(NODE_TREE(cnode));
                break;
            }
        }
        break;

    case GRT_BLOCK:
        exec_blk(NODE_TREE(cnode));
        break;

    case GRT_COMMAND:
        if (NODE_TOK(cnode).type == TOKEN_USE) {
            mao_module(MODULE_USE, NODE_TOK(cnode).name);
        } else if (NODE_TOK(cnode).type == TOKEN_UNUSE) {
            mao_module(MODULE_UNUSE, NODE_TOK(cnode).name);
        }
        break;

    case GRT_EXPRESSION:
        exec_expr(NODE_TREE(cnode));
        break;

    case GRT_NULL:
    case GRT_EMPTY:
    default:
        break;
    }

    return status;
}

/*
 * One way to solve the problem that we cannot find scope of each
 * variable: Each time we declare one or more variable(s), record
 * the scope id allocated. When the scope is over, release all variables
 * from the scope id we recorded in the local list.
 */
static int
exec_dcl(qtree_t src)
{
    static int scope_it_count = 1;
    qmem_iter_t dcl_iter = qmem_iter_new(src->childsptr, qtree_t);
    qmem_iter_t tmp;
    int type             = mao_get_type(NODE_TOK(dcl_iter).type);
    qmem_iter_forward(&dcl_iter);
    mobj current_var_obj;
    while (!qmem_iter_end(dcl_iter)) {
        tmp = dcl_iter;
        qmem_iter_forward(&dcl_iter);
        if (!qmem_iter_end(dcl_iter)) {
            if (NODE_TOK(dcl_iter).type != GRT_IDENTIFIER) {
                mao_register_variable(type, NODE_TOK(tmp).name, 
                        exec_expr(NODE_TREE(dcl_iter)));
            } else {
                mao_register_variable(type, NODE_TOK(tmp).name, NULL);
            }
        }
    }
    return scope_int_count++;
}

static int
exec_blk(qtree_t src)
{
    qmem_iter_t cnode = qmem_iter_new(src->childsptr);
    qmem_t      current_scope_id;
    while (!qm_iter_end(cnode)) {
        /* When declaration, record the scope ids. */
        if (NODE_TOK(cnode).type == GRT_DECLARATION) {
            qmem_append(exec_dcl(NODE_TREE(cnode)));
        } else {
            exec_single_stmt(NODE_TREE(cnode));
        }
        qmem_iter_forward(&cnode);
    }
    cnode = qmem_iter_new(current_scope_id);
    /* Release the variables alloced in block. */
    while (!qm_iter_end(cnode)) {
        mao_unregister_variable_byscope(qm_iter_getval(cnode, int));
    }
    return 0;
}

/* just another code generation */
#define EXEC_EXPR_PART_GEN \
    do { \
        opnum1 = exec_expr(NODE_TREE(cnode)); \
        qmem_iter_forward(&cnode); \
        optp = NODE_TOK(cnode).type; \
        qmem_iter_forward(&cnode); \
        opnum2 = exec_expr(NODE_TREE(cnode)); \
    } while (0)
#define ISCONST(x) \
    ((x) == TOKEN_NUMBER_INT || (x) == TOKEN_NUMBER_FLOAT || \
     (x) == TOKEN_TRUE || (x) == TOKEN_FALSE || (x) == TOKEN_LITERAL)

static mobj
exec_expr(const qtree_t anatree)
{
    qmem_iter_t cnode = qmem_iter_new(anatree->childsptr);
    mobj opnum1 = NULL, opnum2;
    int optp;
    switch (qtree_data(anatree, grt_info).type) {

    case GRT_EXPR_ASSIGN:
        EXEC_EXPR_PART_GEN;
        switch (optp) {
        case TOKEN_OP_ASSIGN:
            return mao_obj_assign(opnum1, opnum2);
        case TOKEN_OP_ADDE:
            return mao_obj_adde(opnum1, opnum2);
        case TOKEN_OP_SUBE:
            return mao_obj_sube(opnum1, opnum2);
        case TOKEN_OP_MULE:
            return mao_obj_mule(opnum1, opnum2);
        case TOKEN_OP_DIVE:
            return mao_obj_dive(opnum1, opnum2);
        case TOKEN_OP_MODE:
            return mao_obj_mode(opnum1, opnum2);
        case TOKEN_OP_EXPE:
            return mao_obj_expe(opnum1, opnum2);
        default:
            break;
        }
        break;

    case GRT_EXPR_ORRES:
        EXEC_EXPR_PART_GEN;
        if (optp == TOKEN_OP_OR) {
            return mao_obj_or(opnum1, opnum2);
        }
        break;

    case GRT_EXPR_ANDRES:
        EXEC_EXPR_PART_GEN;
        if (optp == TOKEN_OP_AND) {
            return mao_obj_and(opnum1, opnum2);
        }
        break;

    case GRT_EXPR_ECOMP:
    case GRT_EXPR_NECOMP:
        EXEC_EXPR_PART_GEN;
        return mao_obj_comp(opnum1, opnum2, optp);

    case GRT_EXPR_TERM:
        EXEC_EXPR_PART_GEN;
        switch (optp) {
        case TOKEN_OP_ADD:
            return mao_obj_add(opnum1, opnum2);
        case TOKEN_OP_SUB:
            return mao_obj_sub(opnum1, opnum2);
        default:
            break;
        }
        break;

    case GRT_EXPR_FACTOR:
        EXEC_EXPR_PART_GEN;
        switch (optp) {
        case TOKEN_OP_MUL:
            return mao_obj_add(opnum1, opnum2);
        case TOKEN_OP_DIV:
            return mao_obj_sub(opnum1, opnum2);
        case TOKEN_OP_EXP:
            return mao_obj_exp(opnum1, opnum2);
        default:
            break;
        }
        break;

    case GRT_EXPR_POWER:
        EXEC_EXPR_PART_GEN;
        if (optp == TOKEN_OP_EXP) {
            return mao_obj_and(opnum1, opnum2);
        }
        break;

    case GRT_EXPR_SINGLE:
        optp = NODE_TOK(cnode).type;
        qmem_iter_forward(&cnode);
        opnum1 = exec_expr(NODE_TREE(cnode));
        if (optp == TOKEN_OP_NOT) {
            return mao_obj_not(opnum1);
        } else if (optp == TOKEN_OP_ADD || optp == TOKEN_OP_SUB) {
            return mao_obj_sign(opnum1, optp);
        }
        break;

    case GRT_EXPR_ARRV:
        if (NODE_TOK(cnode).type == TOKEN_IDENTIFIER) {
            opnum1 = mao_get_variable(NODE_TOK(cnode).name);
        /* for the constants */
        } else if (ISCONST(NODE_TOK(cnode).type)) {
            opnum1 = mao_get_constant(cnode);
        /* when blocked by paren is an expression */
        } else {
            opnum1 = exec_expr(NODE_TREE(cnode));
        }
        qmem_iter_forward(&cnode);
        while (!qmem_iter_end(cnode)) {
            if (NODE_TOK(cnode).type == TOKEN_LBRACKET) {
                qmem_iter_forward(&cnode);
            }
            opnum2 = mao_get_element(opnum1, 
                    exec_expr(NODE_TREE(cnode)));
            opnum1 = opnum2;
        }
        return opnum1;

    default:
        break;
    }
    return NULL;
}

#undef ISCONST
#undef EXEC_EXPR_PART_GEN

