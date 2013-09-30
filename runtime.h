void runtime_init(void);

unsigned short runtime_class(long x);

long runtime_makeNumber(long n);
long runtime_makeString(const char *s);

long runtime_fixnumValue(long n);
const char *runtime_stringValue(long s);

long runtime_makeTuple0(unsigned short class);
long runtime_makeTuple1(unsigned short class, long a);
long runtime_makeTuple2(unsigned short class, long a, long b);
long runtime_makeTuple3(unsigned short class, long a, long b, long c);
long runtime_makeTuple4(unsigned short class, long a, long b, long c, long d);

void runtime_matchFailure(int line);

extern long runtime_0;
extern long runtime_1;
extern long runtime_2;
extern long runtime_3;

long prim_die(long e);
long prim_fetch(long m, long k);
extern long nil;
long prim_cons(long a, long d);
long prim_genTmp(void);
long prim_genLabel(void);

int runtime_isPrim(const char *name);

extern unsigned char runtime_classArities[];

enum {
    CLASS_Fixnum,
    CLASS_String,
    CLASS_Nil,
    CLASS_Cons,
    CLASS_Id,
    CLASS_HiDefineVar,
    CLASS_HiDefineFunc,
    CLASS_HiDefineCons,
    CLASS_HiDefineByMatch,
    CLASS_HiFunc,
    CLASS_HiBegin,
    CLASS_HiBlock,
    CLASS_HiCall,
    CLASS_HiConsApp,
    CLASS_HiPrimApp,
    CLASS_HiMatch,
    CLASS_HiCase,
    CLASS_HiElse,
    CLASS_FiDefineVar,
    CLASS_FiDefineFunc,
    CLASS_FiDefineCons,
    CLASS_FiBlock,
    CLASS_FiStmt,
    CLASS_FiCall,
    CLASS_FiGoto,
    CLASS_FiReturn,
    CLASS_FiMatch,
    CLASS_FiCase,
    CLASS_FiElse,
    CLASS_FiConsApp,
    CLASS_FiPrimApp,
    USER_CLASS_MIN,
};

static inline long Nil(void)
{
    return runtime_makeTuple0(CLASS_Nil);
}

static inline long Cons(long a, long d)
{
    return runtime_makeTuple2(CLASS_Cons, a, d);
}

static inline long Id(long name)
{
    return runtime_makeTuple1(CLASS_Id, name);
}

static inline long HiBegin(long forms)
{
    return runtime_makeTuple1(CLASS_HiBegin, forms);
}

static inline long HiBlock(long expr, long defines)
{
    return runtime_makeTuple2(CLASS_HiBlock, expr, defines);
}

static inline long HiMatch(long test, long clauses)
{
    return runtime_makeTuple2(CLASS_HiMatch, test, clauses);
}

static inline long HiCase(long c, long args, long block)
{
    return runtime_makeTuple3(CLASS_HiCase, c, args, block);
}

static inline long FiDefineVar(long id, long x)
{
    return runtime_makeTuple2(CLASS_FiDefineVar, id, x);
}

static inline long FiDefineFunc(long name, long args, long blocks)
{
    return runtime_makeTuple3(CLASS_FiDefineFunc, name, args, blocks);
}

static inline long FiDefineCons(long id, long args)
{
    return runtime_makeTuple2(CLASS_FiDefineCons, id, args);
}

static inline long FiBlock(long id, long args, long stmts, long transfer)
{
    return runtime_makeTuple4(CLASS_FiBlock, id, args, stmts, transfer);
}

static inline long FiStmt(long x, long expr)
{
    return runtime_makeTuple2(CLASS_FiStmt, x, expr);
}

static inline long FiCall(long cont, long f, long args)
{
    return runtime_makeTuple3(CLASS_FiCall, cont, f, args);
}

static inline long FiGoto(long label, long args)
{
    return runtime_makeTuple2(CLASS_FiGoto, label, args);
}

static inline long FiReturn(long x)
{
    return runtime_makeTuple1(CLASS_FiReturn, x);
}

static inline long FiMatch(long test, long clauses)
{
    return runtime_makeTuple2(CLASS_FiMatch, test, clauses);
}

static inline long FiCase(long c, long label)
{
    return runtime_makeTuple2(CLASS_FiCase, c, label);
}

static inline long FiElse(long label)
{
    return runtime_makeTuple1(CLASS_FiElse, label);
}

static inline long FiConsApp(long c, long args)
{
    return runtime_makeTuple2(CLASS_FiConsApp, c, args);
}

static inline long FiPrimApp(long p, long args)
{
    return runtime_makeTuple2(CLASS_FiPrimApp, p, args);
}
