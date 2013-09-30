#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "runtime.h"
#include "util.h"

struct store {
    unsigned long size;
    unsigned long firstFree;
    void *data;
};

static struct store store;

unsigned char runtime_classArities[1 << 16] = {
    [CLASS_Fixnum] = 0,
    [CLASS_String] = 0,
    [CLASS_Nil] = 0,
    [CLASS_Cons] = 2,
    [CLASS_Id] = 1,
    [CLASS_HiDefineVar] = 2,
    [CLASS_HiDefineFunc] = 3,
    [CLASS_HiDefineCons] = 2,
    [CLASS_HiDefineByMatch] = 3,
    [CLASS_HiFunc] = 2,
    [CLASS_HiBegin] = 1,
    [CLASS_HiBlock] = 2,
    [CLASS_HiCall] = 2,
    [CLASS_HiConsApp] = 2,
    [CLASS_HiPrimApp] = 2,
    [CLASS_HiMatch] = 2,
    [CLASS_HiCase] = 3,
    [CLASS_HiElse] = 1,
    [CLASS_FiDefineVar] = 2,
    [CLASS_FiDefineFunc] = 3,
    [CLASS_FiDefineCons] = 2,
    [CLASS_FiBlock] = 4,
    [CLASS_FiStmt] = 2,
    [CLASS_FiCall] = 3,
    [CLASS_FiGoto] = 2,
    [CLASS_FiReturn] = 1,
    [CLASS_FiMatch] = 2,
    [CLASS_FiCase] = 2,
    [CLASS_FiElse] = 1,
    [CLASS_FiConsApp] = 2,
    [CLASS_FiPrimApp] = 2,
};

long runtime_0;
long runtime_1;
long runtime_2;
long runtime_3;

long nil;

static void mustBe(unsigned short class, long x)
{
    if (runtime_class(x) != class)
        die("Type error.");
}

static void storeInit(unsigned long size)
{
    store.size = size;
    store.firstFree = 0;
    store.data = malloc(size);
    if (store.data == NULL)
        die("Failed to allocate memory.");
}

static long storeAlloc(unsigned long align, unsigned long size)
{
    unsigned long i;

    i = align * (store.firstFree + align - 1) / align;
    if (i + size > store.size)
        die("Out of memory.");

    store.firstFree = i + size;

    return i;
}

static void *storeAddr(long x)
{
    return store.data + ((unsigned long)x >> 16);
}

static long makeNumber(long n)
{
    return (long)((unsigned long)n << 16);
}

static long makeString(const char *s)
{
    unsigned long align;
    unsigned long size;
    unsigned long len;
    unsigned long i;

    len = strlen(s);
    align = sizeof(long);
    size = sizeof(long) + len + 1;
    i = storeAlloc(align, size);
    *(long *)(store.data + i) = makeNumber((long)len);
    memmove(store.data + i + sizeof(long), s, len + 1);

    return (long)(i << 16 | CLASS_String);
}

long runtime_makeString(const char *s)
{
    return makeString(s);
}

const char *runtime_stringValue(long s)
{
    if (runtime_class(s) != CLASS_String)
        die("Type error.");

    return storeAddr(s) + sizeof(long);
}

static long classArgSave;

unsigned short runtime_class(long x)
{
    classArgSave = x; /* TODO Remove this insane debugging hack! */
    return (unsigned short)((unsigned long)x & 0xffff);
}

static long fixnumValue(long n)
{
    unsigned long bits;

    bits = (unsigned long)n >> 16;
    if (n < 0)
        bits |= (unsigned long)0xffff << 48;

    return (long)bits;
}

long runtime_fixnumValue(long n)
{
    return fixnumValue(n);
}

long prim_fetch(long m, long k)
{
    long *tuple;
    unsigned char arity;
    long i;

    /* TODO What sanity checks do we want here? */

    arity = runtime_classArities[runtime_class(m)];
    i = fixnumValue(k);

    if (i >= arity)
        die("Fetching slot that does not exist.");

    tuple = storeAddr(m);
    return tuple[i];
}

long runtime_makeNumber(long n)
{
    return makeNumber(n);
}

