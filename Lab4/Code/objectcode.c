#include"objectcode.h"
#include"assert.h"
extern IRNode InterCodes;
extern BasicBlockList BBList;

int max(int a,int b){  if (a>b)    return a;   else    return b;  }

const bool DEBUG6=true;

char* REG_NAME[32]={"0","at","v0","v1","a0","a1","a2","a3",
                    "t0","t1","t2","t3","t4","t5","t6","t7",
                    "s0","s1","s2","s3","s4","s5","s6","s7",
                    "t8","t9","k0","k1","gp","sp","fp","ra"};

void genObjectCode(char* out_name){
    IRNode p=InterCodes;
    setUpBasicBlock();  // 将基本块和函数基本块建立好
    
    f_out_oc=fopen(out_name,"w");
    assert(f_out_oc!=NULL);    
    initReg();  // 初始化寄存器
    fprintInitInfo(f_out_oc);  // 打印read,write函数等初始信息

    for (int i=0;i<total_fb_num;i++){
        FunctionBlock fb=FBArray[i]->func_block;
        initVarListFB(fb,f_out_oc);   // 初始化函数块中的变量列表
        genObjectCodeFB(fb,f_out_oc);   // 逐函数块打印目标代码
        releaseVarList();               // 处理完一个函数块要释放当前的变量列表
    }
    fclose(f_out_oc);
}

void genObjectCodeFB(FunctionBlock fb,FILE*out){
    if (DEBUG6) printf("\ngenObjectCode for FunctionBlock %d\n",fb->fb_no);
    for (int i=fb->head_bb_no;i<=fb->last_bb_no;i++){  // 逐基本块打印目标代码
        genObjectCodeBB(BBArray[i]->basic_block,out);     
    }
}

void genObjectCodeBB(BasicBlock bb,FILE* out){   
    if (DEBUG6) printf("\ngenObjectCode for BasicBlock %d\n",bb->block_no);
    if (DEBUG6) printLocalVarList();
    refreshActiveInfo(bb);                         // 对每个基本块内语句，更新活跃状态(下次使用的信息)
    IRNode intercodes=bb->head;
    while(intercodes!=bb->last->next){
        // 对于ARG语句，由于中间代码传入的是反向的，因此这里需要先反向扫描四个参数，再将剩下的参数压入栈
        if (intercodes->code->kind==IR_ARG){
            IRNode temp=intercodes;
            int temp_cnt=0;
            while(temp->next->code->kind==IR_ARG){  // 到达传入的第一个参数
                temp=temp->next;
            }
            IRNode next_ic=temp->next;
            while(temp!=intercodes->prev && temp_cnt < 4){     // 扫描4个参数
                fprintObjectCode(temp->code,out);
                temp=temp->prev;
                temp_cnt++;
            }
            if (temp_cnt==4)
                while(intercodes!=temp->next){      // 扫描剩下的参数
                    fprintObjectCode(intercodes->code,out);
                    intercodes=intercodes->next;
                }
            intercodes=next_ic;
        }
        else{
            fprintObjectCode(intercodes->code,out);  // 逐语句打印目标代码
            intercodes=intercodes->next;
        }
    }
     // TODO: 每个基本块代码输出完后将所有修改过的变量写回内存 （当一个变量的next_use变为NULL时进行处理)
     spillAllVar(out);
}

void spillAllVar(FILE* out){  
    if (DEBUG6) printf("spillAllVar\n");
    VariableList varlist=localVarList;
    while(varlist!=NULL){
        Variable var=varlist->var;
        if (var->reg!=NULL){
            spill(var,out);
        }
        varlist = varlist->next;
    }
}

void refreshActiveInfo(BasicBlock bb){
    // 从后往前扫描每个语句
    IRNode temp=bb->last;
    while(temp!=bb->head->prev){
        refreshActiveInfoIC(temp->code);
        temp=temp->prev;
    }
    //if (DEBUG6) printLocalVarList();
}

