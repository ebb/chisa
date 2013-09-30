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

static long hiProgram;

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
%type <syntax> exprs
%type <syntax> expr
%type <syntax> define
%type <syntax> defines
%type <syntax> func
%type <syntax> begin
%type <syntax> block
%type <syntax> stmt
%type <syntax> stmts
%type <syntax> call
%type <syntax> match
%type <syntax> clauses
%type <syntax> clause

%start program

%%

program     : defines { hiProgram = $1; }
defines     : { $$ = nil; }
            | define defines { $$ = prim_cons($1, $2); }
define      : defineVar | defineFunc | defineCons
defineVar   : '(' DEFINE ID expr ')' {
                $$ = runtime_makeTuple2(CLASS_HiDefineVar, $3, $4);
            }
defineFunc  : '(' DEFINE '(' ID ids ')' expr defines ')' {
                long block;
                block = runtime_makeTuple2(CLASS_HiBlock, $7, $8);
                $$ = runtime_makeTuple3(CLASS_HiDefineFunc, $4, $5, block);
            }
defineCons  : '(' DEFINE '(' ID ids ')' ')' {
                $$ = runtime_makeTuple2(CLASS_HiDefineCons, $4, $5);
            }
func        : '(' FUNC '(' ids ')' expr defines ')' {
                long block;
                block = runtime_makeTuple2(CLASS_HiBlock, $6, $7);
                $$ = runtime_makeTuple2(CLASS_HiFunc, $4, block);
            }
ids         : { $$ = nil; }
            | ID ids { $$ = prim_cons($1, $2); }
begin       : '(' BEGIN stmts ')' {
                $$ = runtime_makeTuple1(CLASS_HiBegin, $3);
            }
block       : '(' BLOCK expr defines ')' {
                $$ = runtime_makeTuple2(CLASS_HiBlock, $3, $4);
            }
stmts       : { $$ = nil; }
            | stmt stmts { $$ = prim_cons($1, $2); }
stmt        : '(' DEFINE ID expr defines ')' {
                long block;
                block = runtime_makeTuple2(CLASS_HiBlock, $4, $5);
                $$ = runtime_makeTuple2(CLASS_HiDefineVar, $3, block);
            }
            | '(' DEFINE '(' ID ids ')' expr defines ')' {
                /* TODO Use separate tokens for constructor identifiers. */
                const char *name;
                long block;
                name = runtime_stringValue(prim_fetch($4, 0));
                block = runtime_makeTuple2(CLASS_HiBlock, $7, $8);
                if (isupper(name[0]))
                    $$ = runtime_makeTuple3(
                        CLASS_HiDefineByMatch, $4, $5, block);
                else
                    $$ = runtime_makeTuple3(
                        CLASS_HiDefineFunc, $4, $5, block);
            }
            | expr
call        : '(' ID exprs ')' {
                $$ = runtime_makeTuple2(CLASS_HiCall, $2, $3);
            }
match       : '(' MATCH expr clauses ')' {
                $$ = runtime_makeTuple2(CLASS_HiMatch, $3, $4);
            }
clauses     : { $$ = nil; }
            | clause clauses { $$ = prim_cons($1, $2); }
clause      : '(' CASE '(' ID ids ')' expr defines ')' {
                long block;
                block = runtime_makeTuple2(CLASS_HiBlock, $7, $8);
                $$ = runtime_makeTuple3(CLASS_HiCase, $4, $5, block);
            }
            | '(' ELSE expr defines ')' {
                long block;
                block = runtime_makeTuple2(CLASS_HiBlock, $3, $4);
                $$ = runtime_makeTuple1(CLASS_HiElse, block);
            }
const       : NUMBER | STRING
exprs       : { $$ = nil; }
            | expr exprs { $$ = prim_cons($1, $2); }
expr        : const | ID | func | begin | block | call | match

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
    return hiProgram;
}
