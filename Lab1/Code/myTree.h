#ifndef TREE_NODE
#define TREE_NODE

#include<stdarg.h>
#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>

struct treenode {
    char name[32];
    bool tokenSign;
    int line;
    char val[32];
    int childrenNum;
    struct treenode** children;
};

void print_tree(struct treenode* mynode, int depth);  
struct treenode* token_node(char* name, char* val);  
struct treenode* nonterminal_node(char* name, int line, int node_num, ...);  
#endif