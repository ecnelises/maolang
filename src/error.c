/*
 * error.c
 * Qiu Chaofan, 2015/12/21
 *
 * This file defines the `qalloc` function, which added
 * error handling code to `malloc`.
 */

#include <stdlib.h>
#include "error.h"

int _mao_global_errnum = 0;

void *qalloc(size_t dst_size)
{
    void *res = malloc(dst_size);
    if (res == NULL) {
        fprintf(stderr, "malloc failed: out of memory.\n");
        exit(1);
    }
    return res;
}
