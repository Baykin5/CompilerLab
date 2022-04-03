#include<stdio.h>
extern FILE*yyin;
int yylex(void);
int main(int argc, char** argv) {
    if (argc<=1)
        return 1;
    yyin=fopen(argv[1],"r");
    if (!yyin){
        perror(argv[1]);
        return 1;
    }
    yylex();
    return 0;
}

