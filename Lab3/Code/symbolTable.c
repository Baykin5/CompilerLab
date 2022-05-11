#include"symbolTable.h"


int symbol_no_cnt=0;

void initSymbolTable(HashNode* symbol_table){
    for (int i=0;i<SymbolTableSize;i++){
        symbol_table[i]=NULL;
    }
}

void insertTable(HashNode* symbol_table,FieldList field) 
{
    field->no=symbol_no_cnt;
    //printf("v%d:%s\n",symbol_no_cnt,field->name);
    symbol_no_cnt++;
    field->is_arg=false;

    char*key=field->name;
    int index=hashFunc(key);
    HashNode p=(HashNode)malloc(sizeof (struct HashNode_));
    p->data=field;
    p->next=symbol_table[index];
    symbol_table[index]=p;

};

FieldList lookUp(HashNode* symbol_table,char *key)
{
    //printf("lookUp\n");
    int index=hashFunc(key);
    HashNode p=symbol_table[index];
    while(p!=NULL){
        if (strcmp(p->data->name,key)==0)
            //printf("FIND:%s\n",p->data->name);
            return p->data;
        p=p->next;
    }
    return NULL;
}

int hashFunc(char *name)
{

    int size = SymbolTableSize;
    unsigned int val = 0, i;
    for (; *name; ++name)
    {
        val = (val << 2) + *name;
        if (i = val & ~size)
            val = (val ^ (i >> 12)) & size;
    }
    return val;
}