#include "core/lex.h"
#include "core/parse.h"

int main(void)
{
    qm_t lst = maolang_lex_analyze(stdin);
    mao_parse(lst);
    return 0;
}