void refreshActiveInfoIC(InterCode ic){
   switch (ic->kind)
    {
    case IR_LABEL:
    case IR_FUNCTION:
    case IR_GOTO:
    case IR_READ:
        break;
    case IR_RETURN:
    case IR_ARG:
    case IR_WRITE:
        addNextUse(ic->u.one_op.op,ic->ic_no);
        break;
    case IR_DEC:
        break;
    case IR_ASSIGN:
    case IR_GET_ADDRES:
    case IR_GET_VALUE:
        addNextUse(ic->u.two_op.right,ic->ic_no);
        break;
    case IR_ASSIGN_VALUE:
        addNextUse(ic->u.two_op.left,ic->ic_no);
        addNextUse(ic->u.two_op.right,ic->ic_no);
        break;
    case IR_CALL:
        break;
    case IR_ADD:
    case IR_SUB:
    case IR_MUL:
    case IR_DIV:
        addNextUse(ic->u.three_op.op1,ic->ic_no);
        addNextUse(ic->u.three_op.op2,ic->ic_no);
        break;

    case IR_IF:
        addNextUse(ic->u.if_goto.op1,ic->ic_no);
        addNextUse(ic->u.if_goto.op2,ic->ic_no);
        break;
    default:
        break;
    } 
}

void addNextUse(Operand op,int ic_no){
    if (op->kind == OP_CONSTANT){
        return ; // 一个常量只会使用一次 无需存储下次使用信息
    }
    Variable var=findVar(op);
    assert(var!=NULL);
    NextUseList next_use=(NextUseList)malloc(sizeof(struct NextUseList_));
    next_use->next_no=ic_no;
    next_use->next=NULL;
    if (var->next_use==NULL)
        var->next_use=next_use;
    else{
        next_use->next=var->next_use;
        var->next_use=next_use;
    }
}

void printFBInfo(FunctionBlock fb){
    printf("This is FunctionBlock %d Info:\n",fb->fb_no);
    printf("param_cnt: %d\n",param_cnt);
    printf("local_offset: %d\n",local_offset);
    printf("frame_size: %d\n",fb->frame_size);
}


void initVarListFB(FunctionBlock fb,FILE*out){
    if (DEBUG6) printf("initVarListFB\n");
    param_cnt = 0;
    local_offset = 8;
    arg_num = 0;
    framesize = 0;

    for (int i=fb->head_bb_no;i<=fb->last_bb_no;i++){
        initVarListBB(BBArray[i]->basic_block,out);
    }
    fb->frame_size=local_offset; //+ max(0,(param_cnt - 4)) * 4;
    framesize=fb->frame_size;
    if (DEBUG6) printFBInfo(fb);
}



void initVarListBB(BasicBlock bb,FILE* out){
    if (DEBUG6) printf("initVarListBB\n");
    IRNode intercodes=bb->head;
    while(intercodes!=bb->last->next){
        initVarListIC(intercodes->code,out);
        intercodes=intercodes->next;
    }
}

void initVarListIC(InterCode ic,FILE*out){
    //if (DEBUG6) {printf("initVarListIC for intercode %d :",ic->ic_no); printCode(ic);}
    switch (ic->kind)
    {
    case IR_LABEL:
    case IR_FUNCTION:
    case IR_GOTO:
        break;
    case IR_PARAM:{   // TODO:
        Variable var=(Variable)malloc(sizeof(struct Variable_));
        local_offset += 4;
        if (param_cnt<4){
            int reg_no=param_cnt + 4;
            var->reg=REG[reg_no];
            var->offset = local_offset;
        }
        else{
            var->reg=NULL;
            var->offset = -(param_cnt - 3) * 4;  // 超过四个的形参 存放在fp下方
        }
        var->op=ic->u.one_op.op;
        param_cnt++;
        insertVar(var);
        break;
    }
    case IR_RETURN:
    case IR_ARG:
    case IR_READ:
    case IR_WRITE:
        insertVar(opToVar(ic->u.one_op.op));
        break;
    case IR_DEC:{  // TODO:
        Variable var=(Variable)malloc(sizeof(struct Variable_));
        int size=ic->u.dec.right;
        local_offset += size;
        var->offset = local_offset;
        var->reg=NULL;
        var->op=ic->u.dec.left;
        var->next_use=NULL;
        insertVar(var);
        break;
    }
    case IR_ASSIGN:
    case IR_GET_ADDRES:
    case IR_GET_VALUE:
    case IR_ASSIGN_VALUE:
        insertVar(opToVar(ic->u.two_op.left));
        insertVar(opToVar(ic->u.two_op.right));
        break;
    case IR_CALL:
        insertVar(opToVar(ic->u.two_op.left));
        break;
    case IR_ADD:
    case IR_SUB:
    case IR_MUL:
    case IR_DIV:
        insertVar(opToVar(ic->u.three_op.op1));
        insertVar(opToVar(ic->u.three_op.op2));
        insertVar(opToVar(ic->u.three_op.result));
        break;

    case IR_IF:
        insertVar(opToVar(ic->u.if_goto.op1));
        insertVar(opToVar(ic->u.if_goto.op2));
        break;
    default:
        break;
    } 
}

