/*
 * main.c
 * Qiu Chaofan, 2016/1/1
 *
 * Main function of Mao.
 */

#include <stdio.h>
#include <stdlib.h>
#include "infra/qmemory.h"
#include "lex.h"
#include "runtime.h"
#include "expr.h"

qmem_t global_memory_list;
qmap_t variable_list;

int main(int argc, const char * argv[])
{
    global_memory_list = qmem_create(void*);
    variable_list      = qmap_create(mvar);
    FILE *out_fp       = stdout;
    FILE *fp;

    if (argc == 1) {
        fp = stdin;
    } else {
        if ((fp = fopen(argv[1], "r")) == NULL) {
            perror(argv[1]);
            exit(1);
        }
    }

    qmem_t res = mao_lex_analyze(fp);
    mao_parse(res, out_fp);

    if (argc != 1) {
        fclose(fp);
    }

    return 0;
}
