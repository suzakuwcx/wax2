%{
#include <stdio.h>

#include "cluster-lexer-yacc.h"

extern struct cluster_conf *g_conf_yacc;

static void strip_space(char *text)
{
    for (int i = strlen(clusterlval.string) - 1; i >= 0; --i) {
        if (clusterlval.string[i] == ' ')
            clusterlval.string[i] = '\0';
        else
            break;
    }
}
%}

%option yylineno

%s VALUE

%%
<INITIAL>[[:alnum:]\.\"_-][[:alnum:]\.\"\ _-]* {
    clusterlval.string = strdup(yytext);
    strip_space(clusterlval.string);
    return T_KEY;
}

= BEGIN(VALUE);

<VALUE>[[:alnum:]\.\"=_-][[:alnum:]\.\"\ =_-]* {
    clusterlval.string = strdup(yytext);
    strip_space(clusterlval.string);
    return T_VALUE;
}

\n BEGIN(INITIAL);return T_EOF;

[ \t]+             /* ignore whitespace */;
#.*[<<EOF>>\n]     /* ignore commment */;
";".*[<<EOF>>\n]   /* ignore commment */;
\[.*\][<<EOF>>\n]  /* ignore title */;
%%

void clusterreparse(struct cluster_conf *conf)
{
    g_conf_yacc = conf;
    YY_FLUSH_BUFFER;
    clusterlineno = 0;
    clusterparse();
}
