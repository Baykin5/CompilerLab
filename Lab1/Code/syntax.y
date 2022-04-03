%locations
%{
    #include<stdio.h>
    #include "myTree.h"
    extern int yylineno;
    extern char* yytext;
    extern int syntax_errors;
    extern struct treenode* root;
    #define YYDEBUG 1
%}
%union {
    struct treenode* tree_node
}

/* High-level Definitions */
%type <tree_node> Program
%type <tree_node> ExtDefList
%type <tree_node> ExtDef
%type <tree_node> ExtDecList
/* Specifiers */
%type <tree_node> Specifier
%type <tree_node> StructSpecifier
%type <tree_node> OptTag
%type <tree_node> Tag
/* Declarators */
%type <tree_node> VarDec
%type <tree_node> FunDec
%type <tree_node> VarList
%type <tree_node> ParamDec
/* Statements */
%type <tree_node> CompSt
%type <tree_node> StmtList
%type <tree_node> Stmt
/* Local Definitions */
%type <tree_node> DefList
%type <tree_node> Def
%type <tree_node> DecList
%type <tree_node> Dec
/* Expressions */
%type <tree_node> Exp
%type <tree_node> Args
%start Program

%token <tree_node> INT
%token <tree_node> FLOAT
%token <tree_node> ID
%token <tree_node> SEMI COMMA
%token <tree_node> ASSIGNOP
%token <tree_node> RELOP
%token <tree_node> PLUS MINUS STAR DIV
%token <tree_node> AND OR NOT
%token <tree_node> TYPE
%token <tree_node> LP RP LB RB LC RC
%token <tree_node> STRUCT RETURN IF ELSE WHILE 
%token <tree_node> DOT

%left ASSIGNOP
%left OR
%left AND
%left RELOP
%left PLUS MINUS
%left STAR DIV
%right NOT
%left LP LB RP RB DOT

%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

%%
/* High-level Definitions */
Program : ExtDefList                                            { $$=nonterminal_node("Program",@$.first_line,1,$1);  
                                                                 root=$$; }
;                                                                
ExtDefList : ExtDef ExtDefList                                  { $$=nonterminal_node("ExtDefList",@$.first_line,2,$1,$2); }
    | /* empty */                                               { $$=NULL; }
    ;
ExtDef : Specifier ExtDecList SEMI                              { $$=nonterminal_node("ExtDef",@$.first_line,3,$1,$2,$3); }
    | Specifier SEMI                                            { $$=nonterminal_node("ExtDef",@$.first_line,2,$1,$2); }
    | Specifier FunDec CompSt                                   { $$=nonterminal_node("ExtDef",@$.first_line,3,$1,$2,$3); }
    | error SEMI { yyerrok; }
    | Specifier error SEMI { yyerrok; }
    | Specifier error { yyerrok; }
    ;
ExtDecList : VarDec                                             { $$=nonterminal_node("ExtDecList",@$.first_line,1,$1); }
    | VarDec COMMA ExtDecList                                   { $$=nonterminal_node("ExtDecList",@$.first_line,2,$1,$2); }
    ;

/* Specifiers */
Specifier : TYPE                                                { $$=nonterminal_node("Specifier",@$.first_line,1,$1); }
    | StructSpecifier                                           { $$=nonterminal_node("Specifier",@$.first_line,1,$1); }
    ;
StructSpecifier : STRUCT OptTag LC DefList RC                   { $$=nonterminal_node("StructSpecifier",@$.first_line,5,$1,$2,$3,$4,$5); }
    | STRUCT Tag                                                { $$=nonterminal_node("StructSpecifier",@$.first_line,2,$1,$2); }
    | STRUCT OptTag LC DefList error RC  { yyerrok; }
    ;
OptTag : ID                                                     { $$=nonterminal_node("OptTag",@$.first_line,1,$1); }
    | /* empty */                                               { $$=NULL; }
    ;
Tag : ID                                                        { $$=nonterminal_node("Tag",@$.first_line,1,$1); }
    ;
/* Declarators */
VarDec : ID                                                     { $$=nonterminal_node("VarDec",@$.first_line,1,$1); }
    | VarDec LB INT RB                                          { $$=nonterminal_node("VarDec",@$.first_line,4,$1,$2,$3,$4); }                      
    | VarDec LB error RB { yyerrok; }

    ;
FunDec : ID LP VarList RP                                       { $$=nonterminal_node("FunDec",@$.first_line,4,$1,$2,$3,$4); }
    | ID LP RP                                                  { $$=nonterminal_node("FunDec",@$.first_line,3,$1,$2,$3); }
    | ID LP error RP { yyerrok; }

    ;
VarList : ParamDec COMMA VarList                                { $$=nonterminal_node("VarList",@$.first_line,3,$1,$2,$3); }
    | ParamDec                                                  { $$=nonterminal_node("VarList",@$.first_line,1,$1); }
    ;
ParamDec : Specifier VarDec                                     { $$=nonterminal_node("ParamDec",@$.first_line,2,$1,$2); }
    ;

/* Statements */
CompSt : LC DefList StmtList RC                                 { $$=nonterminal_node("CompSt",@$.first_line,4,$1,$2,$3,$4); }
    ;
StmtList : Stmt StmtList                                        { $$=nonterminal_node("StmtList",@$.first_line,2,$1,$2); }
    | /* empty */                                               { $$=NULL; }
    ;
