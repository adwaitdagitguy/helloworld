%{
#include "y.tab.h"
%}

%%

[0-9]+          { yylval.intval = atoi(yytext); return NUM; }

">="            { return GE; }
"<="            { return LE; }
"=="            { return EQ; }
"!="            { return NE; }

[ \t]           { /* ignore whitespace */ }

\n              { return '\n'; }

.               { return yytext[0]; }

%%

int yywrap(void) { return 1; }
