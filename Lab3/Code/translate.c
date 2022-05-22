#include "translate.h"
#include "semantic.h"

#include <stdio.h>
#include <assert.h>

int temp_cnt=1;
int label_cnt=1;
int var_cnt=1;
int addr_cnt=1;

const bool DEBUG3 = false;

void translateProgram(TreeNode node,char* out_name){
    if (DEBUG3) printf("translate_Start\n");
    /* Program -> ExtDefList */
    translateExtDefList(node->children[0]);
    if (DEBUG3) printf("translate_End\n");
    
    if (out_name!=NULL){
        f_out_ir=fopen(out_name,"w");
        assert(f_out_ir!=NULL);
        fprintCodes(f_out_ir);
        fclose(f_out_ir);
    }

    
}

void translateExtDefList(TreeNode node){
    if (node==NULL)
        return ;
    assert(node->children_num==2);
    if (DEBUG3) printf("translate_ExtDefList\n");
    /* ExtDefList -> ExtDef ExtDeFList */
    translateExtDef(node->children[0]);
    translateExtDefList(node->children[1]);
}

void translateExtDef(TreeNode node){
    assert(node->children_num==3 || node->children_num==2);
    if (DEBUG3) printf("translate_ExtDef\n");
    // 只需考虑定义函数的情况 结构体定义无需翻译
    if (node->children_num==3){
        translateFunDec(node->children[1]);
        translateCompSt(node->children[2]);
    }
    /* ExtDef -> Specifier FunDec CompSt */
}

void translateExtDecList(TreeNode node){
    assert(node->children_num==1 || node->children_num==3);
    if (DEBUG3) printf("translate_ExtDecList\n");
    /* ExtDecList -> VarDec */
    translateVarDec(node->children[0]);
    /* ExtDec -> VarDec COMMA ExtDecList */
    translateVarDec(node->children[0]);
    translateExtDecList(node->children[2]);
}

void translateFunDec(TreeNode node){ 
    assert(node->children_num==3 || node->children_num==4);
    if (DEBUG3) printf("translate_FunDec\n");
    char*name=node->children[0]->val.val_str;
    FieldList f=lookUp(SymbolTable,name);
    assert(f!=NULL);
    genCode(GEN_FUNCTION,1,name);
    /* FunDec -> ID LP VarList RP */
    if (node->children_num==4){
        //translateVarList(node->children[2]);
        FieldList paras=f->type->u.function.paras;
        while(paras!=NULL){ // 函数的参数只有可能是普通变量或者结构体
            Operand op=genOperand(OP_VARIABLE,paras->no,NULL);
            genCode(GEN_PARAM,1,op);
            if (paras->type->kind==STRUCTURE)
                paras->is_arg=true;
            else if (paras->type->kind==ARRAY)
                printTranslateError();
            paras=paras->tail;
        }
    }
    /* FunDec -> ID LP RP 无需传入参数*/
}


void translateCompSt(TreeNode node){
    assert(node->children_num==4);
    if (DEBUG3) printf("translate_CompSt\n");
    /* CompSt -> LC DefList StmtList RC */
    translateDefList(node->children[1]);
    translateStmtList(node->children[2]);
}

void translateDefList(TreeNode node){
    if (node==NULL)
        return ;
    assert(node->children_num==2);
    if (DEBUG3) printf("translate_DefList\n");
    /* DefList -> Def DefList */
    translateDef(node->children[0]);
    translateDefList(node->children[1]);
}

void translateDef(TreeNode node){
    assert(node->children_num==3);
    if (DEBUG3) printf("translate_Def\n");
    /* Def -> Specifier DecList SEMI */
    translateDecList(node->children[1]);
}

void translateDecList(TreeNode node){
    if (node==NULL)
        return ;
    assert(node->children_num==1 || node->children_num==3);
    if (DEBUG3) printf("translate_DecList\n");
    /* DecList -> Dec */
    if (node->children_num==1){
        translateDec(node->children[0]);
    }
    /* DecList -> Dec COMMA DecList */
    else{
        translateDec(node->children[0]);
        translateDecList(node->children[2]);
    }
}

