#include "lexer.h"
#include "parser.h"
#include "printer.h"
#include "runtime.h"
#include "util.h"

int main(void)
{
    long fi;

    require64BitLongs();

    runtime_init();
    lexer_init();

    fi = parse();

    print(fi);

    return 0;
}