long prim_die(long e)
{
    const char *s;

    mustBe(CLASS_String, e);

    s = storeAddr(e) + sizeof(long);
    die(s);

    return 0;
}

long runtime_makeTuple0(unsigned short class)
{
    if (runtime_classArities[class] != 0) {
        fprintf(stderr, "Class: %d Arity: %d\n", (int)class, 0);
        die("Arity error while making tuple.");
    }

    return (long)class;
}

long runtime_makeTuple1(unsigned short class, long a)
{
    unsigned long align;
    unsigned long size;
    unsigned long i;
    long *tuple;

    if (runtime_classArities[class] != 1) {
        fprintf(stderr, "Class: %d Arity: %d\n", (int)class, 1);
        die("Arity error while making tuple.");
    }

    align = sizeof(long);
    size = sizeof(long);
    i = storeAlloc(align, size);
    tuple = store.data + i;

    tuple[0] = a;

    return (long)(i << 16 | class);
}

long runtime_makeTuple2(unsigned short class, long a, long b)
{
    unsigned long align;
    unsigned long size;
    unsigned long i;
    long *tuple;

    if (runtime_classArities[class] != 2) {
        fprintf(stderr, "Class: %d Arity: %d\n", (int)class, 2);
        die("Arity error while making tuple.");
    }

    align = sizeof(long);
    size = 2 * sizeof(long);
    i = storeAlloc(align, size);
    tuple = store.data + i;

    tuple[0] = a;
    tuple[1] = b;

    return (long)(i << 16 | class);
}

long runtime_makeTuple3(unsigned short class, long a, long b, long c)
{
    unsigned long align;
    unsigned long size;
    unsigned long i;
    long *tuple;

    if (runtime_classArities[class] != 3) {
        fprintf(stderr, "Class: %d Arity: %d\n", (int)class, 3);
        die("Arity error while making tuple.");
    }

    align = sizeof(long);
    size = 3 * sizeof(long);
    i = storeAlloc(align, size);
    tuple = store.data + i;

    tuple[0] = a;
    tuple[1] = b;
    tuple[2] = c;

    return (long)(i << 16 | class);
}

long runtime_makeTuple4(unsigned short class, long a, long b, long c, long d)
{
    unsigned long align;
    unsigned long size;
    unsigned long i;
    long *tuple;

    if (runtime_classArities[class] != 4) {
        fprintf(stderr, "Class: %d Arity: %d\n", (int)class, 4);
        die("Arity error while making tuple.");
    }

    align = sizeof(long);
    size = 4 * sizeof(long);
    i = storeAlloc(align, size);
    tuple = store.data + i;

    tuple[0] = a;
    tuple[1] = b;
    tuple[2] = c;
    tuple[3] = d;

    return (long)(i << 16 | class);
}

void runtime_matchFailure(int line)
{
    char buf[256];
    snprintf(buf, sizeof(buf), "Match failure. Line: %d. Class: %d",
        line, (int)runtime_class(classArgSave));
    die(buf);
}

long prim_cons(long a, long d)
{
    return runtime_makeTuple2(CLASS_Cons, a, d);
}

static int tmpCounter;
static int labelCounter;

long prim_genTmp(void)
{
    char name[16];
    snprintf(name, sizeof(name), "x%d", tmpCounter++);
    return Id(makeString(name));
}

long prim_genLabel(void)
{
    char name[16];
    snprintf(name, sizeof(name), "L%d", labelCounter++);
    return Id(makeString(name));
}

static const char *prims[] = {
    "fetch", "cons", "die", "genTmp", "genLabel",
};

int runtime_isPrim(const char *name)
{
    int i;
    for (i = 0; i < ARRAY_SIZE(prims); i++)
        if (!strcmp(prims[i], name))
            return 1;
    return 0;
}

void runtime_init(void)
{
    storeInit(128 * 1024 * 1024);
    runtime_0 = runtime_makeNumber(0);
    runtime_1 = runtime_makeNumber(1);
    runtime_2 = runtime_makeNumber(2);
    runtime_3 = runtime_makeNumber(3);
    nil = runtime_makeTuple0(CLASS_Nil);
}