void releaseVarList(){
    if (DEBUG6) printf("releaseVarList\n");
    VariableList varlist = localVarList;
    while(varlist!=NULL){
        VariableList temp=varlist;
        varlist = varlist->next;
        free(temp->var);
        free(temp);
    }
    localVarList = NULL;
}

Variable opToVar(Operand op){
    if (op->kind==OP_CONSTANT)   // 常数参数不需要加载
        return NULL;
    if (findVar(op)!=NULL) // 检查是否重复
        return NULL;
    local_offset += 4;
    Variable var=(Variable)malloc(sizeof(struct Variable_));
    var->op=op;
    var->reg==NULL;
    var->next_use=NULL;
    var->offset = local_offset;

    var->regs=NULL;
    assert(var->regs==NULL);
    return var;
}

void insertVar(Variable var){  // 将Var插入localVarList
    if (var==NULL)
        return ;
    VariableList varlist=(VariableList)malloc(sizeof(struct VariableList_));
    varlist->var=var;
    varlist->next=NULL;
    if (localVarList==NULL){
        localVarList=varlist;
    }
    else{
        VariableList temp=localVarList;
        while(temp->next!=NULL){
            temp=temp->next;
        }
        temp->next=varlist;
    }
}


Variable findVar(Operand op){  // 根据Operand返回对应的Variable
    //if (DEBUG6) { printf("findVar for ");  printOperand(op); printf("\n");}
    VariableList temp=localVarList;
    while(temp!=NULL && !opEqual(temp->var->op,op)){
        temp=temp->next;
    }
    if (temp==NULL)
        return NULL;
    else 
        return temp->var;
}

bool opEqual(Operand op1,Operand op2){
    //if (DEBUG6) printf("opEqual\n");
    if (op1->kind!=op2->kind)
        return false;
    else{
        switch (op1->kind)
        {
        case OP_VARIABLE:
            return op1->u.var_no==op2->u.var_no;
        case OP_ADDRESS:
            return op1->u.addr.addr_no==op2->u.addr.addr_no;
        case OP_CONSTANT:
            return op1->u.const_val==op2->u.const_val;
        case OP_FUNCTION:
            return strcmp(op1->u.func_name,op2->u.func_name);
        case OP_LABEL:
            return op1->u.label_no==op2->u.label_no;
        case OP_TEMP:
            return op1->u.temp_no==op2->u.temp_no;        
        default:
            break;
        }
    }
}


void fprintObjectCode(InterCode ic,FILE* out){
    if (DEBUG6) { printf("\ngenObjectCode for InterCode %d:",ic->ic_no);
                printCode(ic);
                printLocalVarList(); }
    //printLocalVarList();
    switch (ic->kind)
    {
    case IR_LABEL:
        fprintLABEL(ic,out);
        break;
    case IR_FUNCTION:
        fprintFUNCTION(ic,out);
        break;
    case IR_GOTO:
        fprintGOTO(ic,out);
        break;
    case IR_RETURN:
        fprintRETURN(ic,out);
        break;
    case IR_ARG:
        fprintARG(ic,out);
        break;
    case IR_PARAM:
        break;
    case IR_READ:
        fprintREAD(ic,out);
        break;
    case IR_WRITE:
        fprintWRITE(ic,out);
        break;
    case IR_DEC:
        break;
    case IR_ASSIGN:
        fprintASSIGN(ic,out);
        break;
    case IR_GET_ADDRES:
        fprintGET_ADDRES(ic,out);
        break;
    case IR_GET_VALUE:
        fprintGET_VALUE(ic,out);
        break;
    case IR_ASSIGN_VALUE:
        fprintASSIGN_VALUE(ic,out);
        break;
    case IR_CALL:
        fprintCALL(ic,out);
        break;
    case IR_ADD:
        fprintADD(ic,out);
        break;
    case IR_SUB:
        fprintSUB(ic,out);
        break;
    case IR_MUL:
        fprintMUL(ic,out);
        break;
    case IR_DIV:
        fprintDIV(ic,out);
        break;
    case IR_IF:
        fprintIF(ic,out);
        break;
    default:
        break;
    }
    delNextUseIC(ic);
    //if (DEBUG6) printLocalVarList();
}

