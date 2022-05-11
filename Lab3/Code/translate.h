#include "semantic.h"
#include "intercode.h"

void translateProgram(TreeNode node,char*out_name);
void translateExtDefList(TreeNode node);
void translateExtDef(TreeNode node);
void translateExtDecList(TreeNode node);
void translateFunDec(TreeNode node);
void translateCompSt(TreeNode node);
void translateDefList(TreeNode node);
void translateDef(TreeNode node);
void translateDecList(TreeNode node);
void translateDec(TreeNode node);
Operand translateVarDec(TreeNode node);
void translateStmtList(TreeNode node);
void translateStmt(TreeNode node);
void translateExp(TreeNode node,Operand place);
ArgNode translateArgs(TreeNode node);
void translateCond(TreeNode node,Operand label_true,Operand label_false);

char* getRelop(TreeNode node);
Operand newTemp();
Operand newLabel();
Operand newAddr();
Operand newConst(int val_int);
Type intType();
int calSize(Type type);
int calOffset(FieldList field,char*name);
bool expIsConst(TreeNode node);
int getExpConst(TreeNode node);
bool expIsID(TreeNode node);
Operand getExpArg(TreeNode node);
Type findStructDomainType(FieldList field,char*name);