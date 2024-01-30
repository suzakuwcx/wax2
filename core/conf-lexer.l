%{
#include <stdio.h>
#include <wax/conf-file.h>

#include "conf-lexer-yacc.h"
%}

%option yylineno

%%
[[:alnum:]\.\"_-]+  { conflval.string = strdup(yytext); return T_TEXT; }
= return '=';
\n return T_NEWLINE;
<<EOF>> return T_EOF;


[ \t]+             /* ignore whitespace */;
#.*[<<EOF>>\n]     /* ignore commment */;
\[.*\][<<EOF>>\n]  /* ignore title */;
%%

void confreparse()
{
    YY_FLUSH_BUFFER;
    conflineno = 0;
    confparse();
}
