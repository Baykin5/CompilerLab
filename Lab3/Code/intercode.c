#include "intercode.h"
#include <assert.h>

const bool DEBUG4 = false;

void fprintCodes(){
    if (DEBUG4) printf("fprintCodes\n");
    IRNode p=InterCodes;
    while(p!=NULL){
        fprintCode(p->code);
        p=p->next;
    }
}

void fprintCode(InterCode code){
    if (DEBUG4) printCode(code);
    switch (code->kind)
    {
    case IR_LABEL:
        fprintf(f_out,"LABEL ");
        fprintOperand(code->u.one_op.op);
        fprintf(f_out," :");
        break;
    case IR_FUNCTION:
        fprintf(f_out,"FUNCTION ");
        fprintOperand(code->u.one_op.op);
        fprintf(f_out," :");
        break;
    case IR_GOTO:
        fprintf(f_out,"GOTO ");
        fprintOperand(code->u.one_op.op);
        break;
    case IR_RETURN:
        fprintf(f_out,"RETURN ");
        fprintOperand(code->u.one_op.op);
        break;
    case IR_ARG:
        fprintf(f_out,"ARG ");
        fprintOperand(code->u.one_op.op);
        break;
    case IR_PARAM:
        fprintf(f_out,"PARAM ");
        fprintOperand(code->u.one_op.op);
        break;
    case IR_READ:
        fprintf(f_out,"READ ");
        fprintOperand(code->u.one_op.op);
        break;
    case IR_WRITE:
        fprintf(f_out,"WRITE ");
        fprintOperand(code->u.one_op.op);
        break;

    case IR_DEC:
        fprintf(f_out,"DEC ");
        fprintOperand(code->u.dec.left);
        fprintf(f_out," %d",code->u.dec.right);
        break;
    case IR_ASSIGN:
        fprintOperand(code->u.two_op.left);
        fprintf(f_out," := ");
        fprintOperand(code->u.two_op.right);
        break;
    case IR_GET_ADDRES:
        fprintOperand(code->u.two_op.left);
        fprintf(f_out," := &");
        fprintOperand(code->u.two_op.right);
        break;
    case IR_GET_VALUE:
        fprintOperand(code->u.two_op.left);
        fprintf(f_out," := *");
        fprintOperand(code->u.two_op.right);
        break;
    case IR_ASSIGN_VALUE:
        fprintf(f_out,"*");
        fprintOperand(code->u.two_op.left);
        fprintf(f_out," := ");
        fprintOperand(code->u.two_op.right);
        break;
    case IR_CALL:
        fprintOperand(code->u.two_op.left);
        fprintf(f_out," := CALL ");
        fprintOperand(code->u.two_op.right);
        break;   

    case IR_ADD:
        fprintOperand(code->u.three_op.result);
        fprintf(f_out," := ");
        fprintOperand(code->u.three_op.op1);
        fprintf(f_out," + ");
        fprintOperand(code->u.three_op.op2);
        break;
    case IR_SUB:
        fprintOperand(code->u.three_op.result);
        fprintf(f_out," := ");
        fprintOperand(code->u.three_op.op1);
        fprintf(f_out," - ");
        fprintOperand(code->u.three_op.op2);
        break;
    case IR_MUL:
        fprintOperand(code->u.three_op.result);
        fprintf(f_out," := ");
        fprintOperand(code->u.three_op.op1);
        fprintf(f_out," * ");
        fprintOperand(code->u.three_op.op2);
        break;
    case IR_DIV:
        fprintOperand(code->u.three_op.result);
        fprintf(f_out," := ");
        fprintOperand(code->u.three_op.op1);
        fprintf(f_out," / ");
        fprintOperand(code->u.three_op.op2);
        break;
    case IR_IF:
        fprintf(f_out,"IF ");
        fprintOperand(code->u.if_goto.op1);
        fprintf(f_out," %s ",code->u.if_goto.relop);
        fprintOperand(code->u.if_goto.op2);
        fprintf(f_out," GOTO ");
        fprintOperand(code->u.if_goto.label);
    default:
        break;
    }
    fprintf(f_out,"\n");
}

