/*
 * lex.h
 * Qiu Chaofan, 2015/12/21
 *
 * Definition of token type macros and the token struct.
 */

#ifndef MAOLANG_LEX_H_
#define MAOLANG_LEX_H_

#include "infra/qstring.h"

#define TOKEN_TYPE_INT      0x001
#define TOKEN_TYPE_DOUBLE   0x002

#define TOKEN_FUNC_PRINT    0x041

#define TOKEN_LITERAL       0x031

#define TOKEN_IDENTIFIER    0x051

#define TOKEN_TRUE          0x062
#define TOKEN_FALSE         0x063

#define TOKEN_OP_ADD        0x071
#define TOKEN_OP_SUB        0x072
#define TOKEN_OP_MUL        0x073
#define TOKEN_OP_DIV        0x074
#define TOKEN_OP_ASSIGN     0x077
#define TOKEN_OP_ADDE       0x078
#define TOKEN_OP_SUBE       0x079
#define TOKEN_OP_MULE       0x07A
#define TOKEN_OP_DIVE       0x07B

#define TOKEN_OP_EQUAL      0x081

#define TOKEN_NUMBER_INT    0x0A1
#define TOKEN_NUMBER_FLOAT  0x0A2

#define TOKEN_UNKNOWN       0x0B1
#define TOKEN_END           0x0BF

#define TOKEN_LPAREN        0x0C1
#define TOKEN_RPAREN        0x0C2
#define TOKEN_COMMA         0x0C7
#define TOKEN_SEMICOLON     0x0CA

qmem_t mao_lex_analyze(FILE *fp);

struct token {
    int type;
    unsigned line;
    union {
        qstr_t name;
        int    ival;
        double dval;
    };
};

int mao_parse(qmem_t stream, FILE *fp);

#endif      //MAOLANG_LEX_H_
