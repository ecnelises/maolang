/*
 * error.h
 * Qiu Chaofan, 2015/12/21
 *
 * Macros and numbers for error processing.
 */

#ifndef MAOLANG_ERROR_H_
#define MAOLANG_ERROR_H_

#include <stdio.h>
#include <stdlib.h>

extern int _mao_global_errnum;

void *qalloc(size_t dst_size);

#define add_err_queue(...) \
    do { \
        ++_mao_global_errnum; \
        /*fprintf(stderr, "Error #%d: ", ++_mao_global_errnum);*/ \
        fprintf(stderr, __VA_ARGS__); \
    } while(0) \

#endif      //MAOLANG_ERROR_H_
