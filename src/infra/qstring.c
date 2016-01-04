/*
 * qstring.c Qiu Chaofan, 2015/11/25
 */

#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include "error.h"
#include "qstring.h"

static void
va_qstr_assign(qstr_t item, int assign_type, va_list ap)
{
    assert(item != NULL);
    if (item->unwritten < item->blklen) {
        qmem_clear(item);
    }
    union {
        int   ival;
        char *sval;
        char  cval;
    }   value;
    
    int       itoa_tmp = 1;
    switch (assign_type) {
        case QSTR_INIT_BYINT:
            value.ival = va_arg(ap, int);
            if (value.ival == 0) {
                qstr_push(item, '0');
            } else {
                if (value.ival < 0) {
                    qstr_push(item, '-');
                    value.ival = -value.ival;
                }
                while (itoa_tmp < value.ival) {
                    itoa_tmp *= 10;
                }
                if (itoa_tmp != value.ival) {
                    itoa_tmp /= 10;
                }
                while (itoa_tmp > 0) {
                    qstr_push(item, (value.ival / itoa_tmp) + '0');
                    value.ival %= itoa_tmp;
                    itoa_tmp /= 10;
                }
            }
            break;
        case QSTR_INIT_BYCSTR:
            value.sval = va_arg(ap, char *);
            for (size_t i = 0; i < strlen(value.sval); ++i) {
                qstr_push(item, (value.sval)[i]);
                /*
                 * Maybe 'strcpy' works better?
                 */
            }
            break;
        case QSTR_INIT_BYCHAR:
            /*
             * C will see character literal as int type.
             */
            value.cval = va_arg(ap, int);
            qstr_push(item, value.cval);
            break;
        default:
            break;
    }
}

qstr_t
qstr_create(int init_type,...)
{
    qstr_t        res = qmem_create(char);
    
    va_list        ap;
    va_start(ap, init_type);
    va_qstr_assign(res, init_type, ap);
    va_end(ap);
    
    return res;
}

void
qstr_assign(qstr_t item, int assign_type,...)
{
    va_list        ap;
    va_start(ap, assign_type);
    va_qstr_assign(item, assign_type, ap);
    va_end(ap);
}

qstr_t
qstr_sub(const qstr_t item, size_t start, size_t length)
{
    assert(item != NULL);
    qstr_t        res = qstr_create(QSTR_INIT_BYNONE);
    size_t        i = 0;
    qstr_iter_t   iter = qstr_iter_new(item);
    
    if (length != 0) {
        if (start != 0) {
            while (!qstr_iter_end(iter)) {
                qstr_iter_forward(&iter);
                if (++i == start) {
                    break;
                }
            }
        }
        i = 0;
        do {
            qstr_push(res, qstr_iter_getval(iter));
            qstr_iter_forward(&iter);
        } while (++i < length && !qstr_iter_end(iter));
    }
    return res;
}

qstr_t
qstr_append(qstr_t item, qstr_t str)
{
    assert(item != NULL);
    assert(str != NULL);
    if (qstr_empty(str)) {
        return NULL;
    }
    qstr_iter_t    striter = qstr_iter_new(str);
    
    do {
        qstr_push(item, qstr_iter_getval(striter));
        qstr_iter_forward(&striter);
    } while (!qstr_iter_end(striter));
    
    return item;
}

int
qstr_find_char(const qstr_t item, const char pattern)
{
    assert(item != NULL);
    int        i = 0;
    for (qstr_iter_t iter = qstr_iter_new(item);
         !qstr_iter_end(iter); qstr_iter_forward(&iter), ++i) {
        if (qstr_iter_getval(iter) == pattern) {
            return i;
        }
    }
    
    return -1;
}

