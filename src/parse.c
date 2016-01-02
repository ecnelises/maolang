/*
 * parse.c
 * Qiu Chaofan, 2016/1/1
 *
 * Main parsing function for Mao statements.
 */

#include "infra/qmemory.h"
#include "runtime.h"
#include "expr.h"
#include "lex.h"

#define CURTOK(x) (qmem_iter_getval(x, struct token))

static int parse_declaration(qmem_iter_t *stream_pos);
static int parse_expression(qmem_iter_t *stream_pos);
static int parse_function(qmem_iter_t *stream_pos, FILE *fp);

int
mao_parse(qmem_t stream, FILE *fp)
{
    int status = 0;
    for (qmem_iter_t stream_pos = qmem_iter_new(stream);
         !qmem_iter_end(stream_pos); qmem_iter_forward(&stream_pos)) {
        switch (CURTOK(stream_pos).type) {
        case TOKEN_TYPE_INT:
        case TOKEN_TYPE_DOUBLE:
            status += parse_declaration(&stream_pos);
            break;
        case TOKEN_IDENTIFIER:
        case TOKEN_LPAREN:
        case TOKEN_OP_ADD:
        case TOKEN_OP_SUB:
        case TOKEN_NUMBER_INT:
        case TOKEN_NUMBER_FLOAT:
            status += parse_expression(&stream_pos);
            break;
        case TOKEN_FUNC_PRINT:
            status += parse_function(&stream_pos, fp);
            break;
        default:
            break;
        }
    }
    return status;
}

static int
parse_declaration(qmem_iter_t *stream_pos)
{
    int status = 0;
    int type = CURTOK(*stream_pos).type == TOKEN_TYPE_INT ?
        MAO_OBJ_INT : MAO_OBJ_DOUBLE;
    qmem_iter_forward(stream_pos);

    while (!qmem_iter_end(*stream_pos)) {
        if (CURTOK(*stream_pos).type != TOKEN_IDENTIFIER) {
            add_err_queue("line %u: Expected identifier after typeword '%s'.\n",
                    CURTOK(*stream_pos).line, type == MAO_OBJ_INT ? "int" : "double");
            exit(status = 1);
        }
        mao_register_variable(type, CURTOK(*stream_pos).name);
        qmem_iter_forward(stream_pos);
        if (!qmem_iter_end(*stream_pos)) {
            if (CURTOK(*stream_pos).type == TOKEN_COMMA) {
                qmem_iter_forward(stream_pos);
            } else if (CURTOK(*stream_pos).type == TOKEN_SEMICOLON) {
                break;
            } else {
                add_err_queue("line %u: Expected ',' or ';' after identifier.\n",
                        CURTOK(*stream_pos).line);
                exit(status = 1);
            }
        }
    }
    return status;
}

static int
parse_expression(qmem_iter_t *stream_pos)
{
    int  status    = 0;
    bool semicolon = false;
    qmem_iter_t probe = *stream_pos;

    while (!qmem_iter_end(probe)) {
        if (CURTOK(probe).type == TOKEN_SEMICOLON) {
            semicolon = true;
            break;
        }
        qmem_iter_forward(&probe);
    }

    /* No semicolon found */
    if (!semicolon) {
        add_err_queue("end line: Expected ';' at end of a statement.\n");
        exit(status = 1);
    }

    mao_expr_calc(mao_parse_expr(*stream_pos, probe));
    /* 
     * Every time when an expression is parsed, the temporary
     * memory list will be released.
     */
    global_memory_clean();
    *stream_pos = probe;
    return status;
}

static int
parse_function(qmem_iter_t *stream_pos, FILE *fp)
{
    int status = 0;
    qmem_iter_t probe = *stream_pos;
    int parencount = 0;

    /* Find the matching right parenthesis. */
    while (!qmem_iter_end(probe)) {
        if (CURTOK(probe).type == TOKEN_LPAREN) {
            ++parencount;
        } else if (CURTOK(probe).type == TOKEN_RPAREN) {
            if (--parencount == 0) {
                break;
            }
        }
        qmem_iter_forward(&probe);
    }

    switch (CURTOK(*stream_pos).type) {
        case TOKEN_FUNC_PRINT:
            qmem_iter_forward(stream_pos);
            qmem_iter_forward(stream_pos);
            if (qmem_iter_getval(*stream_pos, struct token).type == TOKEN_LITERAL) {
                qstr_print(qmem_iter_getval(*stream_pos, struct token).name, fp);
            } else {
                print_obj(mao_expr_calc(mao_parse_expr(*stream_pos, probe)), fp);
                *stream_pos = probe;
            }
            qmem_iter_forward(stream_pos);
            break;
        default:
            break;
    }
    return status;
}
