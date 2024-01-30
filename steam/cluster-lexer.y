%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <steam/cluster.h>

#include "cluster-lexer-yacc.h"

struct cluster_conf *g_conf_yacc;

void clustererror(const char *str)
{
    fprintf(stderr, "syntax error at line %d: %s\n", clusterlineno, clusterlval.string);
}

int clusterwrap()
{
    return 1;
}

%}

%union
{
	char *string;
}

%token <string> T_KEY T_VALUE
%token T_EOF

%%
configs:
    |
    config configs
    ;

config:
    T_KEY T_VALUE T_EOF
    {
        cluster_set_value(g_conf_yacc, $1, $2);
        free($1);
        free($2);
    }
    |
    T_EOF
    ;
%%
