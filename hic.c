#include "compiler.h"
#include "parser.h"
#include "printer.h"
#include "runtime.h"
#include "util.h"

int main(void)
{
    long hi;
    long fi;

    require64BitLongs();

    runtime_init();
    compiler_init();

    hi = parse();
    fi = compile(hi);

    print(fi);

    return 0;
}
