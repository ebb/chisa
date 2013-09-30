#include <stdio.h>
#include <stdlib.h>

void die(const char *e)
{
    fprintf(stderr, "Error: %s\n", e);
    exit(1);
}

void require64BitLongs(void)
{
    if (sizeof(long) != 8)
        die("The C long type is not 64 bits wide.");
}
