%{
#include <stdio.h>
%}

%option noyywrap

DIGIT    [0-9]
LETTER   [a-zA-Z_]
ID       {LETTER}({LETTER}|{DIGIT})*
INT      {DIGIT}+
FLOAT    {DIGIT}+"."{DIGIT}+
SCI      ({DIGIT}+("."{DIGIT}+)?)[eE][+-]?{DIGIT}+
STR_D    \"([^\\\"]|\\.)*\"
STR_S    \'([^\\\']|\\.)*\'

%%
    /* -------- Keywords -------- */
"if"        { printf("if -> %s -> IF\n", yytext); }
"else"      { printf("else -> %s -> ELSE\n", yytext); }
"elif"      { printf("elif -> %s -> ELIF\n", yytext); }
"for"       { printf("for -> %s -> FOR\n", yytext); }
"while"     { printf("while -> %s -> WHILE\n", yytext); }
"break"     { printf("break -> %s -> BREAK\n", yytext); }
"continue"  { printf("continue -> %s -> CONTINUE\n", yytext); }
"def"       { printf("def -> %s -> DEF\n", yytext); }
"return"    { printf("return -> %s -> RETURN\n", yytext); }
"class"     { printf("class -> %s -> CLASS\n", yytext); }
"pass"      { printf("pass -> %s -> PASS\n", yytext); }
"import"    { printf("import -> %s -> IMPORT\n", yytext); }
"from"      { printf("from -> %s -> FROM\n", yytext); }
"as"        { printf("as -> %s -> AS\n", yytext); }
"lambda"    { printf("lambda -> %s -> LAMBDA\n", yytext); }

    /* -------- Logical operators -------- */
"and"       { printf("and -> %s -> AND\n", yytext); }
"or"        { printf("or -> %s -> OR\n", yytext); }
"not"       { printf("not -> %s -> NOT\n", yytext); }
"in"        { printf("in -> %s -> IN\n", yytext); }

    /* -------- Assignment operators (Longest Match First) -------- */
"+="        { printf("+= -> %s -> ADD_ASSIGN\n", yytext); }
"-="        { printf("-= -> %s -> SUB_ASSIGN\n", yytext); }
"*="        { printf("*= -> %s -> MUL_ASSIGN\n", yytext); }
"/="        { printf("/= -> %s -> DIV_ASSIGN\n", yytext); }
"="         { printf("= -> %s -> ASSIGN\n", yytext); }

    /* -------- Relational operators -------- */
"=="        { printf("== -> %s -> EQ\n", yytext); }
"!="        { printf("!= -> %s -> NEQ\n", yytext); }
"<="        { printf("<= -> %s -> LE\n", yytext); }
">="        { printf(">= -> %s -> GE\n", yytext); }
"<"         { printf("< -> %s -> LT\n", yytext); }
">"         { printf("> -> %s -> GT\n", yytext); }

    /* -------- Arithmetic operators (Longest Match First) -------- */
"**"        { printf("** -> %s -> POWER\n", yytext); }
"//"        { printf("// -> %s -> FLOOR_DIV\n", yytext); }
"+"         { printf("+ -> %s -> PLUS\n", yytext); }
"-"         { printf("- -> %s -> MINUS\n", yytext); }
"*"         { printf("* -> %s -> MULTIPLY\n", yytext); }
"/"         { printf("/ -> %s -> DIVIDE\n", yytext); }
"%"         { printf("%% -> %s -> MODULUS\n", yytext); }

    /* -------- Literals (Specific to General Order) -------- */
{SCI}       { printf("SCI -> %s -> SCIENTIFIC_LITERAL\n", yytext); }
{FLOAT}     { printf("FLOAT -> %s -> FLOAT_LITERAL\n", yytext); }
{INT}       { printf("INT -> %s -> INTEGER_LITERAL\n", yytext); }
{STR_D}     { printf("STRING -> %s -> STRING_DOUBLE\n", yytext); }
{STR_S}     { printf("STRING -> %s -> STRING_SINGLE\n", yytext); }

    /* -------- Identifiers -------- */
{ID}        { printf("ID -> %s -> IDENTIFIER\n", yytext); }

    /* -------- Delimiters -------- */
"("         { printf("( -> %s -> LPAREN\n", yytext); }
")"         { printf(") -> %s -> RPAREN\n", yytext); }
"["         { printf("[ -> %s -> LBRACKET\n", yytext); }
"]"         { printf("] -> %s -> RBRACKET\n", yytext); }
","         { printf(", -> %s -> COMMA\n", yytext); }
":"         { printf(": -> %s -> COLON\n", yytext); }
"."         { printf(". -> %s -> DOT\n", yytext); }

    /* -------- Comments -------- */
"#".* { printf("COMMENT -> %s -> COMMENT\n", yytext); }

    /* -------- Ignore whitespace (Spaces, Tabs, Newlines, Carriage Returns) -------- */
[ \t\n\r]+  { /* Ignore */ }

    /* -------- Catch-all for errors -------- */
.           { printf("UNKNOWN -> %s -> ERROR\n", yytext); }

%%

int main() {
    yylex();
    return 0;
}