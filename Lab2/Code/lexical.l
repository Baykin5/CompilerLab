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

blank [\n\t\r\v|' ']
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
{TYPE}      { yylval.tree_node=tokenNode("TYPE",TYPE_TYPE,yytext); return TYPE;}
{STRUCT}    { yylval.tree_node=tokenNode("STRUCT",TYPE_OTHER,NULL); return STRUCT;}
{RETURN}    { yylval.tree_node=tokenNode("RETURN",TYPE_OTHER,NULL); return RETURN;}
{IF}        { yylval.tree_node=tokenNode("IF",TYPE_OTHER,NULL); return IF;}
{ELSE}      { yylval.tree_node=tokenNode("ELSE",TYPE_OTHER,NULL); return ELSE;}
{WHILE}     { yylval.tree_node=tokenNode("WHILE",TYPE_OTHER,NULL); return WHILE;}

{INT}       { yylval.tree_node=tokenNode("INT",TYPE_INT,yytext);  return INT;}
{FLOAT}     { yylval.tree_node=tokenNode("FLOAT",TYPE_FLOAT,yytext);  return FLOAT;}
{ID}        { yylval.tree_node=tokenNode("ID",TYPE_ID,yytext); return ID;}
{SEMI}      { yylval.tree_node=tokenNode("SEMI",TYPE_OTHER,NULL); return SEMI;}
{COMMA}     { yylval.tree_node=tokenNode("COMMA",TYPE_OTHER,NULL); return COMMA;}
{ASSIGNOP}  { yylval.tree_node=tokenNode("ASSIGNOP",TYPE_OTHER,NULL); return ASSIGNOP;}
{RELOP}     { yylval.tree_node=tokenNode("RELOP",TYPE_OTHER,NULL); return RELOP;}
{PLUS}      { yylval.tree_node=tokenNode("PLUS",TYPE_OTHER,NULL); return PLUS;}
{MINUS}     { yylval.tree_node=tokenNode("MINUS",TYPE_OTHER,NULL); return MINUS;}
{STAR}      { yylval.tree_node=tokenNode("STAR",TYPE_OTHER,NULL); return STAR;}
{DIV}       { yylval.tree_node=tokenNode("DIV",TYPE_OTHER,NULL); return DIV;}
{AND}       { yylval.tree_node=tokenNode("AND",TYPE_OTHER,NULL); return AND;}
{OR}        { yylval.tree_node=tokenNode("OR",TYPE_OTHER,NULL); return OR;}
{DOT}       { yylval.tree_node=tokenNode("DOT",TYPE_OTHER,NULL); return DOT;}
{NOT}       { yylval.tree_node=tokenNode("NOT",TYPE_OTHER,NULL); return NOT;}

{LP}        { yylval.tree_node=tokenNode("LP" ,TYPE_OTHER,NULL); return LP;}
{RP}        { yylval.tree_node=tokenNode("RP" ,TYPE_OTHER,NULL); return RP;}
{LB}        { yylval.tree_node=tokenNode("LB" ,TYPE_OTHER,NULL); return LB;}
{RB}        { yylval.tree_node=tokenNode("RB" ,TYPE_OTHER,NULL); return RB;}
{LC}        { yylval.tree_node=tokenNode("LC" ,TYPE_OTHER,NULL); return LC;}
{RC}        { yylval.tree_node=tokenNode("RC" ,TYPE_OTHER,NULL); return RC;}
.           {    
                lexical_errors++;
                printf("Error type A at Line %d: Mysterious characters \'%s\'\n", yylineno, yytext);
            }
%%


