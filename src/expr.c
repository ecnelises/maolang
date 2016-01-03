/*
 * expr.c
 * Qiu Chaofan, 2016/1/1
 *
 * Expression parsing code, using binary tree.
 */

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <stdbool.h>
#include <assert.h>
#include "infra/qmemory.h"
#include "lex.h"
#include "expr.h"
#include "runtime.h"

/* If both child is null, that means the expression is a value, not operator */
#define SELECT_VAL(x) \
    (((x)->left_child == NULL && (x)->right_child == NULL) ? (x)->val : mao_expr_calc(x))

/*
 * Calculate expression tree from `mao_parse_expr`
 */
mobj
mao_expr_calc(mao_expr src)
{
    assert(src != NULL);
    mobj tmp;
    if (src->left_child == NULL && src->right_child == NULL) {
        return src->val;
    } else {
        switch (src->op) {
        case ADD:
            return mao_obj_add(SELECT_VAL(src->left_child), SELECT_VAL(src->right_child));
        case SUB:
            return mao_obj_sub(SELECT_VAL(src->left_child), SELECT_VAL(src->right_child));
        case MUL:
            return mao_obj_mul(SELECT_VAL(src->left_child), SELECT_VAL(src->right_child));
        case DIV:
            tmp = SELECT_VAL(src->right_child);
            if (tmp->type == MAO_OBJ_DOUBLE) {
                if (tmp->dval == 0.0) {
                    printf("divided by ZERO\n");
                    exit(1);
                }
            } else {
                if (tmp->ival == 0) {
                    printf("divided by ZERO\n");
                    exit(1);
                }
            }
            return mao_obj_div(SELECT_VAL(src->left_child), SELECT_VAL(src->right_child));
        case ASSIGN:
            return mao_obj_assign(src->left_child->val, SELECT_VAL(src->right_child));
        case ADD_ASSIGN:
            return mao_obj_adde(src->left_child->val, SELECT_VAL(src->right_child));
        case SUB_ASSIGN:
            return mao_obj_sube(src->left_child->val, SELECT_VAL(src->right_child));
        case MUL_ASSIGN:
            return mao_obj_mule(src->left_child->val, SELECT_VAL(src->right_child));
        case DIV_ASSIGN:
            return mao_obj_dive(src->left_child->val, SELECT_VAL(src->right_child));
        case NEG:
            return mao_obj_sign(SELECT_VAL(src->left_child), true);
        case POS:
            return mao_obj_sign(SELECT_VAL(src->left_child), false);
        default:
            return NULL;
        }
    }
}

/* Using macros for simplifying code */
#define TOK_CURTOK(x)  qmem_iter_getval((x), struct token)
#define TOK_CURTYPE(x) qmem_iter_getval((x), struct token).type

/* Operator priority */
static int
op_rank(int op)
{
    switch (op) {
    case TOKEN_OP_ADD:
    case TOKEN_OP_SUB:
        return 1;
    case TOKEN_OP_MUL:
    case TOKEN_OP_DIV:
        return 2;
    case TOKEN_IDENTIFIER:
    case TOKEN_NUMBER_FLOAT:
    case TOKEN_NUMBER_INT:
        return INT_MAX;
    /* Not start yet */
    case 0:
        return -1;
    /* Other operators */
    default:
        return INT_MAX - 2;
    }
}

static bool
op_isassign(int op)
{
    switch (op) {
    case TOKEN_OP_ASSIGN:
    case TOKEN_OP_ADDE:
    case TOKEN_OP_SUBE:
    case TOKEN_OP_MULE:
    case TOKEN_OP_DIVE:
        return true;
    default:
        return false;
    }
}

/*
 * Recursive parser of arithmetic expressions.
 * For example, when parsing `a=1+2`, the root is '=',
 * left child is 'a'. And right child is '+', of which left
 * child is '1', right child is '2'.
 */