void delNextUseIC(InterCode ic){
   switch (ic->kind)
    {
    case IR_LABEL:
    case IR_FUNCTION:
    case IR_GOTO:
    case IR_READ:
        break;
    case IR_RETURN:
    case IR_ARG:
    case IR_WRITE:
        delNextUse(ic->u.one_op.op);
        break;
    case IR_DEC:
        break;
    case IR_ASSIGN:
    case IR_GET_ADDRES:
    case IR_GET_VALUE:
    case IR_ASSIGN_VALUE:
        delNextUse(ic->u.two_op.right);
        break;
    case IR_CALL:
        break;
    case IR_ADD:
    case IR_SUB:
    case IR_MUL:
    case IR_DIV:
        delNextUse(ic->u.three_op.op1);
        delNextUse(ic->u.three_op.op2);
        break;
    case IR_IF:
        delNextUse(ic->u.if_goto.op1);
        delNextUse(ic->u.if_goto.op2);
        break;
    default:
        break;
    } 
}

void delNextUse(Operand op){
    if (op->kind == OP_CONSTANT)
        return ;
    Variable var = findVar(op);
    assert(var && var->next_use);
    NextUseList temp=var->next_use;
    var->next_use=var->next_use->next;
    free(temp);
}


void initReg(){  // 初始化各寄存器信息
    for (int i=0;i<REG_NUM;i++){
        Register reg=(Register)malloc(sizeof(struct Register_));
        reg->reg_no=i;
        reg->name=REG_NAME[i];
        reg->state=FREE;
        reg->vars=NULL;
        REG[i]=reg;
        //printf("REG %d: $%s\n",reg->reg_no,reg->name);
    }
}
 
void fprintInitInfo(FILE* out){
    // head
    fprintf(out, ".data\n");
    fprintf(out, "_prompt: .asciiz \"Enter an integer:\"\n");
    fprintf(out, "_ret: .asciiz \"\\n\"\n");
    fprintf(out, ".globl main\n");
    // read
    fprintf(out, ".text\n");
    fprintf(out, "read:\n");
    fprintf(out, "  li $v0, 4\n");
    fprintf(out, "  la $a0, _prompt\n");
    fprintf(out, "  syscall\n");
    fprintf(out, "  li $v0, 5\n");
    fprintf(out, "  syscall\n");
    fprintf(out, "  jr $ra\n");
    fprintf(out, "\n");
    // write
    fprintf(out, "write:\n");
    fprintf(out, "  li $v0, 1\n");
    fprintf(out, "  syscall\n");
    fprintf(out, "  li $v0, 4\n");
    fprintf(out, "  la $a0, _ret\n");
    fprintf(out, "  syscall\n");
    fprintf(out, "  move $v0, $0\n");
    fprintf(out, "  jr $ra\n");
    fprintf(out, "\n");
}
void fprintBasicBlock(BasicBlock bb,FILE* out){
    IRNode p=bb->head;
    while(p!=bb->last){
        fprintObjectCode(p->code,out);
        p=p->next;
    }
    fprintObjectCode(bb->last,out);
}



void fprintREAD(InterCode ic, FILE* out){
    // 保存返回地址
    fprintf(out, "  addi $sp, $sp, -4\n");
    fprintf(out, "  sw $ra, 0($sp)\n");
    // 调用read函数
    fprintf(out, "  jal read\n");
    // 恢复栈指针和返回地址
    fprintf(out, "  lw $ra, 0($sp)\n");
    fprintf(out, "  addi $sp, $sp, 4\n");
    // 保存返回值
    Operand op=ic->u.one_op.op;
    Register reg=ensure(op,out,LEFT);
    fprintf(out,"  move $%s, $v0\n",reg->name);
}

void fprintWRITE(InterCode ic, FILE* out){
    Operand op=ic->u.one_op.op;
    Register reg=ensure(op,out,RIGHT);
    freeReg(reg);
    // 传入参数
    fprintf(out, "  move $a0, $%s\n", reg->name);
    // 保存返回地址
    fprintf(out, "  addi $sp, $sp, -4\n");
    fprintf(out, "  sw $ra, 0($sp)\n");
    // 调用write函数
    fprintf(out, "  jal write\n");
    // 恢复栈指针和返回地址
    fprintf(out, "  lw $ra, 0($sp)\n");
    fprintf(out, "  addi $sp, $sp, 4\n");
}



void fprintLABEL(InterCode ic, FILE* out){
    Operand op=ic->u.one_op.op;
    if (DEBUG6) printf("label%d:\n",op->u.label_no);
    fprintf(out,"label%d:\n",op->u.label_no);
}


