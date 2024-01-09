%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <wax/conf-file.h>
#include <wax/conf.h>

#include <steam/cluster.h>

#include "conf-lexer-yacc.h"

bool is_parsing_wax = true;

void set_parsing_wax_conf_mode()
{
    is_parsing_wax = true;
}

void set_parsing_cluster_ini_mode()
{
    is_parsing_wax = false;
}

void wrapper_set_value(const char *key, const char *value)
{
    if (is_parsing_wax)
        config_set_value(key, value);
    else
        cluster_set_value(cluster_get_current_conf(), key, value);
}

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
        wrapper_set_value($1, $3);
        free($1);
        free($3);
    }
    ;

value:
    T_TEXT { $$ = $1;}
    |
    { $$ = strdup(""); }
    ;
%%
