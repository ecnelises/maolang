/*
 * error.h
 * Qiu Chaofan, 2015/12/21
 *
 * Macros for error processing.
 */

#ifndef MAOLANG_ERROR_H_
#define MAOLANG_ERROR_H_

#include <stdio.h>
#include <stdlib.h>

extern int _mao_global_errnum;

#define handle_error(ec) \
    do { \
        fprintf(stderr, "Fatal Error: %s in file %s, line %d and function %s.", #ec, __FILE__, __LINE__, __func__); \
        abort(); \
    } while(0)

#define add_err_queue(content, ...) \
    do { \
        fprintf(stderr, "Error #%d: %s\n", ++_mao_global_errnum, content); \
    } while(0) \

#endif      //MAOLANG_ERROR_H_