void fprintGOTO(InterCode ic, FILE* out){
    spillAllVar(out);   // 跳转语句前将所有基本块中使用过的变量溢出到内存
    Operand op=ic->u.one_op.op;
    if (DEBUG6) printf("  j label%d\n",op->u.label_no);
    fprintf(out,"  j label%d\n",op->u.label_no);
}



void fprintASSIGN(InterCode ic, FILE* out){    // x=y
    Operand left=ic->u.two_op.left;
    Operand right=ic->u.two_op.right;
    Register left_reg;
    if (right->kind==OP_CONSTANT){
        left_reg=ensure(left,out,LEFT);
        if (DEBUG6) printf("  li $%s, %d\n",left_reg->name,right->u.const_val);
        fprintf(out,"  li $%s, %d\n",left_reg->name,right->u.const_val);
    }
    else{
        left_reg=ensure(left,out,LEFT);
        Register right_reg=ensure(right,out,RIGHT);
        if (DEBUG6) printf("  move $%s,$%s\n",left_reg->name,right_reg->name);
        fprintf(out,"  move $%s,$%s\n",left_reg->name,right_reg->name);
    }
    
    Variable var=findVar(left);
    if (var->next_use==NULL){
        spill(var,out);
        //fprintf(out,"  sw $%s, %d($fp)\n",left_reg->name,-var->offset);   //TODO:
    }
    
}
void fprintGET_ADDRES(InterCode ic, FILE* out){   // x=&y
    Operand left=ic->u.two_op.left;
    Operand right=ic->u.two_op.right;
    Register left_reg=ensure(left,out,LEFT);
    Variable var=findVar(right);
    if (DEBUG6) printf("  la $%s, %d($fp)\n",left_reg->name,-var->offset);
    fprintf(out,"  la $%s, %d($fp)\n",left_reg->name,-var->offset);
    
    Variable left_var=findVar(left);
    if (var->next_use==NULL){
        spill(var,out);
        //fprintf(out,"  sw $%s, %d($fp)\n",left_reg->name,-left_var->offset);
    }
    
}

void fprintGET_VALUE(InterCode ic, FILE* out){    // x=*y
    Operand left=ic->u.two_op.left;
    Operand right=ic->u.two_op.right;
    Register left_reg=ensure(left,out,LEFT);
    Register right_reg=ensure(right,out,RIGHT);
    fprintf(out,"  lw $%s, 0($%s)\n",left_reg->name,right_reg->name);
    if (DEBUG6) printf("  lw $%s, 0($%s)\n",left_reg->name,right_reg->name);
    
    Variable var=findVar(left);
    if (var->next_use==NULL){
        spill(var,out);
        //fprintf(out,"  sw $%s, %d($fp)\n",left_reg->name,-var->offset);
    }
    
}

void fprintASSIGN_VALUE(InterCode ic, FILE* out){   // *x=y
    Operand left=ic->u.two_op.left;
    Operand right=ic->u.two_op.right;
    Register left_reg=ensure(left,out,RIGHT);
    Register right_reg=ensure(right,out,RIGHT);
    fprintf(out,"  sw $%s,0($%s)\n",right_reg->name,left_reg->name);

}


void fprintFUNCTION(InterCode ic, FILE* out){   // TODO: Prologue 布置好函数的活动记录
    Operand op=ic->u.one_op.op;
    if (DEBUG6) printf("%s:\n",op->u.func_name);
    fprintf(out,"%s:\n",op->u.func_name);

    fprintf(out,"  subu $sp, $sp, %d\n",framesize);  // 给该函数分配空间
    fprintf(out,"  sw $ra, %d($sp)\n",framesize - 4); //  存储返回值
    fprintf(out,"  sw $fp, %d($sp)\n",framesize - 8); // 存储fp
    fprintf(out,"  addi $fp, $sp, %d\n",framesize ); // 设置新的fp
}

void fprintRETURN(InterCode ic, FILE* out){  // TODO: Epilogue 恢复活动记录

    fprintf(out,"  lw $ra, %d($sp)\n",framesize-4);  // 恢复返回值
    fprintf(out,"  lw $fp, %d($sp)\n",framesize-8);  // 恢复fp
    fprintf(out,"  addi $sp, $sp, %d\n",framesize);   // 将栈顶恢复到上一层活动记录

    Register reg=ensure(ic->u.one_op.op,out,RIGHT);
    if (DEBUG6) { printf("  move $v0, $%s\n",reg->name); printf("  jr,$ra\n");}
    fprintf(out,"  move $v0, $%s\n",reg->name);      // 设置返回值
    fprintf(out,"  jr, $ra\n");                      // 跳转回上一层函数
}