/*
 * The easiest way in searching substr is to enumerate each character
 * in item and pattern of the complexity O(MN), where M and N stands
 * for the length of item and pattern.
 *
 * A better algorithm is Knuth-Morris-Pratt(KMP) as well as
 * Boyer-Moore(BM). But I cannot implement those algorithms for now
 * :( I checked how it was implemented in BSD libc - function strstr.
 * And I found it that they code it in the easiest way!
 *
 * So, considering that the item won't be too long, I use the first
 * method until I fully understand BM someday.
 */
int
qstr_find_cstr(const qstr_t text, const char *pattern)
{
    assert(text != NULL);
    assert(pattern != NULL);
    if (strlen(pattern) > qstr_len(text)) {
        return -1;
    }
    int        i = 0;
    for (qstr_iter_t match_tmp, iter = qstr_iter_new(text);
         !qstr_iter_end(iter); qstr_iter_forward(&iter), ++i) {
        match_tmp = iter;
        for (int j = 0; j < strlen(pattern); ++j) {
            if (qstr_iter_getval(match_tmp) != pattern[j]
                || qstr_iter_end(match_tmp)) {
                break;
            }
            qstr_iter_forward(&match_tmp);
            if (j == strlen(pattern) - 1) {
                return i;
            }
        }
    }
    
    return -1;
}

int
qstr_find_qstr(const qstr_t text, const qstr_t pattern)
{
    assert(text != NULL);
    assert(pattern != NULL);
    if (qstr_len(pattern) > qstr_len(text)) {
        return -1;
    }
    int  i = 0,  j;
    for (qstr_iter_t pattern_match, match_tmp, iter = qstr_iter_new(text);
         !qstr_iter_end(iter); qstr_iter_forward(&iter), ++i) {
        match_tmp = iter;
        for (pattern_match = qstr_iter_new(pattern), j = 0;
             !qstr_iter_end(pattern_match);
             qstr_iter_forward(&pattern_match),
             qstr_iter_forward(&match_tmp), ++j) {
            if (qstr_iter_getval(match_tmp) !=
                qstr_iter_getval(pattern_match)
                || qstr_iter_end(match_tmp)) {
                break;
            }
            if (j == qstr_len(pattern) - 1) {
                return i;
            }
        }
        /*
         * Ugly code.
         */
    }
    
    return -1;
}

int
qstr_comp(const qstr_t str1, const qstr_t str2)
{
    qstr_iter_t    str1iter = qstr_iter_new(str1);
    qstr_iter_t    str2iter = qstr_iter_new(str2);
    char           c1 = '\0', c2 = '\0';
    
    for (; !qstr_iter_end(str1iter) && !qstr_iter_end(str2iter);
         qstr_iter_forward(&str1iter), qstr_iter_forward(&str2iter)) {
        c1 = qstr_iter_getval(str1iter), c2 = qstr_iter_getval(str2iter);
        if (c1 != c2) {
            return c1 - c2;
        }
    }
    
    if (qstr_len(str1) != qstr_len(str2)) {
        if (qstr_iter_end(str1iter)) {
            return 0 - c2;
        } else {
            return c1;
        }
    }
    return 0;
}

int
qstr_ccomp(const qstr_t str1, const char *str2)
{
    qstr_iter_t striter = qstr_iter_new(str1);
    size_t      j;
    char        c = '\0';
    
    for (j = 0, striter = qstr_iter_new(str1); j < strlen(str2) && !qstr_iter_end(striter);
         qstr_iter_forward(&striter), ++j) {
        
        c = qstr_iter_getval(striter);
        if (c != str2[j]) {
            return c - str2[j];
        }
    }
    
    if (qstr_len(str1) != strlen(str2)) {
        
        if (qstr_iter_end(striter)) {
            return 0 - str2[j];
        } else {
            return c;
        }
    }
    
    return 0;
}

void
qstr_print(const qstr_t item, FILE *fp)
{
    assert(item != NULL);
    assert(fp != NULL);
    for (qstr_iter_t i = qstr_iter_new(item);
         !qstr_iter_end(i) && qstr_iter_getval(i) != '\0'; qstr_iter_forward(&i)) {
        fputc(qstr_iter_getval(i), fp);
    }
}
