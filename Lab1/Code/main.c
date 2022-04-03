#include<stdio.h>
#include "myTree.h"
#include "syntax.tab.h"
int lexical_errors = 0;              
int syntax_errors = 0;               
struct treenode *root;      
//extern int yydebug;       
YYSTYPE yylval;                    
int main(int argc, char** argv)
{
    if (argc <= 1) return 1;
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
        print_tree(root, 0);
    }
    return 0;
}

