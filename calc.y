%{
#include <stdio.h>
#include <stdlib.h>

void yyerror(char *s);
int  yylex(void);
%}

%union {
    int intval;
}

%token <intval> NUM
%type  <intval> S E T F

/* Lowest precedence – relational operators (left-associative) */
%left  GE LE EQ NE '>' '<'
/* Higher precedence – additive */
%left  '+' '-'
/* Highest precedence – multiplicative */
%left  '*' '/'

%%

program
    : program S '\n'  { printf("= %d\n", $2); }
    | /* empty */
    ;

S
    : E               { $$ = $1; }
    ;

E
    : E '+' T         { $$ = $1 + $3; }
    | E '-' T         { $$ = $1 - $3; }
    | E '>' E         { $$ = $1 >  $3; }
    | E '<' E         { $$ = $1 <  $3; }
    | E GE  E         { $$ = $1 >= $3; }
    | E LE  E         { $$ = $1 <= $3; }
    | E EQ  E         { $$ = $1 == $3; }
    | E NE  E         { $$ = $1 != $3; }
    | T               { $$ = $1; }
    ;

T
    : T '*' F         { $$ = $1 * $3; }
    | T '/' F         {
                        if ($3 == 0) { yyerror("divide by zero"); $$ = 0; }
                        else          $$ = $1 / $3;
                      }
    | F               { $$ = $1; }
    ;

F
    : '(' E ')'       { $$ = $2; }
    | NUM             { $$ = $1; }
    ;

%%

void yyerror(char *s) {
    fprintf(stderr, "Error: %s\n", s);
}

int main(void) {
    printf("Programmable Calculator (type expressions, end with Enter)\n");
    printf("Operators: + - * /  and  > < >= <= == !=\n\n");
    return yyparse();
}
