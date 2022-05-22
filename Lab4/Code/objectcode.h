#ifndef OBJECT_CODE
#define OBJECT_CODE

#include"intercode.h"
#include"basicblock.h"

#define REG_NUM 32
FILE* f_out_oc;

typedef struct Register_* Register;
typedef struct Variable_* Variable;
typedef struct RegisterList_* RegisterList;
typedef struct VariableList_* VariableList;
typedef struct NextUseList_* NextUseList;

Register REG[REG_NUM];
VariableList localVarList; // 存有所有变量
int param_cnt;             // 函数的形式参数数量
int local_offset;      // 基于fp的函数偏移量 0为fp 4为ret 以下为超过四个的形参，以上为各种临时变量
int arg_num;               // 调用函数时,函数的参数数量
int framesize;

enum VarUseType{ LEFT,RIGHT};

struct Register_ { // 寄存器描述符
    int reg_no;
    char* name;              // 寄存器的名字
    enum {FREE,BUSY} state;  // 寄存器状态
    VariableList vars;  // 该寄存器存放了哪些变量
};

struct Variable_ {  //  地址描述符
    Operand op;   // 变量参数
    Register reg;
    RegisterList regs;  // 该变量存放于哪些寄存器
    NextUseList next_use;
    int offset;   // 在函数活动记录里的偏移量
};

struct VariableList_{
    Variable var;
    VariableList next;
};

struct RegisterList_{
    Register reg;
    RegisterList next;
};

struct NextUseList_{  // 记录变量下一次使用到的行数
    int next_no;
    NextUseList next;
};


void genObjectCode(char*out_name);   // 打印目标代码
void genObjectCodeFB(FunctionBlock fb,FILE*out);   // 逐函数块打印目标代码
void genObjectCodeBB(BasicBlock bb,FILE*out);  // 逐基本块打印目标代码

void refreshActiveInfo(BasicBlock bb);
void refreshActiveInfoIC(InterCode ic);
void addNextUse(Operand op,int ic_no);
void delNextUseIC(InterCode ic);
void delNextUse(Operand op);  // 处理完一条语句后 删除该条使用信息

void initVarListFB(FunctionBlock fb,FILE*out);
void initVarListBB(BasicBlock bb,FILE* out);
void initVarListIC(InterCode ic,FILE* out);
void releaseVarList();

Variable opToVar(Operand op);
void insertVar(Variable var);
Variable findVar(Operand op);
bool opEqual(Operand op1,Operand op2);

Register ensure(Operand op,FILE* out,enum VarUseType var_type);
void freeReg(Register reg);
Register allocate(Operand op,FILE* out);
void spillAllVar(FILE* out);
void spill(Variable var,FILE* out);

void pushVarToStack(FILE* out);
void loadVarFromStack(FILE* out);

void fprintLABEL(InterCode ic,FILE* out);
void fprintFUNCTION(InterCode ic,FILE* out);
void fprintGOTO(InterCode ic,FILE* out);
void fprintRETURN(InterCode ic,FILE* out);
void fprintARG(InterCode ic,FILE* out);
void fprintREAD(InterCode ic,FILE* out);
void fprintWRITE(InterCode ic,FILE* out);

void fprintASSIGN(InterCode ic,FILE* out);
void fprintGET_ADDRES(InterCode ic,FILE* out);
void fprintGET_VALUE(InterCode ic,FILE* out);
void fprintASSIGN_VALUE(InterCode ic,FILE* out);
void fprintCALL(InterCode ic,FILE* out);

void fprintADD(InterCode ic,FILE* out);
void fprintSUB(InterCode ic,FILE* out);
void fprintMUL(InterCode ic,FILE* out);
void fprintDIV(InterCode ic,FILE* out);
void fprintIF(InterCode ic,FILE* out);


bool notNeed(Operand op,int ic_no);

void printLocalVarList();

#endif