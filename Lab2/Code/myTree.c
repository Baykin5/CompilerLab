#include "myTree.h"
void printTree(TreeNode p, int depth) {
    if (p==NULL)
        return ;
    for (int i=0;i<depth;i++)
        printf("  ");
    printf("%s",p->name);
    if (p->token_sign){
        switch (p->data_type)
        {
        case TYPE_ID:case TYPE_TYPE:
            printf(": %s",p->val.val_str);
            break;
        case TYPE_INT:
            printf(": %d",p->val.val_int);
            break;
        case TYPE_FLOAT:
            printf(": %f",p->val.val_float);
            break;
        default:
            break;
        }
    }
    else{
        printf(" (%d)",p->line);
    }
    printf("\n");
    for (int i=0;i<p->children_num;i++){
        printTree(p->children[i],depth+1);
    }
}
TreeNode tokenNode(char* name, enum DATATYPE datatype, char* val) {
    TreeNode p = (TreeNode)malloc(sizeof(struct TreeNode_));
    strcpy(p->name,name);
    p->token_sign=true;
    p->children_num=0;
    p->children=NULL;
    p->data_type=datatype;
    switch (datatype)
    {
    case TYPE_ID:case TYPE_TYPE:
        strcpy(p->val.val_str,val);
        break;
    case TYPE_INT:
        p->val.val_int=atoi(val);
        break;
    case TYPE_FLOAT:
        p->val.val_float=atof(val);
        break;
    default:
        break;
    }
    p->line=-1;
    //printf("Add tokenNode %s,%s\n",p->name,p->val);
    return p;
}
TreeNode nonterminalNode(char* name, int line, int node_num, ...) {
    TreeNode p = (TreeNode)malloc(sizeof(struct TreeNode_));
    strcpy(p->name,name);
    p->token_sign=false;
    p->line=line;
    p->children_num=node_num;
    /* 连接子节点  */
    va_list vaList;
    va_start(vaList,node_num);
    p->children = (TreeNode*)malloc(node_num* sizeof(TreeNode));
    for (int i=0;i<p->children_num;i++){
        p->children[i]=va_arg(vaList,TreeNode);
    }
    va_end(vaList);
    //printf("Add nonterminal_node %s %d %d\n",p->name,p->line,p->children_num);
    return p;
}