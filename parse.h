/*
 * parse.h
 * Qiu Chaofan, 2015/12/21
 *
 * Header file of parser of the Mao programming language,
 * including definition of analysis tree and functions
 * in parsing.
 */

#ifndef MAOLANG_PARSE_H_
#define MAOLANG_PARSE_H_

#include "infra/qmemory.h"
#include "infra/qtree.h"
#include "lex.h"

/*
 * "GRT" stands for "Grammer Representative Tree", starting from
 * 0x100. All macros with prefix "GRT_" are all nonterminals.
 */
#define GRT_NULL            0x100
#define GRT_DECLARATION     0x101
#define GRT_FOR             0x102
#define GRT_FOREACH         0x103
#define GRT_WHILE           0x104
#define GRT_DOWHILE         0x105
#define GRT_IF              0x106
#define GRT_BLOCK           0x107
#define GRT_COMMAND         0x108
#define GRT_EXPRESSION      0x109
#define GRT_EMPTY           0x10F

#define GRT_TYPEWORD        0x111
#define GRT_DCLLIST         0x112

#define GRT_RVAL            0x121

/*
 * The following macros defines type of expression nodes.
 * To parse by the rule of operator priority, I defined several
 * nonterminals in the Context-Free Grammar. Each comment besides
 * macro represents the structure of such kind of node.
 *
 * For example, when we parse expression "a = a + 1":
 * root:     GRT_EXPR_ASSIGN
 * child[0]: GRT_EXPR_ARRV
 *    child[0-0]: TOKEN_IDENTIFIER "a"
 * child[1]: TOKEN_OP_ASSIGN
 * child[2]: GRT_EXPR_TERM
 *   child[2-0]: GRT_EXPR_ARRV
 *     child[2-0-0]: TOKEN_IDENTIFIER "a"
 *   child[2-1]: TOKEN_OP_ADD
 *   child[2-2]: GRT_EXPR_ARRV
 *     child[2-2-0]: TOKEN_RVAL "1"
 *
 * More details about the kinds of expression can be found in
 * expr-parse functions' source code.
 */
#define GRT_EXPR_ASSIGN     0x141  /* a = b       */
#define GRT_EXPR_ORRES      0x142  /* a or b      */
#define GRT_EXPR_ANDRES     0x143  /* a and b     */
#define GRT_EXPR_ECOMP      0x144  /* a eqop b    */
#define GRT_EXPR_NECOMP     0x145  /* a neqop b   */
#define GRT_EXPR_TERM       0x146  /* a op-1 b    */
#define GRT_EXPR_FACTOR     0x147  /* a op-2 b    */
#define GRT_EXPR_POWER      0x148  /* a op-3 b    */
#define GRT_EXPR_SINGLE     0x149  /* `not` or `-` or `+` a */
#define GRT_EXPR_ARRV       0x14A  /* `(` a `)` or a`[b]`   */

/* grt_info has the same member as token. */
typedef struct token grt_info;

/*
 * Main function for parsing tokens into analysis tree.
 * Param stream is a list of tokens to be parsed;
 * The return value is list of trees, in which each member is
 * a statement tree.
 */
qmem_t mao_parse(qmem_t stream);

#endif      //MAOLANG_PARSE_H_
