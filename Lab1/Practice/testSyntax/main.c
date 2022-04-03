#include<stdio.h>
#include "lex.yy.c"
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
    yyparse();
    return 0;
}

/*
#include<stdio.h>
extern FILE* yyin;
int yylex(void);
int yyrestart(FILE*);
int yyparse(void);
int main(int argc, char** argv) {
    if (argc<=1)
        return 1;
    FILE* yyin=fopen(argv[1],"r");
    if (!yyin){
        perror(argv[1]);
        return 1;
    }
    yyrestart(yyin);
    yyparse();
    return 0;
}

*/