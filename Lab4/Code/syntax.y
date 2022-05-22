%locations
%{
    #include<stdio.h>
    #include "myTree.h"
    
    typedef struct TreeNode_ *TreeNode;
    
    extern int yylineno;
    extern char* yytext;
    extern int syntax_errors;
    extern TreeNode root;
    #define YYDEBUG 1
%}
%union {
    struct TreeNode_ *tree_node
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
Program : ExtDefList                                        { $$=nonterminalNode("Program",@$.first_line,1,$1);  
                                                                 root=$$; }
;                                                                
ExtDefList : ExtDef ExtDefList                              { $$=nonterminalNode("ExtDefList",@$.first_line,2,$1,$2); }
    | /* empty */                                           { $$=NULL; }
    ;
ExtDef : Specifier ExtDecList SEMI                          { $$=nonterminalNode("ExtDef",@$.first_line,3,$1,$2,$3); }
    | Specifier SEMI                                        { $$=nonterminalNode("ExtDef",@$.first_line,2,$1,$2); }
    | Specifier FunDec CompSt                               { $$=nonterminalNode("ExtDef",@$.first_line,3,$1,$2,$3); }
    | Specifier FunDec SEMI                                 { $$=nonterminalNode("ExtDef",@$.first_line,3,$1,$2,$3); }
    | error SEMI { yyerrok; }
    | Specifier error SEMI { yyerrok; }
    | Specifier error { yyerrok; }
    ;
ExtDecList : VarDec                                         { $$=nonterminalNode("ExtDecList",@$.first_line,1,$1); }
    | VarDec COMMA ExtDecList                               { $$=nonterminalNode("ExtDecList",@$.first_line,2,$1,$2); }
    ;

/* Specifiers */
Specifier : TYPE                                            { $$=nonterminalNode("Specifier",@$.first_line,1,$1); }
    | StructSpecifier                                       { $$=nonterminalNode("Specifier",@$.first_line,1,$1); }
    ;
StructSpecifier : STRUCT OptTag LC DefList RC               { $$=nonterminalNode("StructSpecifier",@$.first_line,5,$1,$2,$3,$4,$5); }
    | STRUCT Tag                                            { $$=nonterminalNode("StructSpecifier",@$.first_line,2,$1,$2); }
    | STRUCT OptTag LC DefList error RC  { yyerrok; }
    ;
OptTag : ID                                                 { $$=nonterminalNode("OptTag",@$.first_line,1,$1); }
    | /* empty */                                           { $$=NULL; }
    ;
Tag : ID                                                    { $$=nonterminalNode("Tag",@$.first_line,1,$1); }
    ;
/* Declarators */
VarDec : ID                                                 { $$=nonterminalNode("VarDec",@$.first_line,1,$1); }
    | VarDec LB INT RB                                      { $$=nonterminalNode("VarDec",@$.first_line,4,$1,$2,$3,$4); }                      
    | VarDec LB error RB { yyerrok; }

    ;
FunDec : ID LP VarList RP                                   { $$=nonterminalNode("FunDec",@$.first_line,4,$1,$2,$3,$4); }
    | ID LP RP                                              { $$=nonterminalNode("FunDec",@$.first_line,3,$1,$2,$3); }
    | ID LP error RP { yyerrok; }

    ;
VarList : ParamDec COMMA VarList                            { $$=nonterminalNode("VarList",@$.first_line,3,$1,$2,$3); }
    | ParamDec                                              { $$=nonterminalNode("VarList",@$.first_line,1,$1); }
    ;
ParamDec : Specifier VarDec                                 { $$=nonterminalNode("ParamDec",@$.first_line,2,$1,$2); }
    ;

/* Statements */
CompSt : LC DefList StmtList RC                             { $$=nonterminalNode("CompSt",@$.first_line,4,$1,$2,$3,$4); }
    ;
StmtList : Stmt StmtList                                    { $$=nonterminalNode("StmtList",@$.first_line,2,$1,$2); }
    | /* empty */                                           { $$=NULL; }
    ;
Stmt : Exp SEMI                                             { $$=nonterminalNode("Stmt",@$.first_line,2,$1,$2); }
    | CompSt                                                { $$=nonterminalNode("Stmt",@$.first_line,1,$1); }
    | RETURN Exp SEMI                                       { $$=nonterminalNode("Stmt",@$.first_line,3,$1,$2,$3); }
    | IF LP Exp RP Stmt %prec LOWER_THAN_ELSE               { $$=nonterminalNode("Stmt",@$.first_line,5,$1,$2,$3,$4,$5); }
    | IF LP Exp RP Stmt ELSE Stmt                           { $$=nonterminalNode("Stmt",@$.first_line,7,$1,$2,$3,$4,$5,$6,$7); }
    | WHILE LP Exp RP Stmt                                  { $$=nonterminalNode("Stmt",@$.first_line,5,$1,$2,$3,$4,$5); }
    | error SEMI  { yyerrok; }
    | IF LP error RP Stmt %prec LOWER_THAN_ELSE { yyerrok; }
    | IF LP Exp RP error ELSE Stmt { yyerrok; }
    | WHILE LP error RP Stmt { yyerrok; }
    | RETURN Exp error { yyerrok; }
    ;

/* Local Definitions */
DefList : Def DefList                                       { $$=nonterminalNode("DefList",@$.first_line,2,$1,$2); }
    | /* empty */                                           { $$=NULL; }
    ;
Def : Specifier DecList SEMI                                { $$=nonterminalNode("Def",@$.first_line,3,$1,$2,$3); }
    | Specifier error SEMI { yyerrok; }
    ;
DecList : Dec                                               { $$=nonterminalNode("DecList",@$.first_line,1,$1); }
    | Dec COMMA DecList                                     { $$=nonterminalNode("DecList",@$.first_line,3,$1,$2,$3); }
    ;
Dec : VarDec                                                { $$=nonterminalNode("Dec",@$.first_line,1,$1); }
    | VarDec ASSIGNOP Exp                                   { $$=nonterminalNode("Dec",@$.first_line,3,$1,$2,$3); }
    ;

/* Expressions */
Exp : Exp ASSIGNOP Exp                                      { $$=nonterminalNode("Exp",@$.first_line,3,$1,$2,$3); }
    | Exp AND Exp                                           { $$=nonterminalNode("Exp",@$.first_line,3,$1,$2,$3); }
    | Exp OR Exp                                            { $$=nonterminalNode("Exp",@$.first_line,3,$1,$2,$3); }
    | Exp RELOP Exp                                         { $$=nonterminalNode("Exp",@$.first_line,3,$1,$2,$3); }
    | Exp PLUS Exp                                          { $$=nonterminalNode("Exp",@$.first_line,3,$1,$2,$3); }
    | Exp MINUS Exp                                         { $$=nonterminalNode("Exp",@$.first_line,3,$1,$2,$3); }
    | Exp STAR Exp                                          { $$=nonterminalNode("Exp",@$.first_line,3,$1,$2,$3); }
    | Exp DIV Exp                                           { $$=nonterminalNode("Exp",@$.first_line,3,$1,$2,$3); }
    | LP Exp RP                                             { $$=nonterminalNode("Exp",@$.first_line,3,$1,$2,$3); }
    | MINUS Exp                                             { $$=nonterminalNode("Exp",@$.first_line,2,$1,$2); }
    | NOT Exp                                               { $$=nonterminalNode("Exp",@$.first_line,2,$1,$2); }
    | ID LP Args RP                                         { $$=nonterminalNode("Exp",@$.first_line,4,$1,$2,$3,$4); }
    | ID LP RP                                              { $$=nonterminalNode("Exp",@$.first_line,3,$1,$2,$3); }
    | Exp LB Exp RB                                         { $$=nonterminalNode("Exp",@$.first_line,4,$1,$2,$3,$4); }
    | Exp DOT ID                                            { $$=nonterminalNode("Exp",@$.first_line,3,$1,$2,$3); }
    | ID                                                    { $$=nonterminalNode("Exp",@$.first_line,1,$1); }
    | INT                                                   { $$=nonterminalNode("Exp",@$.first_line,1,$1); }
    | FLOAT                                                 { $$=nonterminalNode("Exp",@$.first_line,1,$1); }
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
Args : Exp COMMA Args                                       { $$=nonterminalNode("Args",@$.first_line,3,$1,$2,$3); }
    | Exp                                                   { $$=nonterminalNode("Args",@$.first_line,1,$1); }
    ;

%%
#include "lex.yy.c"
yyerror(char* msg) {
    syntax_errors++;
    if (!strcmp(yytext,"")==0)
        printf("Error type B at Line %d: %s, near \"%s\".\n",yylineno,msg,yytext);
}