mao_expr
mao_parse_expr(qmem_iter_t start_pos, qmem_iter_t end_pos)
{
    mao_expr           res = NULL;      /* result */
    int              paren = 0;         /* count of parentheses */
    int              psign = 0;         /* previous token type */
    qmem_iter_t    middle;              /* root token of expr */
    qmem_iter_t       tmp;              /* test for once_out_of_paren */
    qmem_iter_t          i = start_pos; /* loop variable */
    int          lowest_op = INT_MAX;   /* the lowest priority */
    bool once_out_of_paren = false;     /* whether blocked by parentheses */
    bool         obj_found = false;     /* for error handling */
    qmem_t        tmp_save = NULL;      /* tmp for saving copy of token stream add 0 */
    struct token   emu_tmp = (struct token) {
        .type=TOKEN_NUMBER_INT, .line=0, .ival = 0
    };
    
    /* Expression starts with '+' or '-', we fill a zero at the start */
    if (op_rank(TOK_CURTYPE(start_pos)) == 1) {
        tmp_save = qmem_create(struct token);
        qmem_append(tmp_save, emu_tmp, struct token);
        for (i = start_pos; !qmem_iter_eq(i, end_pos); qmem_iter_forward(&i)) {
            qmem_append(tmp_save, qmem_iter_getval(i, struct token), struct token);
        }
        start_pos = qmem_iter_new(tmp_save);
        for (end_pos = start_pos; !qmem_iter_end(end_pos); qmem_iter_forward(&end_pos));
    } else if (op_rank(TOK_CURTYPE(start_pos)) == 2) {
        add_err_queue("line %u: Unexpected '%c' at beginning of sub-expression.\n",
                      TOK_CURTOK(start_pos).line, TOK_CURTYPE(start_pos) == TOKEN_OP_MUL ? '*' : '/');
        exit(1);
    }
    
    /*
     * An expression may be blocked by parentheses, like (1+1)
     * variable `once_out_of_paren` is flag of whether an expression
     * is like this.
     */
    if (TOK_CURTYPE(i) != TOKEN_LPAREN) {
        once_out_of_paren = true;
    }
    
    for (i = start_pos; !qmem_iter_eq(i, end_pos); qmem_iter_forward(&i)) {
        /*
         * There's a problem about operator associativity.
         *
         * + - * / % is left-associative, that means, when we do
         * 1 - 2 - 3, the result is -4, not 2. The expreesion is
         * equivalent to (1 - 2) - 3, not 1 - (2 - 3).
         *
         * But = is right-associative, when we do the expression
         * a = b = 12 , the true order is:
         * a = (b = 12).
         *
         * So, when reaching a '=' out of parentheses, stop.
         */
        if (op_rank(TOK_CURTYPE(i)) == INT_MAX) {
            if (op_rank(psign) == INT_MAX) {
                add_err_queue("line %u: Expected operator after identifier or number.\n",
                              TOK_CURTOK(i).line);
                exit(1);
            }
            obj_found = true;
            
        } else if (op_isassign(TOK_CURTYPE(i)) && paren == 0) {
            middle = i;
            lowest_op = 3;
            break;
        } else if (TOK_CURTYPE(i) == TOKEN_LPAREN) {
            ++paren;
        } else if (TOK_CURTYPE(i) == TOKEN_RPAREN) {
            --paren;
        } else if (op_rank(TOK_CURTYPE(i)) <= lowest_op && paren == 0) {
            /* Previous token is not an operator */
            if (op_rank(TOK_CURTYPE(i)) > 2 || op_rank(psign) > 2) {
                lowest_op = op_rank(TOK_CURTYPE(i));
                middle = i;
            }
        }
        
        tmp = i;
        qmem_iter_forward(&tmp);
        if (paren == 0 && !qmem_iter_eq(tmp, end_pos)) {
            once_out_of_paren = true;
        }
        psign = TOK_CURTYPE(i);
    }
    
    /* Unmatching parentheses */
    if (paren != 0) {
        add_err_queue("line %u: Unmatching parentheses.\n", TOK_CURTOK(start_pos).line);
        exit(1);
    }
    
    /* The whole expression is an operator */
    if (!obj_found) {
        add_err_queue("line %u: Too many operators.\n", TOK_CURTOK(start_pos).line);
        exit(1);
    }
    
    if (once_out_of_paren) {
        res = qalloc(sizeof(struct mao_expr_struct));
        global_memory_register(res);
        
        /* The whole expr is only an identifier or number */
        if (lowest_op == INT_MAX) {
            res->left_child = res->right_child = NULL;
            switch (TOK_CURTOK(start_pos).type) {
            case TOKEN_IDENTIFIER:
                if ((res->val = mao_get_variable_obj(TOK_CURTOK(start_pos).name)) == NULL) {
                    add_err_queue("line %u: Variable '", TOK_CURTOK(start_pos).line);
                    qstr_print(TOK_CURTOK(start_pos).name, stderr);
                    fputs("' is undefined.\n", stderr);
                    exit(1);
                }
                break;
            case TOKEN_NUMBER_INT:
                res->val = mao_obj_new(OBJ_INIT_INT, TOK_CURTOK(start_pos).ival);
                break;
            case TOKEN_NUMBER_FLOAT:
                res->val = mao_obj_new(OBJ_INIT_DOUBLE, TOK_CURTOK(start_pos).dval);
                break;
            default:
                res->val = NULL;
                break;
            }
        } else {
            res->left_child = mao_parse_expr(start_pos, middle);
            res->op = TOK_CURTYPE(middle);
            qmem_iter_forward(&middle);
            res->right_child = mao_parse_expr(middle, end_pos);
        }
    } else {
        qmem_iter_backward(&end_pos);
        qmem_iter_forward(&start_pos);
        return mao_parse_expr(start_pos, end_pos);
    }
    if (tmp_save != NULL) {
        qmem_free(tmp_save);
    }
    return res;
}

