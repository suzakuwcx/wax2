#include <stdio.h>

extern FILE *confin;
extern int conflineno;

int confwrap();
int conflex();

/* The entry to start parse */
void confreparse();
