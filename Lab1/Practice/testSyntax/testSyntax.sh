bison -d testSyntax.y
flex testSyntax.l
gcc testSyntax.tab.c -lfl -ly -o parser
./parser testSyntax.txt