#include <stdio.h>

extern FILE *yyin;
extern int yylineno;

int yywrap();
int yylex();

/* The entry to start parse */
int yyparse();
