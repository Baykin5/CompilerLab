%option yylineno
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
{blank}     {printf("------\n");}

"//" {
    char c=input();
    while(c!='\n')
        c=input();
    printf("annotationLine\n");
}
"/*" {
    char c=input();
    int sign=0;
    while(!sign){
        if (c=='*'){
            c=input();
            if (c=='/'){
                printf("annotationPara\n");
                sign=1;
            }
        }
        else
            c=input();
    }  
}
"*/" {
    printf("Error type A at Line %d\n", yylineno);
}

{TYPE}      {printf("TYPE\n");}
{STRUCT}    {printf("STRUCT\n");}
{RETURN}    {printf("RETURN\n");}
{IF}        {printf("IF\n");}
{ELSE}      {printf("ELSE\n");}
{WHILE}     {printf("WHILE\n");}

{INT}       {printf("INT\n");}
{FLOAT}     {printf("FLOAT\n");}
{ID}        {printf("ID\n");}
{SEMI}      {printf("SEMI\n");}
{COMMA}      {printf("COMMA\n");}
{ASSIGNOP}      {printf("ASSIGNOP\n");}
{RELOP}     {printf("RELOP\n");}
{PLUS}      {printf("PLUS\n");}
{MINUS}     {printf("MINUS\n");}
{STAR}      {printf("STAR\n");}
{DIV}       {printf("DIV\n");}
{AND}       {printf("AND\n");}
{OR}        {printf("OR\n");}
{DOT}       {printf("DOT\n");}
{NOT}       {printf("NOT\n");}

{LP}        {printf("LP\n");}
{RP}        {printf("RP\n");}
{LB}        {printf("LB\n");}
{RB}        {printf("RB\n");}
{LC}        {printf("LC\n");}
{RC}        {printf("RC\n");}

.           printf("Error type A at Line %d: Mysterious characters \'%s\'\n", yylineno, yytext);
%%
