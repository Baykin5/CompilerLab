#ifndef INTER_CODE
#define INTER_CODE

#include"symbolTable.h"
#include<stdarg.h>
#include<stdio.h>



typedef struct IRNode_* IRNode;
typedef struct Operand_* Operand;
typedef struct InterCode_* InterCode;
typedef struct ArgNode_* ArgNode;
IRNode InterCodes;
FILE* f_out;

struct IRNode_
{
    InterCode code;
    struct IR_Node *prev;
    struct IR_Node *next;
};

enum GENTYPE{
    /* 1个参数 */
    GEN_LABEL,
    GEN_FUNCTION,
    GEN_GOTO,
    GEN_RETURN,
    GEN_ARG,
    GEN_PARAM,
    GEN_READ,
    GEN_WRITE,
    /* 2个参数 */
    GEN_DEC,
    GEN_ASSIGN,
    GEN_GET_ADDRES,
    GEN_GET_VALUE,
    GEN_ASSIGN_VALUE,
    GEN_CALL,
    /* 3个参数 */
    GEN_ADD,GEN_SUB,GEN_MUL,GEN_DIV,
    /* 4个参数 */
    GEN_IF
    };

struct Operand_ {
    enum OPTYPE{
        OP_VARIABLE,   
        OP_CONSTANT,   // #1,#2...
        OP_ADDRESS,
        
        OP_TEMP,       // t1,t2...
        OP_LABEL,      // label1,label2...
        OP_FUNCTION
    } kind;
    union {
        int var_no;
        int const_val;
        struct{
            int addr_no;
            union 
            {
                struct{
                    Type type;
                    int size;
                } array_info;
                FieldList st_info;
            }addr_info;
            
        } addr;

        int temp_no;
        int label_no;
        char func_name[32];

    } u;
};

struct InterCode_{
    enum IRTYPE{ 
        /* 1个参数 */
        IR_LABEL,           // LABEL f :
        IR_FUNCTION,        // FUNCTION f :
        IR_GOTO,            // GOTO l
        IR_RETURN,          // RETURN a
        IR_ARG,             // ARG a
        IR_PARAM,           // PARAM a
        IR_READ,            // READ a
        IR_WRITE,           // WRITE a
        /* 2个参数 */
        IR_DEC,             // DEC x [a]
        IR_ASSIGN,          // x:= y
        IR_GET_ADDRES,      // x:= &y
        IR_GET_VALUE,       // x:= *y
        IR_ASSIGN_VALUE,    // *x= y
        IR_CALL,            // x := CALL f
        /* 3个参数 */
        IR_ADD,IR_SUB,IR_MUL,IR_DIV, // x := y + z
        /* 4个参数 */
        IR_IF                        // IF x [RELOP] y GOTO z
    } kind;
    union {
        struct { Operand op; } one_op;
        struct { Operand right, left; } two_op;
        struct { Operand result, op1, op2; } three_op;
        struct { Operand left; int right;} dec;
        struct { Operand op1,op2,label;char relop[32]; } if_goto;
    } u;
};

struct ArgNode_{
    Operand* op;
    ArgNode next;
};


void genCode(enum GENTYPE gen_type,int arg_num,...);
Operand genOperand(enum OPTYPE type,int int_val,char* str_val);

void fprintCode(InterCode code);
void fprintCodes();
void fprintOperand(Operand op);
void addCode(InterCode ic);

void printCode(InterCode code);
void printOperand(Operand op);
#endif