#include <stdio.h>

extern FILE *yyin;
extern int yylineno;

int yywrap();
int yylex();

/* The entry to start parse */
int yyparse();

void set_parsing_wax_conf_mode();
void set_parsing_cluster_ini_mode();
void parser_restart();
