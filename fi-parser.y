%{

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "parser.h"
#include "runtime.h"
#include "util.h"

int yylex(void);
int yyerror(const char *e);

static long fiProgram;

%}

%union {
    char text[64];
    long syntax;
}

%token <syntax> ID
%token <syntax> NUMBER
%token <syntax> STRING
%token DEFINE
%token FUNC
%token BLOCK
%token BEGIN
%token MATCH
%token SWITCH
%token BRANCH
%token CASE
%token ELSE
%token SET
%token GOTO
%token RETURN

%type <syntax> defineVar
%type <syntax> defineFunc
%type <syntax> defineCons
%type <syntax> ids
%type <syntax> const
%type <syntax> expr
%type <syntax> define
%type <syntax> defines
%type <syntax> block
%type <syntax> blocks
%type <syntax> stmt
%type <syntax> stmts
%type <syntax> transfer
%type <syntax> call
%type <syntax> match
%type <syntax> clauses
%type <syntax> goto
%type <syntax> return
%type <syntax> app

%start program

%%

program     : defines { fiProgram = $1; }
defines     : { $$ = nil; }
            | define defines { $$ = prim_cons($1, $2); }
define      : defineVar | defineFunc | defineCons
defineVar   : '(' DEFINE ID const ')' {
                $$ = runtime_makeTuple2(CLASS_FiDefineVar, $3, $4);
            }
defineFunc  : '(' DEFINE '(' ID ids ')' blocks ')' {
                $$ = runtime_makeTuple3(CLASS_FiDefineFunc, $4, $5, $7);
            }
defineCons  : '(' DEFINE '(' ID ids ')' ')' {
                $$ = runtime_makeTuple2(CLASS_FiDefineCons, $4, $5);
            }
ids         : { $$ = nil; }
            | ID ids { $$ = prim_cons($1, $2); }
blocks      : block { $$ = prim_cons($1, nil); }
            | block blocks { $$ = prim_cons($1, $2); }
block       : '(' DEFINE '(' ID ids ')' '(' stmts transfer ')' {
                $$ = runtime_makeTuple4(CLASS_FiBlock, $4, $5, $8, $9);
            }
stmts       : { $$ = nil; }
            | stmt stmts { $$ = prim_cons($1, $2); }
stmt        : SET ID expr ')' '(' {
                $$ = runtime_makeTuple2(CLASS_FiStmt, $2, $3);
            }
transfer    : call | match | goto | return
call        : ID '(' ID ids ')' ')' {
                $$ = runtime_makeTuple3(CLASS_FiCall, $1, $3, $4);
            }
call        : RETURN '(' ID ids ')' ')' {
                /* TODO What to use instead of nil? */
                $$ = runtime_makeTuple3(CLASS_FiCall, nil, $3, $4);
            }
match       : MATCH ID clauses ')' {
                $$ = runtime_makeTuple2(CLASS_FiMatch, $2, $3);
            }
clauses     : { $$ = nil; }
            | '(' CASE ID ID ')' clauses {
                $$ = prim_cons(runtime_makeTuple2(CLASS_FiCase, $3, $4), $6);
            }
            | '(' ELSE ID ')' clauses {
                $$ = prim_cons(runtime_makeTuple1(CLASS_FiElse, $3), $5);
            }
goto        : GOTO '(' ID ids ')' ')' {
                $$ = runtime_makeTuple2(CLASS_FiGoto, $3, $4);
            }
return      : RETURN ID ')' {
                $$ = runtime_makeTuple1(CLASS_FiReturn, $2);
            }
const       : NUMBER | STRING
expr        : const | app
app         : '(' ID ids ')' {
                const char *name;
                name = runtime_stringValue(prim_fetch($2, 0));
                if (isupper(name[0]))
                    $$ = runtime_makeTuple2(CLASS_FiConsApp, $2, $3);
                else
                    $$ = runtime_makeTuple2(CLASS_FiPrimApp, $2, $3);
            }

%%

#include "lexer.c"

int yyerror(const char *e)
{
    fprintf(stderr, "Line: %d\n", lexer_lineNr);
    die(e);
    return 0;
}

long parse(void)
{
    yyparse();
    return fiProgram;
}
