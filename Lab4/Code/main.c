#include<stdio.h>

#include "syntax.tab.h"
#include "myTree.h"
#include "symbolTable.h"
#include "semantic.h"
#include "translate.h"

int lexical_errors = 0;              
int syntax_errors = 0;      
int semantic_errors = 0;         
TreeNode root;      
//extern int yydebug;       
YYSTYPE yylval;                    
int main(int argc, char** argv)
{
    if (argc <= 2) return 1;
    FILE* f = fopen(argv[1], "r");
    if (!f)
    {
        perror(argv[1]);
        return 1;
    }
    yyrestart(f);
    //yydebug=1;
    yyparse();
    if (lexical_errors==0 && syntax_errors==0) {
        //printTree(root, 0);
        //printf("START SEMANTIC ANALYSE\n");
        semanticAnalyse(root);
        if (semantic_errors==0){
            //printf("START TRANSLATE\n");
            translateProgram(root,NULL);
            genObjectCode(argv[2]);
        }

    }
    return 0;
}