void translateDec(TreeNode node){
    assert(node->children_num==1 || node->children_num==3);
    if (DEBUG3) printf("translate_Dec\n");
    /* Dec -> VarDec */
    if (node->children_num==1){
        translateVarDec(node->children[0]);
    }
    /* Dec -> VarDec ASSIGNOP Exp */
    else{
        if (expIsConst(node->children[2])){
            int val=getExpConst(node->children[2]);
            Operand v1=translateVarDec(node->children[0]);
            genCode(GEN_ASSIGN,2,v1,newConst(val));
        }  
        else{
            Operand t1=newTemp();
            translateExp(node->children[2],t1);
            Operand v1=translateVarDec(node->children[0]);
            genCode(GEN_ASSIGN,2,v1,t1);
        }
    }
}

Operand translateVarDec(TreeNode node){  
    assert(node->children_num==1 || node->children_num==4);
    if (DEBUG3) printf("translate_VarDec\n");
    /* VarDec -> ID */
    if (node->children_num==1){  
        char*name=node->children[0]->val.val_str;
        FieldList f=lookUp(SymbolTable,name);
        assert(f!=NULL);
        if (f->type->kind==BASIC && f->type->u.basic==BASIC_INT){  // int变量
            Operand op=genOperand(OP_VARIABLE,f->no,NULL);
            return op;
        }
        else if (f->type->kind==STRUCTURE){  // 结构体 需要进行DEC声明
            int size=calSize(f->type);
            genCode(GEN_DEC,2,f->no,size);
            return NULL;
        }
        else if (f->type->kind==ARRAY){
            int size=calSize(f->type);
            genCode(GEN_DEC,2,f->no,size);
        }
    }
    /* VarDec -> VarDec LB INT RB */
    else{
        translateVarDec(node->children[0]);  // 数组信息处理已经在语义分析阶段完成了，直接使用符号表数据救好
    }
}


void translateStmtList(TreeNode node){
    if (node==NULL)
        return ;
    /* StmtList -> Stmt StmtList */
    assert(node->children_num==2);
    if (DEBUG3) printf("translate_StmtList\n");
    translateStmt(node->children[0]);
    translateStmtList(node->children[1]);     
}

void translateStmt(TreeNode node){
    /* Stmt -> Exp SEMI */
    if (node->children_num==2){
        if (DEBUG3) printf("Stmt -> EXP SEMI\n");
        translateExp(node->children[0],NULL);
        if (DEBUG3) printf("Stmt -> EXP SEMI !!!\n");
    }
    /* Stmt -> CompSt */
    else if (node->children_num==1){
        if (DEBUG3) printf("Stmt -> CompSt\n");
        translateCompSt(node->children[0]);
        if (DEBUG3) printf("Stmt -> CompSt !!!\n");
    }

    /* Stmt -> RETURN Exp SEMI */
    else if (node->children_num==3){
        if (DEBUG3) printf("Stmt -> RETURN EXP SEMI\n");
        Operand t1=newTemp();
        translateExp(node->children[1],t1);
        genCode(GEN_RETURN,1,t1);
        if (DEBUG3) printf("Stmt -> RETURN EXP SEMI !!!\n");
    }
    /* Stmt -> IF LP Exp RP Stmt ELSE Stmt */
    else if (node->children_num==7){
        if (DEBUG3) printf("Stmt -> IF LP EXP RP Stmt ELSE Stmt\n");
        Operand label1=newLabel();
        Operand label2=newLabel();
        Operand label3=newLabel();
        translateCond(node->children[2],label1,label2);
        genCode(GEN_LABEL,1,label1);
        translateStmt(node->children[4]);
        genCode(GEN_GOTO,1,label3);
        genCode(GEN_LABEL,1,label2);
        translateStmt(node->children[6]);
        genCode(GEN_LABEL,1,label3);
        if (DEBUG3) printf("Stmt -> IF LP EXP RP Stmt ELSE Stmt !!!\n");
    }
    /* Stmt -> IF LP EXP RP Stmt */
    else if (strcmp(node->children[0]->name,"IF")==0){
        if (DEBUG3) printf("Stmt -> IF LP EXP RP Stmt\n");
        Operand label1=newLabel();
        Operand label2=newLabel();
        translateCond(node->children[2],label1,label2);
        genCode(GEN_LABEL,1,label1);
        translateStmt(node->children[4]);
        genCode(GEN_LABEL,1,label2);
        if (DEBUG3) printf("Stmt -> IF LP EXP RP Stmt !!!\n");
    }
    /* Stmt -> WHILE LP Exp RP Stmt */
    else if (strcmp(node->children[0]->name,"WHILE")==0){
        if (DEBUG3) printf("Stmt -> WHILE LP EXP RP Stmt\n");
        Operand label1=newLabel();
        Operand label2=newLabel();
        Operand label3=newLabel();
        genCode(GEN_LABEL,1,label1);
        translateCond(node->children[2],label2,label3);
        genCode(GEN_LABEL,1,label2);
        translateStmt(node->children[4]);
        genCode(GEN_GOTO,1,label1);
        genCode(GEN_LABEL,1,label3);
        if (DEBUG3) printf("Stmt -> WHILE LP EXP RP Stmt !!!\n");
    }
}


