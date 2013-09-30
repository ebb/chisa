#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "printer.h"
#include "runtime.h"
#include "util.h"

static int match(long x, unsigned short class, ...)
{
    va_list ap;
    long *p;
    int arity, i, matched;

    va_start(ap, class);

    matched = (runtime_class(x) == class);
    arity = runtime_classArities[class];
    for (i = 0; i < arity; i++) {
        p = va_arg(ap, long *);
        if (matched)
            *p = prim_fetch(x, runtime_makeNumber(i));
    }

    va_end(ap);

    return matched;
}

#define forEach(xs, x)                          \
    for (long forEach_state_##__LINE__ = xs;    \
            match(forEach_state_##__LINE__,     \
                CLASS_Cons, &x,                 \
                &forEach_state_##__LINE__); )

static void pr(const char *s)
{
    printf(s);
}

static void prStr(long s)
{
    pr(runtime_stringValue(s));
}

static void prNum(long n)
{
    printf("%ld", runtime_fixnumValue(n));
}

static long idName(long id)
{
    return prim_fetch(id, runtime_0);
}

static void prId(long id)
{
    prStr(idName(id));
}

static void prIds(long ids)
{
    long id;
    const char *sep = "";

    forEach(ids, id)
        pr(sep), prId(id), sep = ", ";
}

static void prTypedIds(long ids)
{
    long id;
    const char *sep = "";

    forEach(ids, id)
        pr(sep), pr("long "), prId(id), sep = ", ";
}

static void prExpr(long expr)
{
    long id, args, name;

    if (match(expr, CLASS_FiPrimApp, &id, &args))
        pr("prim_"), prId(id), pr("("), prIds(args), pr(")");
    else if (match(expr, CLASS_FiConsApp, &id, &args))
        prId(id), pr("("), prIds(args), pr(")");
    else if (match(expr, CLASS_Fixnum))
       pr("runtime_makeNumber("), prNum(expr), pr(")");
    else if (match(expr, CLASS_String))
        pr("runtime_makeString(\""), prStr(expr), pr("\")");
    else if (match(expr, CLASS_Id, &name))
        prId(expr);
    else {
        fprintf(stderr, "Expression class: %d.\n", (int)runtime_class(expr));
        die("Unknown expression class.");
    }
}

static long findArgs(long key, long blocks)
{
    const char *keyString;
    long id, args, stmts, transfer, block;

    keyString = runtime_stringValue(key);
    forEach(blocks, block) {
        if (match(block, CLASS_FiBlock, &id, &args, &stmts, &transfer))
            if (!strcmp(runtime_stringValue(idName(id)), keyString))
                return args;
    }

    die("Failed to find arguments for block.");
    return nil;
}

static void prTransfer(long transfer, long blocks)
{
    long ret, id, arg, args, name, clauses, els, cont, formalArg;

    if (match(transfer, CLASS_FiCall, &ret, &id, &args)) {
        /*
         * Call
         */
        name = idName(id);
        if (match(ret, CLASS_Nil)) {
            pr("    return ");
            if (runtime_isPrim(runtime_stringValue(name)))
                pr("prim_");
            prStr(name), pr("("), prIds(args), pr(");\n");
        } else if (match(ret, CLASS_Id, &cont)) {
            long vars;

            vars = findArgs(cont, blocks);
            pr("    "), prIds(vars), pr(" = "), prStr(name);
            pr("("), prIds(args), pr(");\n");
            pr("    goto "), prStr(cont), pr(";\n");
        }
    } else if (match(transfer, CLASS_FiGoto, &id, &args)) {
        /*
         * Goto
         */
        forEach(findArgs(idName(id), blocks), formalArg) {
            if (!match(args, CLASS_Cons, &arg, &args))
                die("Wrong number of arguments in goto.");
            pr("        "), prId(formalArg), pr(" = "), prId(arg), pr(";\n");
        }
        pr("    goto "), prId(id), pr(";\n");
    } else if (match(transfer, CLASS_FiReturn, &id)) {
        /*
         * Return
         */
        pr("    return "), prId(id), pr(";\n");
    } else if (match(transfer, CLASS_FiMatch, &id, &clauses, &els)) {
        /*
         * Match
         */
        pr("    switch (runtime_class("), prId(id), pr(")) {\n");
        {
            /*
             * Cases
             */
            long clause, cons, label, arg;
            int i, elseCounter = 0;

            forEach(clauses, clause) {
                if (match(clause, CLASS_FiCase, &cons, &label)) {
                    pr("    case CLASS_"), prId(cons), pr(":\n");
                    i = 0;
                    forEach(findArgs(idName(label), blocks), arg) {
                        pr("        "), prId(arg), pr(" = prim_fetch(");
                        prId(id);
                        pr(", "), printf("runtime_makeNumber(%d)", i++);
                        pr(");\n");
                    }
                    pr("        goto "), prId(label), pr(";\n");
                }
                if (match(clause, CLASS_FiElse, &id)) {
                    pr("    default:\n");
                    pr("        goto "), prId(id), pr(";\n");
                    elseCounter++;
                }
            }
            if (elseCounter == 0) {
                pr("    default:\n");
                pr("        runtime_matchFailure(__LINE__);\n");
            }
        }
        pr("    }\n");
    }
}

static void prVariables(long blocks)
{
    long id, block, var, allVars = nil, arg, args, stmts, stmt, transfer;
    long expr;
    const char *sep = "";

    forEach(blocks, block) {
        if (match(block, CLASS_FiBlock, &id, &args, &stmts, &transfer)) {
            forEach(args, arg)
                allVars = prim_cons(arg, allVars);
            forEach(stmts, stmt)
                if (match(stmt, CLASS_FiStmt, &id, &expr))
                    allVars = prim_cons(id, allVars);
        }
    }

    if (allVars == nil)
        return;

    pr("    long ");
    forEach(allVars, var)
        pr(sep), prId(var), sep = ", ";
    pr(";\n");
}

static void prBlocks(long blocks)
{
    long id, args, stmts, transfer, expr, block, stmt;
    int flag = 0;

    forEach(blocks, block) {
        if (match(block, CLASS_FiBlock, &id, &args, &stmts, &transfer)) {
            if (!flag)
                pr("    goto "), prId(id), pr(";\n"), flag = 1;
            prId(id), pr(":\n");
            forEach(stmts, stmt)
                if (match(stmt, CLASS_FiStmt, &id, &expr))
                    pr("    "), prId(id), pr(" = "), prExpr(expr), pr(";\n");
            prTransfer(transfer, blocks);
        }
    }
}

static int length(long args)
{
    long arg;
    int len = 0;
    forEach(args, arg)
        len++;
    return len;
}

static void prFuncSpec(long name, long args)
{
    pr("long "), prStr(name), pr("(");
    if (length(args) > 0)
        prTypedIds(args);
    else
        pr("void");
    pr(")");
}

static int nrClasses(long defs)
{
    long def, id, args;
    int nr = 0;
    forEach(defs, def) {
        if (match(def, CLASS_FiDefineCons, &id, &args))
            nr++;
    }
    return nr;
}

static unsigned char arities[1 << 16];
static int classCounter = USER_CLASS_MIN;

void print(long fi)
{
    long def, id, args, value, blocks;

    /*
     * Includes.
     */
    pr("#include \"runtime.h\"\n");

    /*
     * Enums for classes.
     */
    if (nrClasses(fi) > 0) {
        pr("\n");
        pr("enum {\n");
        {
            const char *initializer = " = USER_CLASS_MIN";

            forEach(fi, def) {
                if (match(def, CLASS_FiDefineCons, &id, &args)) {
                    pr("    CLASS_"), prId(id), pr(initializer), pr(",\n");
                    initializer = "";
                }
            }
        }
        pr("};\n");
    }

    /*
     * Declarations.
     */
    {
        int isVar;

        forEach(fi, def) {
            pr("\n");
            if (match(def, CLASS_FiDefineVar, &id, &value))
                isVar = 1;
            else if (match(def, CLASS_FiDefineFunc, &id, &args, &blocks))
                isVar = 0;
            else if (match(def, CLASS_FiDefineCons, &id, &args))
                isVar = 0;
            if (isVar)
                pr("long "), prId(id), pr(";\n");
            else
                prFuncSpec(idName(id), args), pr(";\n");
        }
    }

    /*
     * Definitions.
     */
    {
        forEach(fi, def) {
            if (match(def, CLASS_FiDefineFunc, &id, &args, &blocks)) {
                /*
                 * Func
                 */
                pr("\n");
                prFuncSpec(idName(id), args), pr("\n");
                pr("{\n");
                prVariables(blocks);
                prBlocks(blocks);
                pr("}\n");
            } else if (match(def, CLASS_FiDefineCons, &id, &args)) {
                /*
                 * Cons
                 */
                pr("\n");
                prFuncSpec(idName(id), args), pr("\n");
                pr("{\n");
                {
                    int len;

                    len = length(args);
                    arities[classCounter++] = length(args);
                    printf("    return runtime_makeTuple%d(CLASS_", len);
                    prId(id);
                    if (len > 0)
                        pr(", "), prIds(args), pr(");\n");
                    else
                        pr(");\n");
                }
                pr("}\n");
            }
        }
    }

    /*
     * compiler_init() (For setting globals)
     */
    {
        int i;

        pr("\n");
        pr("void compiler_init(void)\n");
        pr("{\n");
        for (i = USER_CLASS_MIN; i < classCounter; i++) {
            printf("    runtime_classArities[%d] = %d;\n",
                i, (int)arities[i]);
        }
        forEach(fi, def) {
            if (match(def, CLASS_FiDefineVar, &id, &value)) {
                pr("    "), prId(id), pr(" = ");
                if (match(value, CLASS_Fixnum))
                    pr("runtime_makeNumber("), prNum(value);
                else if (match(value, CLASS_String))
                    pr("runtime_makeString(\""), prStr(value), pr("\"");
                else
                    die("Unknown constant type.");
                pr(");\n");
            }
        }
        pr("}\n");
    }
}