void fprintOperand(Operand op){
    switch (op->kind)
    {
    case OP_VARIABLE:
        fprintf(f_out,"v%d",op->u.var_no);
        break;
    case OP_CONSTANT:
        fprintf(f_out,"#%d",op->u.const_val);
        break;
    case OP_ADDRESS:
        fprintf(f_out,"addr%d",op->u.addr.addr_no);
        break;
    case OP_TEMP:
        fprintf(f_out,"t%d",op->u.temp_no);
        break;
    case OP_LABEL:
        fprintf(f_out,"label%d",op->u.label_no);
        break;
    case OP_FUNCTION:
        fprintf(f_out,"%s",op->u.func_name);
        break;

    default:
        break;
    }
}
void addCode(InterCode ic){
    IRNode ir=(IRNode)malloc(sizeof(struct IRNode_));
    ir->code=ic;
    ir->next=NULL;
    if (InterCodes==NULL){
        InterCodes=ir;
        ir->prev=NULL;
    }
    else{
        IRNode p=InterCodes;
        while(p->next!=NULL)
            p=p->next;
        p->next=ir;
        ir->prev=p;
    }
}

void genCode(enum GENTYPE gen_type,int arg_num,...){
    va_list vaList;
    va_start(vaList,arg_num);
    InterCode ic=(InterCode)malloc(sizeof(struct InterCode_));
    switch (gen_type)
    {
    case GEN_LABEL:{
        /* LABEL LABEL */
        Operand label=va_arg(vaList,Operand);
        ic->kind=IR_LABEL;
        assert(label->kind==OP_LABEL);
        ic->u.one_op.op=label;
        break;}
    case GEN_FUNCTION:{
        /* FUNCTION FUC_NAME */
        ic->kind=IR_FUNCTION;
        char* func_name=va_arg(vaList,char*);
        ic->u.one_op.op=genOperand(OP_FUNCTION,-1,func_name);
        break;}
    case GEN_GOTO:{
        /* GOTO LABEL */
        ic->kind=IR_GOTO;
        Operand label=va_arg(vaList,Operand);
        assert(label->kind==OP_LABEL);
        ic->u.one_op.op=label;
        break;}
    case GEN_RETURN:{
        /* RETURN OPERAND */  
        ic->kind=IR_RETURN;
        Operand op=va_arg(vaList,Operand);
        ic->u.one_op.op=op;
        break;}
    case GEN_ARG:{
        /* ARG OP_TYPE INT */
        ic->kind=IR_ARG;
        Operand op=va_arg(vaList,Operand);
        ic->u.one_op.op=op;
        break;}
    case GEN_PARAM:{
        /* PARAM OPRAND*/
        ic->kind=IR_PARAM;
        Operand op=va_arg(vaList,Operand);
        ic->u.one_op.op=op;
        break;}
    case GEN_READ:{
        /* READ OPRAND */
        ic->kind=IR_READ;
        Operand op=va_arg(vaList,Operand);
        ic->u.one_op.op=op;
        break;}
    case GEN_WRITE:{
        /* WRITE OPRAND */
        ic->kind=IR_WRITE;
        Operand op=va_arg(vaList,Operand);
        ic->u.one_op.op=op;
        break;}
    
    case GEN_DEC:{
        /* DEC VAR_INT INT */
        ic->kind=IR_DEC;
        int var_int=va_arg(vaList,int);
        int val_int=va_arg(vaList,int);
        ic->u.dec.left=genOperand(OP_VARIABLE,var_int,NULL);
        ic->u.dec.right=val_int;
        break;}
    case GEN_ASSIGN:{
        /* ASSIGN Op1 Op2 */
        ic->kind=IR_ASSIGN;
        Operand op1=va_arg(vaList,Operand);
        Operand op2=va_arg(vaList,Operand);
        ic->u.two_op.left=op1;
        ic->u.two_op.right=op2;
        break;}
    case GEN_GET_ADDRES:{
        ic->kind=IR_GET_ADDRES;
        Operand op1=va_arg(vaList,Operand);
        Operand op2=va_arg(vaList,Operand);
        ic->u.two_op.left=op1;
        ic->u.two_op.right=op2;
        break;}
    case GEN_GET_VALUE:{
        ic->kind=IR_GET_VALUE;
        Operand op1=va_arg(vaList,Operand);
        Operand op2=va_arg(vaList,Operand);
        ic->u.two_op.left=op1;
        ic->u.two_op.right=op2;
        break;}
    case GEN_ASSIGN_VALUE:{
        ic->kind=IR_ASSIGN_VALUE;
        Operand op1=va_arg(vaList,Operand);
        Operand op2=va_arg(vaList,Operand);
        ic->u.two_op.left=op1;
        ic->u.two_op.right=op2;
        break;}
    case GEN_CALL:{
        ic->kind=IR_CALL;
        Operand op1=va_arg(vaList,Operand);
        Operand op2=genOperand(OP_FUNCTION,-1,va_arg(vaList,char*));
        ic->u.two_op.left=op1;
        ic->u.two_op.right=op2;
        break;}
    case GEN_ADD:{
        /* */
        ic->kind=IR_ADD;
        Operand result=va_arg(vaList,Operand);
        Operand op1=va_arg(vaList,Operand);
        Operand op2=va_arg(vaList,Operand);
        ic->u.three_op.result=result;
        ic->u.three_op.op1=op1;
        ic->u.three_op.op2=op2;
        break;}
    case GEN_SUB:{
        /* */
        ic->kind=IR_SUB;
        Operand result=va_arg(vaList,Operand);
        Operand op1=va_arg(vaList,Operand);
        Operand op2=va_arg(vaList,Operand);
        ic->u.three_op.result=result;
        ic->u.three_op.op1=op1;
        ic->u.three_op.op2=op2;
        break;}
    case GEN_MUL:{
        ic->kind=IR_MUL;
        Operand result=va_arg(vaList,Operand);
        Operand op1=va_arg(vaList,Operand);
        Operand op2=va_arg(vaList,Operand);
        ic->u.three_op.result=result;
        ic->u.three_op.op1=op1;
        ic->u.three_op.op2=op2;
        break;}
    case GEN_DIV:{
        ic->kind=IR_DIV;
        Operand result=va_arg(vaList,Operand);
        Operand op1=va_arg(vaList,Operand);
        Operand op2=va_arg(vaList,Operand);
        ic->u.three_op.result=result;
        ic->u.three_op.op1=op1;
        ic->u.three_op.op2=op2;
        break;}
    
    case GEN_IF:{
        ic->kind=IR_IF;
        Operand t1=va_arg(vaList,Operand);
        char* relop=va_arg(vaList,char*);
        Operand t2=va_arg(vaList,Operand);
        Operand label_true=va_arg(vaList,Operand);
        ic->u.if_goto.op1=t1;
        ic->u.if_goto.op2=t2;
        ic->u.if_goto.label=label_true;
        strcpy(ic->u.if_goto.relop,relop);
        break;}
    default:
        break;
    }
    addCode(ic);
    va_end(vaList);
}