void translateExp(TreeNode node,Operand place){
    /* Exp -> Exp ASSIGNOP EXP */
    if (node->children_num==3 && strcmp(node->children[1]->name,"ASSIGNOP")==0){
        if (DEBUG3) printf("EXP -> EXP ASSIGN EXP\n");
        switch (node->children[0]->children_num)
        {
        case 1:{ // 普通变量 ID
            char*name=node->children[0]->children[0]->val.val_str;
            FieldList f=lookUp(SymbolTable,name);
            if (f->type->kind==BASIC){
                Operand v1=genOperand(OP_VARIABLE,f->no,NULL);
                if (expIsConst(node->children[2])){
                    int val=getExpConst(node->children[2]);
                    genCode(GEN_ASSIGN,2,v1,newConst(val));
                }
                else if (expIsID(node->children[2])){
                    FieldList f=lookUp(SymbolTable,getExpID(node->children[2]));
                    genCode(GEN_ASSIGN,2,v1,genOperand(OP_VARIABLE,f->no,NULL));
                }
                else{   
                    Operand right_value=newTemp();
                    translateExp(node->children[2],right_value);
                    genCode(GEN_ASSIGN,2,v1,right_value);
                }

                if (place!=NULL){
                    genCode(GEN_ASSIGN,2,place,v1);
                }
            }
            else if (f->type->kind==ARRAY){
                Operand left=newAddr();
                Operand right=newAddr();
                translateExp(node->children[0],left);
                translateExp(node->children[2],right);
                copyArray(left,right);
            }
            break;}
        case 3:{  // 结构体域 Exp1 -> Exp DOT ID   // TODO:
            Operand addr=newAddr();
            translateExp(node->children[0],addr);
            Operand right_value=newTemp();
            translateExp(node->children[2],right_value);
            if (right_value->kind==OP_ADDRESS)
            {
                copyArray(addr,right_value);
            }
            else{
                genCode(GEN_ASSIGN_VALUE,2,addr,right_value);
            }
            if (place!=NULL){
                genCode(GEN_ASSIGN,2,place,right_value);
            }
            break;}
        case 4:{  // 数组元素 Exp1 -> Exp LB EXP RB   // TODO:
            Operand addr=newAddr();
            translateExp(node->children[0],addr);
            Operand right_value=newTemp();
            translateExp(node->children[2],right_value); 
            genCode(GEN_ASSIGN_VALUE,2,addr,right_value); // *addr1 = t2;
            if (place!=NULL){
                genCode(GEN_ASSIGN,2,place,right_value);
            }
            break;}
        default:
            break;
        }
    }
    /* Exp -> Exp AND|OR|RELOP Exp && Exp-> NOT Exp*/  
    else if ((node->children_num==3 &&  (strcmp(node->children[1]->name,"AND")==0 || 
                                         strcmp(node->children[1]->name,"OR")==0   ||
                                         strcmp(node->children[1]->name,"RELOP")==0)) ||
             (node->children_num==2 && strcmp(node->children[0]->name,"NOT")==0)){
        if (DEBUG3) printf("EXP -> EXP RELOP EXP\n");
        Operand l1=newLabel();
        Operand l2=newLabel();
        if (place!=NULL){
            genCode(GEN_ASSIGN,2,place,genOperand(OP_CONSTANT,0,NULL));
            translateCond(node,l1,l2);
            genCode(GEN_LABEL,1,l1);
            genCode(GEN_ASSIGN,2,place,genOperand(OP_CONSTANT,1,NULL));
        }
    }
    /* Exp -> Exp PLUS Exp */
    else if (node->children_num==3 && strcmp(node->children[1]->name,"PLUS")==0){
        if (DEBUG3) printf("EXP -> EXP PLUS EXP\n");
        if (place !=NULL){
            Operand t1,t2;
            if (expIsConst(node->children[0]))
                t1=newConst(getExpConst(node->children[0]));
            else if (expIsID(node->children[0])){
                FieldList f=lookUp(SymbolTable,getExpID(node->children[0]));
                t1=genOperand(OP_VARIABLE,f->no,NULL);
            }
            else {
                t1=newTemp();
                translateExp(node->children[0],t1);
            }
            if (expIsConst(node->children[2]))
                t2=newConst(getExpConst(node->children[2]));
            else if (expIsID(node->children[2])){
                FieldList f=lookUp(SymbolTable,getExpID(node->children[2]));
                t2=genOperand(OP_VARIABLE,f->no,NULL);
            }
            else {
                t2=newTemp();
                translateExp(node->children[2],t2);
            }
            genCode(GEN_ADD,3,place,t1,t2);         
        }
    }
    /* Exp -> Exp MINUS Exp */
    else if (node->children_num==3 && strcmp(node->children[1]->name,"MINUS")==0){
        if (DEBUG3) printf("EXP -> EXP MINUS EXP\n");
        if (place !=NULL){
            Operand t1,t2;
            if (expIsConst(node->children[0]))
                t1=newConst(getExpConst(node->children[0]));
            else if (expIsID(node->children[0])){
                FieldList f=lookUp(SymbolTable,getExpID(node->children[0]));
                t1=genOperand(OP_VARIABLE,f->no,NULL);
            }
            else {
                t1=newTemp();
                translateExp(node->children[0],t1);
            }
            if (expIsConst(node->children[2]))
                t2=newConst(getExpConst(node->children[2]));
            else if (expIsID(node->children[2])){
                FieldList f=lookUp(SymbolTable,getExpID(node->children[2]));
                t2=genOperand(OP_VARIABLE,f->no,NULL);
            }
            else {
                t2=newTemp();
                translateExp(node->children[2],t2);
            }
            genCode(GEN_SUB,3,place,t1,t2);
        }
    }
    /* Exp -> Exp STAR Exp */
    else if (node->children_num==3 && strcmp(node->children[1]->name,"STAR")==0){
        if (DEBUG3) printf("EXP -> EXP MUL EXP\n");
        if (place !=NULL){
            Operand t1,t2;
            if (expIsConst(node->children[0]))
                t1=newConst(getExpConst(node->children[0]));
            else if (expIsID(node->children[0])){
                FieldList f=lookUp(SymbolTable,getExpID(node->children[0]));
                t1=genOperand(OP_VARIABLE,f->no,NULL);
            }
            else {
                t1=newTemp();
                translateExp(node->children[0],t1);
            }
            if (expIsConst(node->children[2]))
                t2=newConst(getExpConst(node->children[2]));
            else if (expIsID(node->children[2])){
                FieldList f=lookUp(SymbolTable,getExpID(node->children[2]));
                t2=genOperand(OP_VARIABLE,f->no,NULL);
            }
            else {
                t2=newTemp();
                translateExp(node->children[2],t2);
            }
            genCode(GEN_MUL,3,place,t1,t2);
        }
    }

    /* Exp -> Exp DIV Exp */
    else if (node->children_num==3 && strcmp(node->children[1]->name,"DIV")==0){
        if (DEBUG3) printf("EXP -> EXP DIV EXP\n");
        if (place !=NULL){
            Operand t1,t2;
            if (expIsConst(node->children[0]))
                t1=newConst(getExpConst(node->children[0]));
            else if (expIsID(node->children[0])){
                FieldList f=lookUp(SymbolTable,getExpID(node->children[0]));
                t1=genOperand(OP_VARIABLE,f->no,NULL);
            }
            else {
                t1=newTemp();
                translateExp(node->children[0],t1);
            }
            if (expIsConst(node->children[2]))
                t2=newConst(getExpConst(node->children[2]));
            else if (expIsID(node->children[2])){
                FieldList f=lookUp(SymbolTable,getExpID(node->children[2]));
                t2=genOperand(OP_VARIABLE,f->no,NULL);
            }
            else {
                t2=newTemp();
                translateExp(node->children[2],t2);
            }
            genCode(GEN_DIV,3,place,t1,t2);
        }
    }

    /* Exp -> LP Exp RP */
    else if (node->children_num==3 && strcmp(node->children[0]->name,"LP")==0){
        if (DEBUG3) printf("EXP -> LP EXP RP\n");
        translateExp(node->children[1],place);
    }

    /* Exp -> MINUS Exp */
    else if (node->children_num==2 && strcmp(node->children[0]->name,"MINUS")==0){
        if (DEBUG3) printf("EXP -> MINUS EXP\n");
        if (place !=NULL){
            Operand t1=genOperand(OP_TEMP,temp_cnt,NULL);
            temp_cnt++;
            if (expIsConst(node->children[1])){   // 优化 如果exp为一个常数 则无需声明一个临时变量
                int val=getExpConst(node->children[1]);
                genCode(GEN_ASSIGN,2,place,newConst(-val));
            }
            else{
                Operand t1=genOperand(OP_TEMP,temp_cnt,NULL);
                translateExp(node->children[1],t1);
                genCode(GEN_SUB,3,place,genOperand(OP_CONSTANT,0,NULL),t1);
            }
            
        }
    }
    /* Exp -> ID LP Args RP */   // func(a,b)  //TODO:
    else if (node->children_num==4 && strcmp(node->children[2]->name,"Args")==0){
        if (DEBUG3) printf("EXP -> ID LP ARGS RP\n");
        char*name=node->children[0]->val.val_str;
        FieldList f = lookUp(SymbolTable,name);
        ArgNode arg_list = translateArgs(node->children[2]);
        if (strcmp(name,"write")==0){
            genCode(GEN_WRITE,1,arg_list->op);
            if (place!=NULL)
                genCode(GEN_ASSIGN,2,place,newConst(0));
        }
        else{
            while(arg_list!=NULL){
                genCode(GEN_ARG,1,arg_list->op);
                arg_list=arg_list->next;
            }
            genCode(GEN_CALL,2,place,name);
        }
    }
    /* Exp -> ID LP RP */       // func()
    else if (node->children_num==3 && strcmp(node->children[1]->name,"LP")==0){
        if (DEBUG3) printf("EXP -> ID LP RP\n");
        char*name=node->children[0]->val.val_str;
        FieldList f=lookUp(SymbolTable,name);
        if (strcmp(name,"read")==0){
            genCode(GEN_READ,1,place);
        }
        else{
            genCode(GEN_CALL,2,place,name);
        }

    }
    /* Exp -> Exp LB Exp RB */   // A[b]    //TODO:
    else if (node->children_num==4 && strcmp(node->children[1]->name,"LB")==0){
        if (DEBUG3) printf("EXP -> EXP LB EXP RB\n");
        Operand base=newAddr();
        translateExp(node->children[0],base);

        Operand t1=newTemp();
        translateExp(node->children[2],t1);
        Operand offset=newTemp();
        genCode(GEN_MUL,3,offset,t1,newConst(calSize(base->u.addr.addr_info.array_info.type)));
        genCode(GEN_ADD,3,base,base,offset);  
        if (place->kind==OP_TEMP){   //这里需要该表达式的值
            if (base->u.addr.addr_info.array_info.type->kind == BASIC)
                genCode(GEN_GET_VALUE,2,place,base);
            else if (base->u.addr.addr_info.array_info.type->kind == STRUCTURE){  // 但表达式的结果实际上是一个地址,所以这里应当是在函数调用
                place->kind=OP_ADDRESS;
                place->u.addr.addr_no=addr_cnt;
                addr_cnt++;
                place->u.addr.addr_info.st_info=base->u.addr.addr_info.array_info.type->u.structure;
                genCode(GEN_ASSIGN,2,place,base);
            }
            else if (base->u.addr.addr_info.array_info.type->kind == ARRAY){
                printTranslateError();
            }
        }
        else if (place->kind == OP_ADDRESS) {   // 这里需要该表达式的地址，直接将地址运算后赋值给place
            genCode(GEN_ASSIGN,2,place,base);
            if (base->u.addr.addr_info.array_info.type->kind==STRUCTURE){
                place->u.addr.addr_info.st_info=base->u.addr.addr_info.array_info.type->u.structure;
            }
            else if (base->u.addr.addr_info.array_info.type->kind==ARRAY){
                printTranslateError();
            }
        }

    }
    /* Exp -> Exp DOT ID */      //s.a      //TODO:
    else if (node->children_num==3 && strcmp(node->children[1]->name,"DOT")==0){
        if (DEBUG3) printf("EXP -> EXP DOT ID\n");
        Operand addr=newAddr();
        translateExp(node->children[0],addr);

        char* name=node->children[2]->val.val_str;
        Operand offset=newConst(calOffset(addr->u.addr.addr_info.st_info,name));
        genCode(GEN_ADD,3,addr,addr,offset);
        if (place->kind==OP_TEMP){
            Type type=findStructDomainType(addr->u.addr.addr_info.st_info,name);
            if (type->kind==BASIC)
                genCode(GEN_GET_VALUE,2,place,addr);
            else if (type->kind==ARRAY){
                place->kind=OP_ADDRESS;
                place->u.addr.addr_no=addr_cnt;
                addr_cnt++;
                place->u.addr.addr_info.array_info.type=type->u.array.elem;
                place->u.addr.addr_info.array_info.size=type->u.array.size;
                genCode(GEN_ASSIGN,2,place,addr);
            }
            else if (type->kind==STRUCTURE){
                place->kind=OP_ADDRESS;
                place->u.addr.addr_no=addr_cnt;
                addr_cnt++;
                place->u.addr .addr_info.st_info=type->u.structure;
                genCode(GEN_ASSIGN,2,place,addr);
            }
        }
        else if (place->kind==OP_ADDRESS){
            genCode(GEN_ASSIGN,2,place,addr);
            FieldList f=lookUp(SymbolTable,name);
            if (f->type->kind==ARRAY){
                place->u.addr.addr_info.array_info.type=f->type->u.array.elem;
                place->u.addr.addr_info.array_info.size=f->type->u.array.size;
            }
        }

    }   

    /* Exp -> ID */
    else if (strcmp(node->children[0]->name,"ID")==0){  // TODO:
        if (DEBUG3) printf("EXP -> ID:%s\n",node->children[0]->val.val_str);
        if (place==NULL)
            return ;
        else{
            char*name=node->children[0]->val.val_str;
            FieldList f=lookUp(SymbolTable,name);
            assert(f!=NULL);
            if (f->type->kind==BASIC){
                if (place->kind==OP_TEMP){
                    genCode(GEN_ASSIGN,2,place,genOperand(OP_VARIABLE,f->no,NULL));
                }
                else {
                    genCode(GEN_GET_ADDRES,2,place,genOperand(OP_VARIABLE,f->no,NULL));
                }
            }
            else if (f->type->kind == ARRAY){
                if (place->kind == OP_ADDRESS){
                    place->u.addr.addr_info.array_info.type=f->type->u.array.elem;
                    place->u.addr.addr_info.array_info.size=f->type->u.array.size;
                    genCode(GEN_GET_ADDRES,2,place,genOperand(OP_VARIABLE,f->no,NULL));
                }
            }
            else if (f->type->kind == STRUCTURE){
                FieldList p=f->type->u.structure;
                if (place->kind == OP_ADDRESS){
                    place->u.addr.addr_info.st_info=f->type->u.structure;
                    if (f->is_arg){
                        genCode(GEN_ASSIGN,2,place,genOperand(OP_VARIABLE,f->no,NULL));
                    }
                    else{
                        genCode(GEN_GET_ADDRES,2,place,genOperand(OP_VARIABLE,f->no,NULL));
                    }
                }
            }
        }
    }
    /* Exp -> INT */
    else if (strcmp(node->children[0]->name,"INT")==0){
        if (DEBUG3) printf("EXP -> INT:%d\n",node->children[0]->val.val_int);
        if (place == NULL)
            return ;
        else{
            int val=node->children[0]->val.val_int;
            genCode(GEN_ASSIGN,2,place,newConst(val));
        }
    }
}


