#ifndef TREE_NODE
#define TREE_NODE

#include<stdarg.h>
#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>

enum DATATYPE { TYPE_INT, TYPE_FLOAT, TYPE_ID, TYPE_TYPE, TYPE_OTHER };

typedef struct TreeNode_ *TreeNode;

struct TreeNode_ {
    char name[32];
    bool token_sign;
    int line;
    enum DATATYPE data_type;
    union {                  // 存储对应类型的值
        unsigned val_int;
        float val_float;
        char val_str[32];
    } val;
    int children_num;
    TreeNode* children;
};

void printTree(TreeNode mynode, int depth);  
TreeNode tokenNode(char* name, enum DATATYPE datatype, char* val);  
TreeNode nonterminalNode(char* name, int line, int node_num, ...);  
#endif