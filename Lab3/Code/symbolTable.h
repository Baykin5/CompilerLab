#ifndef SYMBOL_TABLE
#define SYMBOL_TABLE


#include <stdio.h>
#include <stdbool.h>
#define SymbolTableSize 0x3fff

typedef struct Type_ *Type;
typedef struct FieldList_ *FieldList;
typedef struct HashNode_ *HashNode;

HashNode SymbolTable[SymbolTableSize];
char *all_func[SymbolTableSize];


struct Type_
{
    enum { BASIC,ARRAY,STRUCTURE,FUNCTION,STRUCTURETAG } kind;  //类型的种类:基本类型/数组/结构体
    union
    {
        enum { BASIC_INT,BASIC_FLOAT} basic;      
        struct {
            Type elem;
            int size;
        } array;                      // 数组类型信息包括元素类型与数组大小构成
        FieldList structure;          // 结构体类型信息是一个链表
        struct {
            bool defined_sign;    //函数是否被定义
            int line;    //函数声明的行数 若函数已经被定义则为函数定义的行数
            int para_num;
            FieldList paras;
            Type ret;
        } function;                  
        FieldList structure_tag;
    } u;
};

struct FieldList_
{
    char *name; // 域的名字
    Type type;  // 域的类型
    FieldList tail; // 下一个域

    int no;  // 变量的编号 生成中间代码时使用
    bool is_arg; // 指示变量是否为形参
};

struct HashNode_
{
    FieldList data;
    HashNode next;
};

int hashFunc(char *name);
void insertTable(HashNode* symbol_table, FieldList filed); //向符号表中插入一个符号
void initSymbolTable(HashNode* symbol_table); //初始化符号表
FieldList lookUp(HashNode* symbol_table, char *key); //在符号表中查阅名称为key的符号信息

#endif