ArgNode translateArgs(TreeNode node){  // TODO:
    if (DEBUG3) printf("translate_Args\n");
    /* Arg -> EXP */
    if (node->children_num == 1){
        Operand op;
        if (expIsID(node->children[0])){
            op = getExpArg(node->children[0]);
        }
        else{
            op=newTemp();
            translateExp(node->children[0],op);    
        }
        ArgNode arg_node=(ArgNode)malloc(sizeof(struct ArgNode_));
        arg_node->op=op;
        arg_node->next=NULL;
        return arg_node;

    }
    /* Arg -> EXP COMMA Arg */
    else{
        ArgNode arglist = translateArgs(node->children[2]);
        Operand op;
        if (expIsID(node->children[0])){
            op=getExpArg(node->children[0]);
        }
        else{
            op=newTemp();
            translateExp(node->children[0],op);    
        }
        ArgNode arg_node=(ArgNode)malloc(sizeof(struct ArgNode_));
        arg_node->op=op;
        arg_node->next=NULL;
        
        ArgNode temp=arglist;
        while(temp->next!=NULL)
            temp=temp->next;
        temp->next=arg_node;
        return arglist;
        
    }
}

void translateCond(TreeNode node,Operand label_true,Operand label_false){
    /* EXP -> EXP RELOP EXP */
    if (node->children_num==3 && strcmp(node->children[1]->name,"RELOP")==0){
        if (DEBUG3) printf("Cond -> EXP RELOP EXP\n");
        
        Operand t1,t2;
        if (expIsConst(node->children[0])){
            t1=newConst(getExpConst(node->children[0]));
        }
        else if (expIsID(node->children[0])){
            FieldList f=lookUp(SymbolTable,getExpID(node->children[0]));
            t1=genOperand(OP_VARIABLE,f->no,NULL);
        }
        else{
            t1=newTemp();
            translateExp(node->children[0],t1);
        }
        if (expIsConst(node->children[2])){
            t2=newConst(getExpConst(node->children[2]));
        }
        else if (expIsID(node->children[2])){
            FieldList f=lookUp(SymbolTable,getExpID(node->children[2]));
            t2=genOperand(OP_VARIABLE,f->no,NULL);
        }
        else{
            t2=newTemp();
            translateExp(node->children[2],t2);
        }
        char* relop=getRelop(node->children[1]);
        genCode(GEN_IF,4,t1,relop,t2,label_true);
        genCode(GEN_GOTO,1,label_false);

    }
    /* EXP -> NOT EXP */
    else if (node->children_num==2 && strcmp(node->children[0]->name,"NOT")==0){
        if (DEBUG3) printf("Cond -> NOT EXP\n");
        translateCond(node->children[1],label_false,label_true);
    }
    /* EXP -> EXP AND EXP */
    else if (node->children_num==3 && strcmp(node->children[1]->name,"AND")==0){
        if (DEBUG3) printf("Cond -> EXP AND EXP\n");
        Operand label1=newLabel();
        translateCond(node->children[0],label1,label_false);
        genCode(GEN_LABEL,1,label1);
        translateCond(node->children[1],label_true,label_false);
    }
    /* EXP -> EXP OR EXP */
    else if (node->children_num==3 && strcmp(node->children[1]->name,"OR")==0){
        if (DEBUG3) printf("Cond -> EXP OR EXP\n");
        Operand label1=newLabel();
        translateCond(node->children[0],label_true,label1);
        genCode(GEN_LABEL,1,label1);
        translateCond(node->children[1],label_true,label_false);
    }
    /* other cases */
    else{
        if (DEBUG3) printf("Cond -> other cases\n");
        Operand t1=newTemp();
        translateExp(node,t1);
        genCode(GEN_IF,4,t1,"!=",newConst(0),label_true);
        genCode(GEN_GOTO,1,label_false);
    }
}