Stmt : Exp SEMI                                                 { $$=nonterminal_node("Stmt",@$.first_line,2,$1,$2); }
    | CompSt                                                    { $$=nonterminal_node("Stmt",@$.first_line,1,$1); }
    | RETURN Exp SEMI                                           { $$=nonterminal_node("Stmt",@$.first_line,3,$1,$2,$3); }
    | IF LP Exp RP Stmt %prec LOWER_THAN_ELSE                   { $$=nonterminal_node("Stmt",@$.first_line,5,$1,$2,$3,$4,$5); }
    | IF LP Exp RP Stmt ELSE Stmt                               { $$=nonterminal_node("Stmt",@$.first_line,7,$1,$2,$3,$4,$5,$6,$7); }
    | WHILE LP Exp RP Stmt                                      { $$=nonterminal_node("Stmt",@$.first_line,5,$1,$2,$3,$4,$5); }
    | error SEMI  { yyerrok; }
    | IF LP error RP Stmt %prec LOWER_THAN_ELSE { yyerrok; }
    | IF LP Exp RP error ELSE Stmt { yyerrok; }
    | WHILE LP error RP Stmt { yyerrok; }
    | RETURN Exp error { yyerrok; }
    ;

/* Local Definitions */
DefList : Def DefList                                           { $$=nonterminal_node("DefList",@$.first_line,2,$1,$2); }
    | /* empty */                                               { $$=NULL; }
    ;
Def : Specifier DecList SEMI                                    { $$=nonterminal_node("Def",@$.first_line,3,$1,$2,$3); }
    | Specifier error SEMI { yyerrok; }
    ;
DecList : Dec                                                   { $$=nonterminal_node("DecList",@$.first_line,1,$1); }
    | Dec COMMA DecList                                         { $$=nonterminal_node("DecList",@$.first_line,3,$1,$2,$3); }
    ;
Dec : VarDec                                                    { $$=nonterminal_node("Dec",@$.first_line,1,$1); }
    | VarDec ASSIGNOP Exp                                       { $$=nonterminal_node("Dec",@$.first_line,3,$1,$2,$3); }
    ;

/* Expressions */
Exp : Exp ASSIGNOP Exp                                          { $$=nonterminal_node("Exp",@$.first_line,3,$1,$2,$3); }
    | Exp AND Exp                                               { $$=nonterminal_node("Exp",@$.first_line,3,$1,$2,$3); }
    | Exp OR Exp                                                { $$=nonterminal_node("Exp",@$.first_line,3,$1,$2,$3); }
    | Exp RELOP Exp                                             { $$=nonterminal_node("Exp",@$.first_line,3,$1,$2,$3); }
    | Exp PLUS Exp                                              { $$=nonterminal_node("Exp",@$.first_line,3,$1,$2,$3); }
    | Exp MINUS Exp                                             { $$=nonterminal_node("Exp",@$.first_line,3,$1,$2,$3); }
    | Exp STAR Exp                                              { $$=nonterminal_node("Exp",@$.first_line,3,$1,$2,$3); }
    | Exp DIV Exp                                               { $$=nonterminal_node("Exp",@$.first_line,3,$1,$2,$3); }
    | LP Exp RP                                                 { $$=nonterminal_node("Exp",@$.first_line,3,$1,$2,$3); }
    | MINUS Exp                                                 { $$=nonterminal_node("Exp",@$.first_line,2,$1,$2); }
    | NOT Exp                                                   { $$=nonterminal_node("Exp",@$.first_line,2,$1,$2); }
    | ID LP Args RP                                             { $$=nonterminal_node("Exp",@$.first_line,4,$1,$2,$3,$4); }
    | ID LP RP                                                  { $$=nonterminal_node("Exp",@$.first_line,3,$1,$2,$3); }
    | Exp LB Exp RB                                             { $$=nonterminal_node("Exp",@$.first_line,4,$1,$2,$3,$4); }
    | Exp DOT ID                                                { $$=nonterminal_node("Exp",@$.first_line,3,$1,$2,$3); }
    | ID                                                        { $$=nonterminal_node("Exp",@$.first_line,1,$1); }
    | INT                                                       { $$=nonterminal_node("Exp",@$.first_line,1,$1); }
    | FLOAT                                                     { $$=nonterminal_node("Exp",@$.first_line,1,$1); }
    | LP error RP { yyerrok; }   
    | ID LP error RP { yyerrok; }  
    | Exp LB error RB { yyerrok; }  
    | Exp ASSIGNOP error { yyerrok;}  
    | Exp AND error { yyerrok;}    
    | Exp OR error { yyerrok;}    
    | Exp RELOP error { yyerrok;}    
    | Exp PLUS error { yyerrok;}    
    | Exp MINUS error { yyerrok;}  
    | Exp STAR error { yyerrok;}    
    | Exp DIV error { yyerrok;}     
    | LP Exp error { yyerrok;}       
    | NOT error { yyerrok;}  
    ;
Args : Exp COMMA Args                                           { $$=nonterminal_node("Args",@$.first_line,3,$1,$2,$3); }
    | Exp                                                       { $$=nonterminal_node("Args",@$.first_line,1,$1); }
    ;

%%
#include "lex.yy.c"
yyerror(char* msg) {
    syntax_errors++;
    if (!strcmp(yytext,"")==0)
        printf("Error type B at Line %d: %s, near \"%s\".\n",yylineno,msg,yytext);
}
