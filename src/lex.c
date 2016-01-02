/*
 * lex.c
 * Qiu Chaofan, 2015/12/21
 *
 * Lexical scanner of the Mao language.
 */

#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include "infra/qmemory.h"
#include "infra/qstring.h"
#include "error.h"
#include "lex.h"

static int line_count = 1;
static const char * ops = "+-*=";   /* operators */
static const char * pcs = "(),;";   /* punctuations */

static struct token lex_identifier (FILE *fp, char ch);
static void         lex_comment    (FILE *fp, char ch, bool singlelined);
static struct token lex_string     (FILE *fp, char ch);
static struct token lex_operator   (FILE *fp, char ch);
static struct token lex_number     (FILE *fp, char ch);
static struct token lex_punctuation(FILE *fp, char ch);
static void         lex_unknown    (char ch);
static char         escape         (char ch);

/*
 * Main function of scanner.
 * It chooses the right function considering the first character of
 * each token, using `ungetc` for backtracking.
 */
qmem_t
mao_lex_analyze(FILE *fp)
{
    char tmp, ch;
    qmem_t res = qmem_create(struct token);
    
    while ((ch = fgetc(fp)) != EOF) {
        
        if (isalpha(ch) || ch == '_') {
            qmem_append(res, lex_identifier(fp, ch), struct token);
            
        } else if (ch == '/') {
            tmp = fgetc(fp);
            if (tmp == '*') {
                lex_comment(fp, fgetc(fp), false);
            } else if (tmp == '/') {
                lex_comment(fp, fgetc(fp), true);
            } else {
                ungetc(tmp, fp);
                qmem_append(res, lex_operator(fp, ch), struct token);
            }
            
        } else if (strchr(ops, ch)) {
            qmem_append(res, lex_operator(fp, ch), struct token);
            
        } else if (ch == '\"') {
            qmem_append(res, lex_string(fp, fgetc(fp)), struct token);
            
        } else if (strchr(pcs, ch)) {
            qmem_append(res, lex_punctuation(fp, ch), struct token);
            
        } else if (ch == '.' || isdigit(ch)) {
            qmem_append(res, lex_number(fp, ch), struct token);
            
        } else if (ch == '\n') {
            ++line_count;

        } else if (!isspace(ch)) {
            lex_unknown(ch);
        }
    }
    /* End flag */
    struct token tmp_end;
    tmp_end.type = TOKEN_END;
    qmem_append(res, tmp_end, struct token);
    return res;
}

/*
 * Saving identifiers and judge whether it is a keyword.
 */
static struct token
lex_identifier(FILE *fp, char ch)
{
    qstr_t  currentname;
    int     currenttype;
    bool    iskeyword;
    
    currentname = qstr_create(QSTR_INIT_BYNONE);
    while (ch != EOF && (isalnum(ch) || ch == '_')) {
        qstr_push(currentname, ch);
        ch = fgetc(fp);
    }
    
    iskeyword = true;
    
    if (!qstr_ccomp(currentname, "int")) {
        currenttype = TOKEN_TYPE_INT;
    } else if (!qstr_ccomp(currentname, "double")) {
        currenttype = TOKEN_TYPE_DOUBLE;
    } else if (!qstr_ccomp(currentname, "print")) {
        currenttype = TOKEN_FUNC_PRINT;
    } else {
        iskeyword = false;
        currenttype = TOKEN_IDENTIFIER;
    }
    
    if (iskeyword) {
        qstr_free(currentname);
        currentname = NULL;
    }
    ungetc(ch, fp);
    
    return (struct token) {
        currenttype, line_count, .name = currentname
    };
}

/* Mao supports both C and C++ style comments. */
static void
lex_comment(FILE *fp, char ch, bool singlelined)
{
    bool comment_end = false;
    if (singlelined) {
        do {
            if (ch == '\n') {
                break;
            }
        } while ((ch = fgetc(fp)) != EOF);
        
    } else {
        char tmp;
        do {
            if (ch == '*') {
                tmp = fgetc(fp);
                if (tmp == '/') {
                    comment_end = true;
                    break;
                } else {
                    ungetc(tmp, fp);
                }
            }
        } while ((ch = fgetc(fp)) != EOF);
    }
    
    /* When the multi-line comment doesn't end validly */
    if (!comment_end) {
        add_err_queue("line %u: Multi-line comment doesn't have an end.\n", line_count);
    }
}

static char
escape(char ch)
{
    switch (ch) {
    case '\"':
        return '\"';
    case '\'':
        return '\'';
    case '\\':
        return '\\';
    case '/':
        return '/';
    case 'n':
        return '\n';
    case 'b':
        return '\b';
    case 't':
        return '\t';
    case 'r':
        return '\r';
    case 'f':
        return '\f';
    default:
        return '\\';
    }
}

