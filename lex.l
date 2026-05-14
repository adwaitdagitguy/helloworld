%{
#include "y.tab.h"
%}

%%
START   { return START; }
n       { return N; }
s       { return SOUTH; }
e       { return E; }
w       { return W; }
[ \t\n] ;
.       { return yytext[0]; }
%%

int yywrap()
{
    return 1;
}