void fprintARG(InterCode ic, FILE* out){   // TODO:
    if (arg_num<4){   // 当前传入的参数个数小于4 只需要压入a0-a3寄存器
        int reg_no = arg_num + 4; 
        Operand op = ic->u.one_op.op;
        Register reg = ensure(op, out, RIGHT);
        fprintf(out,"  move $%s, $%s\n",REG[reg_no]->name,reg->name);
        freeReg(reg);
    }
    else{  // 当前传入的参数个数大于4 需要压入栈
        Operand op=ic->u.one_op.op;
        Register reg = ensure(op,out,RIGHT);
        fprintf(out, "  addi $sp, $sp, -4\n");
        fprintf(out, "  sw $%s, 0($sp)\n", reg->name);
        freeReg(reg);
    }
    arg_num++;
}

void fprintCALL(InterCode ic, FILE* out){  // TODO:
    //pushVarToStack(out);  // 函数调用前将活跃变量压入栈
    spillAllVar(out);
    fprintf(out,"  jal %s\n",ic->u.two_op.right->u.func_name);
    // 调用结束 将传递到栈中的参数清零
    fprintf(out, "  addi $sp, $sp, %d\n", max(0, (arg_num - 4) * 4));
    //loadVarFromStack(out);
    arg_num = 0;  
    // 将函数返回值传给左值
    Operand left = ic->u.two_op.left;
    Register reg = ensure(left,out,LEFT);
    fprintf(out,"  move $%s, $v0\n",reg->name);
}

void pushVarToStack(FILE* out){  // 将值存在寄存器中的变量压回栈中，以进行函数调用
    if (DEBUG6) printf("push var to stack\n");
    VariableList varlist=localVarList;
    while(varlist!=NULL){
        Variable var=varlist->var;
        if (var->reg!=NULL){
            fprintf(out,"  sw $%s, %d($fp)\n",var->reg->name,-var->offset);
        }
        varlist = varlist->next;
    }
}

void loadVarFromStack(FILE* out){  // 将内存中的变量重新存回寄存器
    if (DEBUG6) printf("load var from stack\n");
    VariableList varlist=localVarList;
    while(varlist!=NULL){
        Variable var=varlist->var;
        if (var->reg!=NULL){
            fprintf(out,"  lw $%s, %d($fp)\n",var->reg->name,-var->offset);
        }
        varlist = varlist->next;
    }
}

void fprintADD(InterCode ic, FILE* out){  
    Operand op1=ic->u.three_op.op1;
    Operand op2=ic->u.three_op.op2;
    Operand result=ic->u.three_op.result;
    Register res_reg;
    if (op2->kind==OP_CONSTANT){
        Register reg1=ensure(op1,out,RIGHT);
        res_reg=allocate(result,out);
        if (DEBUG6) printf("  addi $%s, $%s, %d\n",res_reg->name,reg1->name,op2->u.const_val);
        fprintf(out,"  addi $%s, $%s, %d\n",res_reg->name,reg1->name,op2->u.const_val);
        if (notNeed(op1,ic->ic_no))
            freeReg(reg1);
    }
    else{
        Register reg1=ensure(op1,out,RIGHT);
        Register reg2=ensure(op2,out,RIGHT);
        res_reg=allocate(result,out);
        if (DEBUG6) printf("  add $%s, $%s, $%s\n",res_reg->name,reg1->name,reg2->name);
        fprintf(out,"  add $%s, $%s, $%s\n",res_reg->name,reg1->name,reg2->name);
        if (notNeed(op1,ic->ic_no))
            freeReg(reg1);
        if (notNeed(op2,ic->ic_no))
            freeReg(reg2);
    }
    Variable var=findVar(result);
    
    if (var->next_use==NULL){
        spill(var,out);
        //fprintf(out,"  sw $%s, %d($fp)\n",res_reg->name,-var->offset);
    }
    
}