/* Parse string literal between " in source code. */
static struct token
lex_string(FILE *fp, char ch)
{
    qstr_t  currentname = qstr_create(QSTR_INIT_BYNONE);
    bool string_end = false;
    
    do {
        if (ch == '\"') {
            string_end = true;
            break;
        } else if (ch == '\\') {
            qstr_push(currentname, escape(fgetc(fp)));
        } else {
            qstr_push(currentname, ch);
        }
    } while ((ch = fgetc(fp)) != EOF && ch != '\n');
    
    if (!string_end) {
        add_err_queue("line %u: Multirow string literal is not valid.\n", line_count);
    }
    
    return (struct token) {
        TOKEN_LITERAL, line_count, .name = currentname
    };
}

static struct token
lex_operator(FILE *fp, char ch)
{
    char tmp         = fgetc(fp);
    int  currenttype = TOKEN_UNKNOWN;
    
    switch (ch) {
        case '+':
            if (tmp == '=') { currenttype = TOKEN_OP_ADDE;      /* += */   }
            else            { currenttype = TOKEN_OP_ADD; ungetc(tmp, fp); }
            break;
        case '-':
            if (tmp == '=') { currenttype = TOKEN_OP_SUBE;      /* -= */   }
            else            { currenttype = TOKEN_OP_SUB; ungetc(tmp, fp); }
            break;
        case '*':
            if (tmp == '=') { currenttype = TOKEN_OP_MULE;      /* *= */   }
            else            { currenttype = TOKEN_OP_MUL; ungetc(tmp, fp); }
            break;
        case '/':
            if (tmp == '=') { currenttype = TOKEN_OP_DIVE;      /* /= */   }
            else            { currenttype = TOKEN_OP_DIV; ungetc(tmp, fp); }
            break;
        case '=':
            if (tmp == '=') { currenttype = TOKEN_OP_EQUAL;     /* == */      }
            else            { currenttype = TOKEN_OP_ASSIGN; ungetc(tmp, fp); }
            break;
    }
    
    return (struct token) {
        currenttype, line_count, .name = NULL
    };
}

/*
 * Scan the number of integer or float.
 *
 * For floating numbers, it starts with a digit or point, with an optional
 * E/e and the exponent. The exponent can start with + or - sign.
 *
 * For integer numbers, no 0x/0X or 0b/0B was considered yet.
 *
 * Pay attention that the +/- sign at the start of each number will be scanned
 * as a single operator. That will be parsed in the parsing process.
 */
static struct token
lex_number(FILE *fp, char ch)
{
    static char buf[31];
    bool   isfloat = false;
    bool    hasexp = false;
    size_t   index = 0;
    
    while (ch != EOF && isdigit(ch)) {
        if (index <= 30) {
            buf[index++] = ch;
        }
        ch = fgetc(fp);
    }
    
    if (ch == '.') {
        isfloat = true;
        do {
            if (index <= 30) {
                buf[index++] = ch;
            }
            ch = fgetc(fp);
        } while (ch != EOF && isdigit(ch));
    }
    
    if (ch == 'E' || ch == 'e') {
        isfloat = hasexp = true;
        if (index <= 30) {
            buf[index++] = ch;
        }
        ch = fgetc(fp);
        if (ch == '+' || ch == '-') {
            if (index <= 30) {
                buf[index++] = ch;
            }
            ch = fgetc(fp);
        }
    }
    
    if (hasexp) {
        while (ch != EOF && isdigit(ch)) {
            if (index <= 30) {
                buf[index++] = ch;
            }
            ch = fgetc(fp);
        }
    }
    if (index >= 30) {
        buf[30] = '\0';
        add_err_queue("line %u: Lengthy number will be cut as '%s' and result is undefined.\n", line_count,buf);
    } else {
        buf[index] = '\0';
    }
    ungetc(ch, fp);
    
    if (isfloat) {
        return (struct token) {
            TOKEN_NUMBER_FLOAT, line_count, .dval = atof(buf)
        };
    } else {
        return (struct token) {
            TOKEN_NUMBER_INT, line_count, .ival = atoi(buf)
        };
    }
}

static struct token
lex_punctuation(FILE *fp, char ch)
{
    int currenttype = TOKEN_UNKNOWN;
    switch (ch) {
        case '(':
            currenttype = TOKEN_LPAREN;
            break;
        case ')':
            currenttype = TOKEN_RPAREN;
            break;
        case ',':
            currenttype = TOKEN_COMMA;
            break;
        case ';':
            currenttype = TOKEN_SEMICOLON;
            break;
        default:
            break;
    }
    return (struct token) {
        currenttype, line_count, .name = NULL
    };
}

static void
lex_unknown(char ch)
{
    add_err_queue("line %u: Unknown character '%c'.\n", line_count, ch);
}
