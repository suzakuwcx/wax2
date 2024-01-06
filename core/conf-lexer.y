%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <wax/conf-file.h>
#include <wax/conf.h>

#include "conf-lexer-yacc.h"

void yyerror(const char *str)
{
    fprintf(stderr, "syntax error at line %d: %s\n", yylineno, yylval.string);
}

int yywrap()
{
    return 1;
}

%}

%union
{
	char *string;
}

%token <string> T_TEXT
%token T_NEWLINE
%token T_EOF

%type <string> value

%%
configs:
    |
    config configs
    ;

config:
    statement T_NEWLINE
    |
    T_NEWLINE
    |
    statement T_EOF { YYACCEPT; }
    |
    T_EOF { YYACCEPT; }
    ;

statement:
    T_TEXT '=' value
    {
        config_set_value($1, $3);
        free($1);
        free($3);
    }
    ;

value:
    T_TEXT { $$ = $1; }
    |
    { $$ = strdup(""); }
    ;
%%
