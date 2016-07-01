/*
 * qstring.h
 * Qiu Chaofan, 2015/11/25
 * 
 * Dynamic string powered by qmemory. No utf-8 support so far.
 *
 * type: qstr_t, qstr_iter_t
 */

#ifndef MAOLANG_QSTRING_H_
#define MAOLANG_QSTRING_H_

#include <stddef.h>
#include <stdbool.h>
#include "qmemory.h"

#define QSTR_INIT_BYINT     1
#define QSTR_INIT_BYCSTR    2
#define QSTR_INIT_BYCHAR    3
#define QSTR_INIT_BYNONE    4
#define QSTR_ASSIGN_BYINT   QSTR_INIT_BYINT
#define QSTR_ASSIGN_BYCSTR  QSTR_INIT_BYCSTR
#define QSTR_ASSIGN_BYCHAR  QSTR_INIT_BYCHAR

typedef qmemory qstring;
typedef qmemory_iterator qstring_iterator;
typedef qmem_t    qstr_t;
typedef qmem_iter_t qstr_iter_t;

qstr_t          qstr_create(int init_type, ...);
void            qstr_assign(qstr_t item, int assign_type, ...);
qstr_t          qstr_sub(const qstr_t item, size_t start, size_t length);

static inline   qstr_t
qstr_duplicate(const qstr_t item)
{
    return qmem_duplicate(item);
}

/*
 * Append a copy of str to the end of item. 
 */
qstr_t          qstr_append(qstr_t item, qstr_t str);

static inline void
qstr_push(qstr_t item, char ch)
{
    qmem_append(item, ch, char);
}

static inline   size_t
qstr_len(const qstr_t item)
{
    return qmem_len(item);
}

static inline void
qstr_lessen(qstr_t item, size_t dest_len)
{
    qmem_lessen(item, dest_len);
}

static inline   bool
qstr_empty(const qstr_t item)
{
    return qmem_empty(item);
}

static inline void
qstr_clear(qstr_t item)
{
    qmem_clear(item);
}

static inline void
qstr_free(qstr_t item)
{
    qmem_free(item);
}

/*
 * If no pattern is found, these functions return -1 
 */
int             qstr_find_char(const qstr_t text, const char pattern);
int             qstr_find_cstr(const qstr_t text, const char *pattern);
int             qstr_find_qstr(const qstr_t text, const qstr_t pattern);

/*
 * The function returns difference of first different position 
 */
int             qstr_comp(const qstr_t str1, const qstr_t str2);
int            qstr_ccomp(const qstr_t str1, const char * str2);

static inline   qstr_iter_t
qstr_iter_new(const qstr_t dest)
{
    return qmem_iter_new(dest);
}

static inline char
qstr_iter_getval(const qstr_iter_t item)
{
    return qmem_iter_getval(item, char);
}

static inline bool
qstr_iter_end(const qstr_iter_t item)
{
    return qmem_iter_end(item);
}

static inline void
qstr_iter_forward(qstr_iter_t * item)
{
    qmem_iter_forward(item);
}

#endif				// MAOLANG_QSTR_H_