Operand genOperand(enum OPTYPE type,int val_int,char* val_str){
    Operand op=(Operand)malloc(sizeof(struct Operand_));
    op->kind=type;
    switch (type)
    {
    case OP_CONSTANT:
        op->u.const_val=val_int;
        break;
    case OP_ADDRESS:
        op->u.addr.addr_no=val_int;
        break;
    case OP_TEMP:
        op->u.temp_no=val_int;
        break;
    case OP_LABEL:
        op->u.label_no=val_int;
        break;
    case OP_VARIABLE:
        op->u.var_no=val_int;
        break;
    case OP_FUNCTION:
        strcpy(op->u.func_name,val_str);
        break;

    default:
        break;
    }
    return op;
}



void printCode(InterCode code){
    printf("%d\n",code->kind);
    switch (code->kind)
    {
    case IR_LABEL:
        printf( "LABEL ");
        printOperand(code->u.one_op.op);
        printf( " :");
        break;
    case IR_FUNCTION:
        printf( "FUNCTION ");
        printOperand(code->u.one_op.op);
        printf(" :");
        break;
    case IR_GOTO:
        printf("GOTO ");
        printOperand(code->u.one_op.op);
        break;
    case IR_RETURN:
        printf("RETURN ");
        printOperand(code->u.one_op.op);
        break;
    case IR_ARG:
        printf("ARG ");
        printOperand(code->u.one_op.op);
        break;
    case IR_PARAM:
        printf("PARAM ");
        printOperand(code->u.one_op.op);
        break;
    case IR_READ:
        printf("READ ");
        printOperand(code->u.one_op.op);
        break;
    case IR_WRITE:
        printf("WRITE ");
        printOperand(code->u.one_op.op);
        break;

    case IR_DEC:
        printf("DEC ");
        printOperand(code->u.dec.left);
        printf(" %d",code->u.dec.right);
        break;
    case IR_ASSIGN:
        printOperand(code->u.two_op.left);
        printf(" := ");
        printOperand(code->u.two_op.right);
        break;
    case IR_GET_ADDRES:
        printOperand(code->u.two_op.left);
        printf(" := &");
        printOperand(code->u.two_op.right);
        break;
    case IR_GET_VALUE:
        printOperand(code->u.two_op.left);
        printf(" := *");
        printOperand(code->u.two_op.right);
        break;
    case IR_ASSIGN_VALUE:
        printf("*");
        printOperand(code->u.two_op.left);
        printf(" := ");
        printOperand(code->u.two_op.right);
        break;
    case IR_CALL:
        printOperand(code->u.two_op.left);
        printf(" := CALL ");
        printOperand(code->u.two_op.right);
        break;   

    case IR_ADD:
        printOperand(code->u.three_op.result);
        printf(" := ");
        printOperand(code->u.three_op.op1);
        printf(" + ");
        printOperand(code->u.three_op.op2);
        break;
    case IR_SUB:
        printOperand(code->u.three_op.result);
        printf(" := ");
        printOperand(code->u.three_op.op1);
        printf(" - ");
        printOperand(code->u.three_op.op2);
        break;
    case IR_MUL:
        printOperand(code->u.three_op.result);
        printf(" := ");
        printOperand(code->u.three_op.op1);
        printf(" * ");
        printOperand(code->u.three_op.op2);
        break;
    case IR_DIV:
        printOperand(code->u.three_op.result);
        printf(" := ");
        printOperand(code->u.three_op.op1);
        printf(" / ");
        printOperand(code->u.three_op.op2);
        break;
    case IR_IF:
        printf("IF ");
        printOperand(code->u.if_goto.op1);
        printf(" %s ",code->u.if_goto.relop);
        printOperand(code->u.if_goto.op2);
        printf(" GOTO ");
        printOperand(code->u.if_goto.label);
    default:
        break;
    }
    printf("\n");
}
void printOperand(Operand op){
    assert(op!=NULL);
    switch (op->kind)
    {
    case OP_VARIABLE:
        printf("v%d",op->u.var_no);
        break;
    case OP_CONSTANT:
        printf("#%d",op->u.const_val);
        break;
    case OP_ADDRESS:
        printf("addr%d",op->u.addr.addr_no);
        break;
    case OP_TEMP:
        printf("t%d",op->u.temp_no);
        break;
    case OP_LABEL:
        printf("label%d",op->u.label_no);
        break;
    case OP_FUNCTION:
        printf("%s",op->u.func_name);
        break;

    default:
        break;
    }
}