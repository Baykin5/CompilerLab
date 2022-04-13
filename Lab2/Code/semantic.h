#include "myTree.h"
#include "symbolTable.h"


void printErrorInfo(int type,int line,char* message);

void semanticAnalyse(TreeNode root);

/* 对每一个产生式都进行分析 */
void Program(TreeNode node);
void ExtDefList(TreeNode node);
void ExtDef(TreeNode node);
void ExtDecList(TreeNode node,Type type);
Type Specifier(TreeNode node);
Type StructSpecifier(TreeNode node);
char* OptTag(TreeNode node);
char* Tag(TreeNode node);
void VarDecFunc(TreeNode node,Type type,FieldList field);
void VarDecStruct(TreeNode node,Type type,Type struct_type);
bool inStructType(Type struct_type,char* name);
void VarDec(TreeNode node,Type type);
void FunDec(TreeNode node,FieldList field,bool dec_flag);
void VarList(TreeNode node,FieldList field);
void ParamDec(TreeNode node,FieldList field);
void CompSt(TreeNode node,Type type_ret);
void StmtList (TreeNode node,Type type_ret);
void Stmt(TreeNode node,Type type_ret);
void DefListStruct(TreeNode node,Type type_struct);
void DefList(TreeNode node);
void DefStruct(TreeNode node,Type type_struct);
void Def(TreeNode node);
void DecListStruct(TreeNode node,Type type,Type type_struct);
void DecList(TreeNode node,Type type);
void DecStruct(TreeNode node,Type type,Type type_struct);
void Dec(TreeNode node,Type type);
Type Exp(TreeNode node);
FieldList Args(TreeNode node);
char* semanticID(TreeNode node);
int semanticINT(TreeNode node);
float semanticFLOAT(TreeNode node);

bool typeEqual(Type t1,Type t2);
bool valListEqual(FieldList f1,FieldList f2);
bool isLValue(TreeNode node);

void checkFunDec(HashNode symbol_table,char**all_func,int func_cnt);

