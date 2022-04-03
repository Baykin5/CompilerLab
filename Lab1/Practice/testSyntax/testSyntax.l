%{
#include"testSyntax.tab.h"
%}
%option yylineno
BLANK [' '|\n|\t]
ADD "+"
SUB "-"
MUL "*"
DIV "/"
INT -?(0|[1-9][0-9]*)
FLOAT -?([0-9]+\.[0-9]*|\.[0-9]+)

%%
{INT}       {
    printf("INT\n");
    yylval.type_int= atoi(yytext);
    return INT;
    }
{FLOAT} {
    printf("FLOAT\n");
    yylval.type_float= atof(yytext);
    return FLOAT;
}
{BLANK} {}
"+"      {printf("PLUS\n");return ADD;}
"-"     {printf("SUB\n");return SUB;}
"*"      {printf("MUL\n");return MUL;}
"/"      {printf("DIV\n");return DIV;}

.           printf("Error type A at Line %d: Mysterious characters \'%s\'\n", yylineno, yytext);
%%
