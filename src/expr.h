/*
 * expr.h
 * Qiu Chaofan, 2016/1/1
 */

#ifndef MAOLANG_EXPR_H_
#define MAOLANG_EXPR_H_

#include "infra/qmemory.h"
#include "runtime.h"
#include "lex.h"

/*
 * Structure of expression tree node.
 */
struct mao_expr_struct {
    struct mao_expr_struct *left_child;
    struct mao_expr_struct *right_child;
    union {
        mobj val;
        enum {
            ADD        = TOKEN_OP_ADD,
            SUB        = TOKEN_OP_SUB,
            MUL        = TOKEN_OP_MUL,
            DIV        = TOKEN_OP_DIV,
            ASSIGN     = TOKEN_OP_ASSIGN,
            ADD_ASSIGN = TOKEN_OP_ADDE,
            SUB_ASSIGN = TOKEN_OP_SUBE,
            MUL_ASSIGN = TOKEN_OP_MULE,
            DIV_ASSIGN = TOKEN_OP_DIVE,
            NEG, POS
        } op;
    };
};

typedef struct mao_expr_struct *mao_expr;

mobj mao_expr_calc(mao_expr src);
mao_expr mao_parse_expr(qmem_iter_t start_pos, qmem_iter_t end_pos);

#endif //MAOLANG_EXPR_H_
