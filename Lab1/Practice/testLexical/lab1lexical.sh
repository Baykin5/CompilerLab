flex testLexical.l
gcc main.c lex.yy.c -lfl -o lab1
./lab1 test.txt