void fprintSUB(InterCode ic, FILE* out){
    Operand op1=ic->u.three_op.op1;
    Operand op2=ic->u.three_op.op2;
    Operand result=ic->u.three_op.result;
    Register res_reg;
    if (op2->kind==OP_CONSTANT){
        Register reg1=ensure(op1,out,RIGHT);
        res_reg=allocate(result,out);
        if (DEBUG6) printf("  addi $%s, $%s, %d\n",res_reg->name,reg1->name,-(op2->u.const_val));
        fprintf(out,"  addi $%s, $%s, %d\n",res_reg->name,reg1->name,-(op2->u.const_val));
        if (notNeed(op1,ic->ic_no))
            freeReg(reg1);
    }
    else{
        Register reg1=ensure(op1,out,RIGHT);
        Register reg2=ensure(op2,out,RIGHT);
        res_reg=allocate(result,out);
        if (DEBUG6) printf("  sub $%s, $%s, $%s\n",res_reg->name,reg1->name,reg2->name);
        fprintf(out,"  sub $%s, $%s, $%s\n",res_reg->name,reg1->name,reg2->name);
        if (notNeed(op1,ic->ic_no))
            freeReg(reg1);
        if (notNeed(op2,ic->ic_no))
            freeReg(reg2);
    }
    Variable var=findVar(result);
    
    if (var->next_use==NULL){
        spill(var,out);
        //fprintf(out,"  sw $%s, %d($fp)\n",res_reg->name,-var->offset);
    }
    
}

void fprintMUL(InterCode ic, FILE* out){  // 乘法除法不支持非零常数
    Operand op1=ic->u.three_op.op1;
    Operand op2=ic->u.three_op.op2;
    Operand result=ic->u.three_op.result;

    Register reg1=ensure(op1,out,RIGHT);
    Register reg2=ensure(op2,out,RIGHT);
    Register res_reg=allocate(result,out);
    if (DEBUG6) printf("  mul $%s, $%s, $%s\n",res_reg->name,reg1->name,reg2->name);
    fprintf(out,"  mul $%s, $%s, $%s\n",res_reg->name,reg1->name,reg2->name);
    if (notNeed(op1,ic->ic_no))
        freeReg(reg1);
    if (notNeed(op2,ic->ic_no))
        freeReg(reg2);

    Variable var=findVar(result);
    
    if (var->next_use==NULL){
        spill(var,out);
        //fprintf(out,"  sw $%s, %d($fp)\n",res_reg->name,-var->offset);
    }
}

void fprintDIV(InterCode ic, FILE* out){
    Operand op1=ic->u.three_op.op1;
    Operand op2=ic->u.three_op.op2;
    Operand result=ic->u.three_op.result;

    Register reg1=ensure(op1,out,RIGHT);
    Register reg2=ensure(op2,out,RIGHT);
    Register res_reg=allocate(result,out);
    if (DEBUG6) printf("  div $%s, $%s, $%s\n",res_reg->name,reg1->name,reg2->name);
    fprintf(out,"  div $%s, $%s, $%s\n",res_reg->name,reg1->name,reg2->name);
    if (notNeed(op1,ic->ic_no))
        freeReg(reg1);
    if (notNeed(op2,ic->ic_no))
        freeReg(reg2);
    Variable var=findVar(result);
    
    if (var->next_use==NULL){
        spill(var,out);
        //fprintf(out,"  sw $%s, %d($fp)\n",res_reg->name,-var->offset);
    }
    
}

void fprintIF(InterCode ic, FILE* out){
    Operand op1=ic->u.if_goto.op1;
    Operand op2=ic->u.if_goto.op2;
    Operand label=ic->u.if_goto.label;
    Register reg1=ensure(op1,out,RIGHT);
    Register reg2=ensure(op2,out,RIGHT);
    char*relop=ic->u.if_goto.relop;
    spillAllVar(out);
    if (strcmp(relop,"==")==0){
        if (DEBUG6) printf("  beq $%s,$%s,label%d\n",reg1->name,reg2->name,label->u.label_no);
        fprintf(out,"  beq $%s,$%s,label%d\n",reg1->name,reg2->name,label->u.label_no);
    }
    else if (strcmp(relop,"!=")==0){
        if (DEBUG6) printf("  bne $%s,$%s,label%d\n",reg1->name,reg2->name,label->u.label_no);
        fprintf(out,"  bne $%s,$%s,label%d\n",reg1->name,reg2->name,label->u.label_no);
    }
    else if (strcmp(relop,">")==0){
        if (DEBUG6) printf("  bgt $%s,$%s,label%d\n",reg1->name,reg2->name,label->u.label_no);
        fprintf(out,"  bgt $%s,$%s,label%d\n",reg1->name,reg2->name,label->u.label_no);
    }
    else if (strcmp(relop,"<")==0){
        if (DEBUG6) printf("  blt $%s,$%s,label%d\n",reg1->name,reg2->name,label->u.label_no);
        fprintf(out,"  blt $%s,$%s,label%d\n",reg1->name,reg2->name,label->u.label_no);
    }
    else if (strcmp(relop,">=")==0){
        if (DEBUG6) printf("  bge $%s,$%s,label%d\n",reg1->name,reg2->name,label->u.label_no);
        fprintf(out,"  bge $%s,$%s,label%d\n",reg1->name,reg2->name,label->u.label_no);
    }
    else if (strcmp(relop,"<=")==0){
        if (DEBUG6) printf("  ble $%s,$%s,label%d\n",reg1->name,reg2->name,label->u.label_no);
        fprintf(out,"  ble $%s,$%s,label%d\n",reg1->name,reg2->name,label->u.label_no);
    }
}