char* getRelop(TreeNode node){
    if (DEBUG3) printf("getRelop\n");
    char*relop=node->val.val_str;
    return relop;
}

Operand newTemp(){
    Operand op=genOperand(OP_TEMP,temp_cnt,NULL);
    temp_cnt++;
    return op;
}

Operand newLabel(){
    Operand op=genOperand(OP_LABEL,label_cnt,NULL);
    label_cnt++;
    return op;
}

Operand newAddr(){
    Operand op=genOperand(OP_ADDRESS,addr_cnt,NULL);
    addr_cnt++;
    return op;
}

Operand newConst(int val_int){
    Operand op=genOperand(OP_CONSTANT,val_int,NULL);
    return op;
}

Type intType(){
    Type t=(Type)malloc(sizeof(struct Type_));
    t->kind=BASIC;
    t->u.basic=BASIC_INT;
    return t;
}

void copyArray(Operand left,Operand right){
    int left_num=left->u.addr.addr_info.array_info.size;
    int right_num=right->u.addr.addr_info.array_info.size;
    int num=left_num<right_num?left_num:right_num;
    int size=calSize(left->u.addr.addr_info.array_info.type);

    Operand val=newTemp();
    genCode(GEN_GET_VALUE,2,val,right);
    genCode(GEN_ASSIGN_VALUE,2,left,val);
    if (num>=2){
        for (int i=1;i<num;i++){
            genCode(GEN_ADD,3,left,left,newConst(size));
            genCode(GEN_ADD,3,right,right,newConst(size));
            genCode(GEN_GET_VALUE,2,val,right);
            genCode(GEN_ASSIGN_VALUE,2,left,val);
        }      
    }
}


