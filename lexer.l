%{
    #include <stdio.h>
%}

%option noyywrap

/* DEFINITIONS*/

DIGIT [0-9]
LETTER [a-zA-Z]
ID {LETTER}({LETTER}|{DIGIT})* 
SCI ((DIGIT)+("."{DIGIT}+)?)[eE][+-]?{DIGIT}+

%%
/* RULES*/
"if" {printf("if-> %s->IF\n", yytext);}



/* USER CODE*/
%%
int main()
{
    yylex();
    return 0;
}