Register ensure(Operand op, FILE* out, enum VarUseType use_type){  // 为Operand找到其寄存器
    if (DEBUG6) printf("ensure a register\n");
    if (op->kind==OP_CONSTANT)  // 分配一个临时寄存器给常量使用，使用完即可释放
    {
        Register res=REG[24];  // t8寄存器专门存放临时变量
        fprintf(out,"  li $%s, %d\n",res->name,op->u.const_val);
        return res;
    }
    Variable var=findVar(op); 
    assert(var!=NULL);
    Register res;
    if (var->reg!=NULL){  // 变量的值已经有寄存器存储 直接返回该寄存器
        res=var->reg;
    }
    else{                 // 没有寄存器存有该变量，需要为该变量分配一个新的寄存器
        res=allocate(op,out);
        if (use_type == RIGHT)  // 如果是右值，需要使用该变量的值，那么就需要从本地中将该变量取出
            fprintf(out,"  lw $%s, %d($fp)\n",res->name,-var->offset); // 从本地将该变量取出 并放入寄存器
        var->reg=res;
    }
    return res;
}

void freeReg(Register reg){  // 将寄存器设置为闲置
    if (DEBUG6) printf("FREE REG $%s\n",reg->name);
    reg->state = FREE;
}


Register allocate(Operand op, FILE* out){  // 为Operand分配一个新的寄存器
    if (DEBUG6) printf("allocate a new register\n");
    // 首先寻找空闲的寄存器，如果找到则返回
    for (int i=8;i<15;i++){   
        if (REG[i]->state==FREE){     
            if (DEBUG6) printf("Find FREE Reg $%s\n",REG[i]->name);
            REG[i]->state=BUSY;
            Variable var=findVar(op);
            var->reg=REG[i];
            //addRegForVar(var,REG[i]);
            return REG[i];
        }
    }
    // 如果没有空闲的寄存器那么需要将某个寄存器的变量溢出回内存
    Variable spill_var;
    int max_next_use=0;
    VariableList varlist=localVarList;
    while(varlist!=NULL){
        Variable var=varlist->var;
        if (var->reg!=NULL){
            if (var->next_use==NULL){  // 不再使用但占用了一个寄存器 将其溢出回内存
                spill_var=var;
                break;
            }
            else if (var->next_use->next_no > max_next_use){
                spill_var=var;
                max_next_use=var->next_use->next_no;
            }
        }
        varlist = varlist->next;
    }
    Register new_reg=spill_var->reg;
    spill(spill_var,out);   // 将变量溢出回内存
    new_reg->state=BUSY;
    return new_reg;
}


bool notNeed(Operand op,int ic_no){  // 该变量后续不会被用到
    if (op->kind==OP_CONSTANT)
        return true;
    Variable var=findVar(op);
    if (var->next_use==NULL)
        return true;
    else if (var->next_use->next_no==ic_no && var->next_use->next==NULL)  // 当前正在使用 但后续不会被用到
        return true;
    else 
        return false;
}

void spill(Variable var,FILE* out){  // 将寄存器内的变量溢出到内存
    if (DEBUG6) {
        printf("spill a var ");
        printOperand(var->op);
        printf(" from $%s\n",var->reg->name);
        printf("  sw $%s, %d($fp)\n",var->reg->name,-(var->offset));
    }
    fprintf(out,"  sw $%s, %d($fp)\n",var->reg->name,-(var->offset));
    var->reg->state = FREE;
    var->reg = NULL;
}

void printLocalVarList(){
    printf("This is localVarList:\n");
    VariableList varlist=localVarList;
    while(varlist!=NULL){
        Variable var=varlist->var;
        printOperand(var->op);
        if (var->reg!=NULL)
            printf(" reg: $%s",var->reg->name);
        NextUseList next_use=var->next_use;
        printf(" offset: %d ",var->offset);
        printf("  Next_Use: ");
        while(next_use!=NULL){
            printf("%d ",next_use->next_no);
            next_use=next_use->next;
        }
        printf("\n");
        varlist=varlist->next;
    }
}