int calSize(Type type){  // 根据名称计算出该类型占用的空间
    if (type->kind==ARRAY){
        return (calSize(type->u.array.elem))*type->u.array.size;
    }
    else if (type->kind==STRUCTURE){
        int sum=0;
        FieldList p=type->u.structure;
        while(p!=NULL){
            sum+=calSize(p->type);
            p=p->tail;
        }
        return sum;
    }
    else{ 
        return 4;    // int
    }
}

int calOffset(FieldList field,char*name){  // TODO:
    if (DEBUG3) printf("calOffset %s:%s\n",field->name,name);
    FieldList f=field;
    int sum=0;
    while(f!=NULL && strcmp(f->name,name)!=0){
        sum+=calSize(f->type);
        f=f->tail;
    }
    return sum;
}

bool expIsConst(TreeNode node){
    return (node->children_num==1 && strcmp(node->children[0]->name,"INT")==0);
}

int getExpConst(TreeNode node){   // 获取常数exp的值 方便优化
    return node->children[0]->val.val_int;
}

bool expIsID(TreeNode node){
    return (node->children_num==1 && strcmp(node->children[0]->name,"ID")==0);
}

char* getExpID(TreeNode node){
    return node->children[0]->val.val_str;
}

Operand getExpArg(TreeNode node){  // 获取变量exp对应的Operand 方便优化
    char*name=node->children[0]->val.val_str;
    FieldList f=lookUp(SymbolTable,name);
    assert(f!=NULL);
    Operand op;
    if (f->type->kind==BASIC){
        op=genOperand(OP_VARIABLE,f->no,NULL);
    }
    else if (f->type->kind==STRUCTURE){
        op=newAddr();
        genCode(GEN_GET_ADDRES,2,op,genOperand(OP_VARIABLE,f->no,NULL));
    }
    return op;
}

Type findStructDomainType(FieldList field,char*name){
    FieldList f=field;
    while(f!=NULL && strcmp(f->name,name)!=0){
        f=f->tail;
    }
    return f->type;
}

void printTranslateError(){
    printf("Cannot translate: Code contains variables of multi-dimensional array type \
or parameters of array type.\n");
    exit(0);
}
