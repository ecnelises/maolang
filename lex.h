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
#define TOKEN_TYPE_BOOL     0x003
#define TOKEN_TYPE_STRING   0x004
#define TOKEN_TYPE_VOID     0x005

#define TOKEN_TYPE_LIST     0x011
#define TOKEN_TYPE_MAP      0x012
#define TOKEN_TYPE_STRUCT   0x013

#define TOKEN_BYREF         0x021
#define TOKEN_BYVAL         0x022
#define TOKEN_AUTOCAST      0x023
#define TOKEN_RETURN        0x024

#define TOKEN_IF            0x031
#define TOKEN_ELSE          0x032
#define TOKEN_WHILE         0x033
#define TOKEN_DO            0x034
#define TOKEN_FOR           0x035
#define TOKEN_FOREACH       0x036
#define TOKEN_SWITCH        0x037
#define TOKEN_CASE          0x038

#define TOKEN_USE           0x041
#define TOKEN_UNUSE         0x042

#define TOKEN_IDENTIFIER    0x051

#define TOKEN_LITERAL       0x061
#define TOKEN_TRUE          0x062
#define TOKEN_FALSE         0x063

/*
 * In the parsing process, for cleaner code, I'll
 * use macro for code generation. When judging operator,
 * a supportive array will be used. So the order of such 
 * macros is so important, don't change it.
 */
#define TOKEN_OP_ADD        0x071
#define TOKEN_OP_SUB        0x072
#define TOKEN_OP_MUL        0x073
#define TOKEN_OP_DIV        0x074
#define TOKEN_OP_MOD        0x075
#define TOKEN_OP_EXP        0x076
#define TOKEN_OP_ASSIGN     0x077
#define TOKEN_OP_ADDE       0x078
#define TOKEN_OP_SUBE       0x079
#define TOKEN_OP_MULE       0x07A
#define TOKEN_OP_DIVE       0x07B
#define TOKEN_OP_MODE       0x07C
#define TOKEN_OP_EXPE       0x07D

#define TOKEN_OP_LESS       0x081
#define TOKEN_OP_GREATER    0x082
#define TOKEN_OP_LESSE      0x083
#define TOKEN_OP_GREATERE   0x084
#define TOKEN_OP_EQUAL      0x085
#define TOKEN_OP_NOEQUAL    0x086

#define TOKEN_OP_AND        0x091
#define TOKEN_OP_OR         0x092
#define TOKEN_OP_NOT        0x093
/* end */

#define TOKEN_NUMBER_INT    0x0A1
#define TOKEN_NUMBER_FLOAT  0x0A2

#define TOKEN_UNKNOWN       0x0B1
#define TOKEN_END           0x0BF

#define TOKEN_LPAREN        0x0C1
#define TOKEN_RPAREN        0x0C2
#define TOKEN_LBRACKET      0x0C3
#define TOKEN_RBRACKET      0x0C4
#define TOKEN_LBRACE        0x0C5
#define TOKEN_RBRACE        0x0C6
#define TOKEN_COMMA         0x0C7
#define TOKEN_SEMICOLON     0x0CA
#define TOKEN_IN            0x0CB
#define TOKEN_RANK          0x0CC

qmem_t maolang_lex_analyze(FILE *fp);

struct token {
    int     type;
    qstr_t  name;
};

#endif      //MAOLANG_LEX_H_
