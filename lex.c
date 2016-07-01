/*
 * lex.c
 * Qiu Chaofan, 2015/12/21
 *
 * Lexical scanner of the Mao programming language.
 */

#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include "infra/qmemory.h"
#include "infra/qstring.h"
#include "syst/error.h"
#include "lex.h"

static const char * ops = "+-*^!%=<>";  /* operators */
static const char * pcs = "()[]{},;";   /* punctuations */

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
maolang_lex_analyze(FILE *fp)
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

        } else if (!isspace(ch)) {
          lex_unknown(ch);

        }
    }
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
    }
    /*
     * I am not sure whether the judge will use any of the keywords below.
     * So if we define the FPJ macro, these will be masked.
     */
#ifndef FPJ
    else if (!qstr_ccomp(currentname, "struct"))   { currenttype = TOKEN_TYPE_STRUCT; }
    else if (!qstr_ccomp(currentname, "boolean"))  { currenttype = TOKEN_TYPE_BOOL;   }
    else if (!qstr_ccomp(currentname, "string"))   { currenttype = TOKEN_TYPE_STRING; }
    else if (!qstr_ccomp(currentname, "void"))     { currenttype = TOKEN_TYPE_VOID;   }
    else if (!qstr_ccomp(currentname, "list"))     { currenttype = TOKEN_TYPE_LIST;   }
    else if (!qstr_ccomp(currentname, "map"))      { currenttype = TOKEN_TYPE_MAP;    }

    else if (!qstr_ccomp(currentname, "byref"))    { currenttype = TOKEN_BYREF;       }
    else if (!qstr_ccomp(currentname, "byval"))    { currenttype = TOKEN_BYVAL;       }
    else if (!qstr_ccomp(currentname, "autocast")) { currenttype = TOKEN_AUTOCAST;    }
    else if (!qstr_ccomp(currentname, "return"))   { currenttype = TOKEN_RETURN;      }

    else if (!qstr_ccomp(currentname, "use"))      { currenttype = TOKEN_USE;         }
    else if (!qstr_ccomp(currentname, "unuse"))    { currenttype = TOKEN_UNUSE;       }
    else if (!qstr_ccomp(currentname, "mod"))      { currenttype = TOKEN_OP_MOD;      }
    else if (!qstr_ccomp(currentname, "true"))     { currenttype = TOKEN_TRUE;        }
    else if (!qstr_ccomp(currentname, "false"))    { currenttype = TOKEN_FALSE;       }

    else if (!qstr_ccomp(currentname, "if"))       { currenttype = TOKEN_IF;          }
    else if (!qstr_ccomp(currentname, "else"))     { currenttype = TOKEN_ELSE;        }
    else if (!qstr_ccomp(currentname, "while"))    { currenttype = TOKEN_WHILE;       }
    else if (!qstr_ccomp(currentname, "do"))       { currenttype = TOKEN_DO;          }
    else if (!qstr_ccomp(currentname, "for"))      { currenttype = TOKEN_FOR;         }
    else if (!qstr_ccomp(currentname, "foreach"))  { currenttype = TOKEN_FOREACH;     }
    else if (!qstr_ccomp(currentname, "switch"))   { currenttype = TOKEN_SWITCH;      }
    else if (!qstr_ccomp(currentname, "case"))     { currenttype = TOKEN_CASE;        }

    else if (!qstr_ccomp(currentname, "and"))      { currenttype = TOKEN_OP_AND;      }
    else if (!qstr_ccomp(currentname, "or"))       { currenttype = TOKEN_OP_OR;       }
    else if (!qstr_ccomp(currentname, "not"))      { currenttype = TOKEN_OP_NOT;      }
    else if (!qstr_ccomp(currentname, "in"))       { currenttype = TOKEN_IN;          }
    else if (!qstr_ccomp(currentname, "rank"))     { currenttype = TOKEN_RANK;        }
    else if (!qstr_ccomp(currentname, "inf")) {
        currenttype = TOKEN_NUMBER_FLOAT;
        iskeyword = false;
    } else if (!qstr_ccomp(currentname, "nan")) {
        currenttype = TOKEN_NUMBER_FLOAT;
        iskeyword = false;
    }
#endif
    else {
        iskeyword = false;
        currenttype = TOKEN_IDENTIFIER;
    }
    if (iskeyword) {
        qstr_free(currentname);
        currentname = NULL;
    }
    ungetc(ch, fp);

    return (struct token) {
        currenttype, currentname
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
        add_err_queue("Multi-line comment doesn't have an end.");
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
        add_err_queue("Multirow string literal is not valid.");
    }
    
    return (struct token) {
        TOKEN_LITERAL, currentname
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
    case '%':
        if (tmp == '=') { currenttype = TOKEN_OP_MODE;      /* %= */   }
        else            { currenttype = TOKEN_OP_MOD; ungetc(tmp, fp); }
        break;
    case '^':
        if (tmp == '=') { currenttype = TOKEN_OP_EXPE;      /* ^= */   }
        else            { currenttype = TOKEN_OP_EXP; ungetc(tmp, fp); }
        break;
    case '=':
        if (tmp == '=') { currenttype = TOKEN_OP_EQUAL;     /* == */      }
        else            { currenttype = TOKEN_OP_ASSIGN; ungetc(tmp, fp); }
        break;
    case '<':
        if (tmp == '=') { currenttype = TOKEN_OP_LESSE;     /* <= */    }
        else            { currenttype = TOKEN_OP_LESS; ungetc(tmp, fp); }
        break;
    case '>':
        if (tmp == '=') { currenttype = TOKEN_OP_GREATERE;  /* >= */       }
        else            { currenttype = TOKEN_OP_GREATER; ungetc(tmp, fp); }
        break;
    case '!':
        if (tmp == '=') { currenttype = TOKEN_OP_NOEQUAL;   /* != */   }
        else            { currenttype = TOKEN_OP_NOT; ungetc(tmp, fp); }
        break;
    }

    return (struct token) {
        currenttype, NULL
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
 * as a single operator. We'll tackle it during the parsing process.
 */
static struct token
lex_number(FILE *fp, char ch)
{
    bool    isfloat     = false;
    qstr_t  currentname = qstr_create(QSTR_INIT_BYNONE);

    while (ch != EOF && isdigit(ch)) {
        qstr_push(currentname, ch);
        ch = fgetc(fp);
    }

    if (ch == '.') {
        isfloat = true;
        do {
            qstr_push(currentname, ch);
            ch = fgetc(fp);
        } while (ch != EOF && isdigit(ch));
    }

    if (ch == 'E' || ch == 'e') {
        isfloat = true;
        qstr_push(currentname, ch);
        ch = fgetc(fp);
        if (ch == '+' || ch == '-') {
            qstr_push(currentname, ch);
            ch = fgetc(fp);
        }
    } else if (isfloat) {
        ungetc(ch, fp);
    }
    
    if (isfloat) {
        while (ch != EOF && isdigit(ch)) {
            qstr_push(currentname, ch);
            ch = fgetc(fp);
        }
    }
    ungetc(ch, fp);
    
    return (struct token) {
        isfloat ? TOKEN_NUMBER_FLOAT : TOKEN_NUMBER_INT, currentname
    };
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
    case '[':
        currenttype = TOKEN_LBRACKET;
        break;
    case ']':
        currenttype = TOKEN_RBRACKET;
        break;
    case '{':
        currenttype = TOKEN_LBRACE;
        break;
    case '}':
        currenttype = TOKEN_RBRACE;
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
        currenttype, NULL
    };
}

static void
lex_unknown(char ch)
{
    add_err_queue("Unknown character.");
}
