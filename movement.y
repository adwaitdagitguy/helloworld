%{
#include <stdio.h>

int x = 0;
int y = 0;

void yyerror(const char *s);
int yylex();
%}

%token START N SOUTH E W

%%

S : START
  | S D
  ;

D : N      { y++; }
  | SOUTH  { y--; }
  | E      { x++; }
  | W      { x--; }
  ;

%%

int main()
{
    printf("Enter moves:\n");
    if(!yyparse())
        printf("Final position is (%d , %d)\n", x, y);
}

void yyerror(const char *s)
{
    printf("Invalid string\n");
}