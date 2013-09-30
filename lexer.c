#include "util.h"

int lexer_lineNr;

struct {
    const char *keyword;
    int token;
} keywords[] = {
    { "define", DEFINE },
    { "func", FUNC },
    { "block", BLOCK },
    { "begin", BEGIN },
    { "match", MATCH },
    { "switch", SWITCH },
    { "branch", BRANCH },
    { "case", CASE },
    { "else", ELSE },
    { "set", SET },
    { "return", RETURN },
    { "goto", GOTO },
};

void lexer_init(void)
{
    lexer_lineNr = 1;
}

int yylex(void)
{
    int c;
    char *buf;
    int i;
    int isNumber;

    buf = yylval.text;

    for (;;) {
        do {
            c = fgetc(stdin);
            if (c == '\n')
                lexer_lineNr++;
        } while (c == ' ' || c == '\n');
        if (c != '#')
            break;
        do c = fgetc(stdin); while (c != '\n');
        lexer_lineNr++;
    }

    if (c == EOF)
        return EOF;

    if (c == '(' || c == ')')
        return c;

    if (c == '"') {
        for (i = 0; ; buf[i++] = c) {
            if (i == sizeof(yylval.text))
                die("String is too large.");
            c = fgetc(stdin);
            if (c == '"')
                break;
            if (c == EOF)
                die("Incomplete input.");
        }
        buf[i] = '\0';
        yylval.syntax = runtime_makeString(buf);
        return STRING;
    }

    if (!isalnum(c))
        die("Bad token.");
    isNumber = isdigit(c);

    i = 0;
    do {
        buf[i++] = (char)c;
        if (i == sizeof(yylval.text))
            die("Token is too large.");
        c = fgetc(stdin);
        if (isNumber && isalpha(c))
            die("Bad token.");
    } while(isalnum(c));
    buf[i] = '\0';

    if (ungetc(c, stdin) == EOF)
        die("File stream error.");

    if (isNumber) {
        yylval.syntax = runtime_makeNumber(atol(buf));
        return NUMBER;
    }

    for (i = 0; i < ARRAY_SIZE(keywords); i++)
        if (!strcmp(keywords[i].keyword, buf))
            return keywords[i].token;

    yylval.syntax = runtime_makeTuple1(CLASS_Id, runtime_makeString(buf));
    return ID;
}
