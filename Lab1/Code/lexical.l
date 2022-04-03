%option yylineno
%{
#include"syntax.tab.h"
#include"myTree.h"
    extern int lexical_errors;
    extern YYSTYPE yylval;
    int yycolumn = 1;
    #define YY_USER_ACTION \
    yylloc.first_line = yylloc.last_line = yylineno; \
    yylloc.first_column = yycolumn; \
    yylloc.last_column = yycolumn + yyleng - 1; \
    yycolumn += yyleng;
%}

blank [\n|\t|' ']
annotationLine "//"
annotationPara "/*"
digit [0-9]
letter [_a-zA-Z]
INT -?(0|[1-9][0-9]*)
FLOAT -?([0-9]+\.[0-9]*|\.[0-9]+)
ID {letter}({letter}|{digit})*
SEMI ";"
COMMA ,
ASSIGNOP "="
RELOP >|<|<=|>=|==|!=
PLUS \+
MINUS -
STAR \*
DIV \/
AND &&
OR \|\|
DOT  \.
NOT  !
TYPE int|float
LP  \(
RP  \)
LB  \[
RB  \]
LC  \{
RC  \}
STRUCT  struct
RETURN  return
IF  if
ELSE  else
WHILE  while

%%
\n { yycolumn = 1; }
"//" {
    char c=input();
    while(c!='\n')
        c=input();
}
"/*" {
        char c=input();
        int sign=0;
        while(sign==0 && c!='\0'){
            if (c=='*'){
                c=input();
                if (c=='/'){
                    sign=1;
                }
            }
            else
                c=input();
        }  
        if (sign==0){
            lexical_errors++;
            printf("Error type A at Line %d: Unmatched annotation\n", yylineno);
        }
    }

{blank}     {}
{TYPE}      { yylval.tree_node=token_node("TYPE",yytext); return TYPE;}
{STRUCT}    { yylval.tree_node=token_node("STRUCT",NULL); return STRUCT;}
{RETURN}    { yylval.tree_node=token_node("RETURN",NULL); return RETURN;}
{IF}        { yylval.tree_node=token_node("IF",NULL); return IF;}
{ELSE}      { yylval.tree_node=token_node("ELSE",NULL); return ELSE;}
{WHILE}     { yylval.tree_node=token_node("WHILE",NULL); return WHILE;}

{INT}       { yylval.tree_node=token_node("INT",yytext);  return INT;}
{FLOAT}     { yylval.tree_node=token_node("FLOAT",yytext);  return FLOAT;}
{ID}        { yylval.tree_node=token_node("ID",yytext); return ID;}
{SEMI}      { yylval.tree_node=token_node("SEMI",NULL); return SEMI;}
{COMMA}     { yylval.tree_node=token_node("COMMA",NULL); return COMMA;}
{ASSIGNOP}  { yylval.tree_node=token_node("ASSIGNOP",NULL); return ASSIGNOP;}
{RELOP}     { yylval.tree_node=token_node("RELOP",NULL); return RELOP;}
{PLUS}      { yylval.tree_node=token_node("PLUS",NULL); return PLUS;}
{MINUS}     { yylval.tree_node=token_node("MINUS",NULL); return MINUS;}
{STAR}      { yylval.tree_node=token_node("STAR",NULL); return STAR;}
{DIV}       { yylval.tree_node=token_node("DIV",NULL); return DIV;}
{AND}       { yylval.tree_node=token_node("AND",NULL); return AND;}
{OR}        { yylval.tree_node=token_node("OR",NULL); return OR;}
{DOT}       { yylval.tree_node=token_node("DOT",NULL); return DOT;}
{NOT}       { yylval.tree_node=token_node("NOT",NULL); return NOT;}

{LP}        { yylval.tree_node=token_node("LP" ,NULL); return LP;}
{RP}        { yylval.tree_node=token_node("RP" ,NULL); return RP;}
{LB}        { yylval.tree_node=token_node("LB" ,NULL); return LB;}
{RB}        { yylval.tree_node=token_node("RB" ,NULL); return RB;}
{LC}        { yylval.tree_node=token_node("LC" ,NULL); return LC;}
{RC}        { yylval.tree_node=token_node("RC" ,NULL); return RC;}
.           {    
                lexical_errors++;
                printf("Error type A at Line %d: Mysterious characters \'%s\'\n", yylineno, yytext);
            }
%%


