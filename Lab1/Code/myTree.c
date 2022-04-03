#include "myTree.h"
void print_tree(struct treenode* p, int depth) {
    if (p==NULL)
        return ;
    for (int i=0;i<depth;i++)
        printf("  ");
    printf("%s",p->name);
    if (p->tokenSign){
        if (strcmp(p->name,"INT")==0)
            printf(": %d",atoi(p->val));
        
        else if (strcmp(p->name,"FLOAT")==0)
            printf(": %f",atof(p->val));

        else if (strcmp(p->name,"ID") ==0 || strcmp(p->name,"TYPE")==0)
            printf(": %s",p->val);
        }
    else{
        printf(" (%d)",p->line);
    }
    printf("\n");
    for (int i=0;i<p->childrenNum;i++){
        print_tree(p->children[i],depth+1);
    }
}
struct treenode* token_node(char* name, char* val) {
    struct treenode* p = (struct treenode*)malloc(sizeof(struct treenode));
    strcpy(p->name,name);
    p->tokenSign=true;
    p->childrenNum=0;
    p->children=NULL;
    if (val!=NULL)
        strcpy(p->val,val);
    p->line=-1;
    //printf("Add token_node %s,%s\n",p->name,p->val);
    return p;
}
struct treenode* nonterminal_node(char* name, int line, int node_num, ...) {
    struct treenode* p = (struct treenode*)malloc(sizeof(struct treenode));
    strcpy(p->name,name);
    p->tokenSign=false;
    p->line=line;
    p->childrenNum=node_num;
    /* 连接子节点  */
    va_list vaList;
    va_start(vaList,node_num);
    p->children = (struct treenode**)malloc(node_num* sizeof(struct treenode*));
    for (int i=0;i<p->childrenNum;i++){
        p->children[i]=va_arg(vaList,struct treenode*);
    }
    va_end(vaList);
    //printf("Add nonterminal_node %s %d %d\n",p->name,p->line,p->childrenNum);
    return p;
}