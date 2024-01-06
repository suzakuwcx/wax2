%{
#include <stdio.h>
#include <wax/conf-file.h>

#include "conf-lexer-yacc.h"
%}

%option yylineno

%%
[[:alnum:]_-]+  { yylval.string = strdup(yytext); return T_TEXT; }
= return '=';
\n return T_NEWLINE;
<<EOF>> return T_EOF;

[ \t]+          /* ignore whitespace */;
#.*[<<EOF>>\n]  /* ignore commment */;
%%

