#include <assert.h>
#include "semantic.h"
extern HashNode SymbolTable[SymbolTableSize];
extern char* all_func[0x3fff];
int func_cnt=0;
int ano_struct_cnt=0;
int ano_var_name_cnt=0;

const bool DEBUG=false;

void semanticAnalyse(TreeNode root){  //程序的开始
    if (DEBUG) printf("semanticAnalyse\n");
    initSymbolTable(SymbolTable);
    Program(root);
    checkFunDec(SymbolTable,all_func,func_cnt);
}

void Program(TreeNode node){
    if (DEBUG) printf("Program\n");
    /* Program -> ExtDefList */
    TreeNode p=node->children[0];
    ExtDefList(p);
}

void ExtDefList(TreeNode node){
    if (DEBUG) printf("ExtDefList\n");
    /* ExtDefList -> empty */
    if (node==NULL)
        return;
    assert(node->children_num==2);
    /* ExtDefList -> ExtDef ExtDefList */
    ExtDef(node->children[0]);
    ExtDefList(node->children[1]);
}

void ExtDef(TreeNode node){
    if (DEBUG) printf("ExtDef\n");
    assert(node->children_num==2 || node->children_num==3);
    /* ExtDef -> Specifier SEMI */
    if (node->children_num==2){                              
        Type t=Specifier(node->children[0]);
    }
    /* ExtDef -> Specifier ExtDecList SEMI */
    else if (strcmp(node->children[1]->name,"ExtDecList")==0){  
        Type t=Specifier(node->children[0]); 
        ExtDecList(node->children[1],t);  //将ExtDecList中的变量定义都赋值为t类型 并插入符号表
    }
    /* ExtDef -> Specifier FunDec CompSt */
    else if (strcmp(node->children[2]->name,"CompSt")==0){ 
        FieldList f=(FieldList)malloc(sizeof(struct FieldList_));
        Type t=(Type)malloc(sizeof (struct Type_));
        f->type=t;
        f->tail=NULL;
        f->type->kind=FUNCTION;
        f->type->u.function.ret=Specifier(node->children[0]);
        f->type->u.function.defined_sign=true;
        f->type->u.function.line=node->line;
        FunDec(node->children[1],f,false);
        CompSt(node->children[2],f->type->u.function.ret);
    }
    /* ExtDef -> Specifier FunDec Semi */
    else {                                                  //声明了一个函数  int main();
        FieldList f=(FieldList)malloc(sizeof(struct FieldList_));
        Type t=(Type)malloc(sizeof (struct Type_));
        f->type=t;
        f->tail=NULL;
        f->type->kind=FUNCTION;
        f->type->u.function.ret=Specifier(node->children[0]);
        f->type->u.function.defined_sign=false;
        f->type->u.function.line=node->line;
        FunDec(node->children[1],f,true);
    }
}

void ExtDecList(TreeNode node,Type type){
    if (DEBUG) printf("ExtDecList\n");
    assert(node->children_num==1 || node->children_num==3);
    /* ExtDecList -> VarDec */
    if (node->children_num==1){
        VarDec(node->children[0],type);
    }
    /* ExtDecList -> VarDec COMMA ExtDecList */
    else{
        VarDec(node->children[0],type);
        ExtDecList(node->children[2],type);
    }
}

Type Specifier(TreeNode node){
    if (DEBUG) printf("Specifier\n");
    assert(node->children_num==1);
    TreeNode p=node->children[0];
    /* Specifier -> TYPE */
    if (strcmp(p->name,"TYPE")==0){                         //基本类型
        Type t=(Type)malloc(sizeof (struct Type_));
        t->kind=BASIC;
        if (strcmp(p->val.val_str,"int")==0)
            t->u.basic=BASIC_INT;
        else
            t->u.basic=BASIC_FLOAT;
        return t;
    }
    /* Specifier -> StructSpecifier */
    else{                                                   //结构体
        return StructSpecifier(p);
    }
}

Type StructSpecifier(TreeNode node){
    if (DEBUG) printf("StructSpecifier\n");
    assert(node->children_num==2 || node->children_num==5);
    /* StructSpecifier -> STRUCT OptTag LC DefList RC */
    if (node->children_num==5){ // 定义了一个新的结构体
        char *name=OptTag(node->children[1]);
        if (name!=NULL){   
            if (lookUp(SymbolTable,name)!=NULL)      
                printErrorInfo(16,node->line,name);   // 结构体重复定义
            else{  
                Type type_tag=(Type)malloc(sizeof(struct Type_));
                type_tag->kind=STRUCTURETAG;
                type_tag->u.structure_tag=NULL;
                FieldList f=(FieldList)malloc(sizeof(struct FieldList_));
                f->name=name;
                f->type=type_tag;
                f->tail=NULL;
                DefListStruct(node->children[3],type_tag);
                insertTable(SymbolTable,f);

                Type t1=(Type)malloc(sizeof(struct Type_));
                t1->kind=STRUCTURE;
                t1->u.structure=type_tag->u.structure_tag;
                return t1;
            }

        }
        else{               //   定义了一个匿名的结构体 
            sprintf(name,"%d",ano_struct_cnt);
            Type type_tag=(Type)malloc(sizeof(struct Type_));
            type_tag->kind=STRUCTURETAG;
            type_tag->u.structure_tag=NULL;
            FieldList f=(FieldList)malloc(sizeof(struct FieldList_));
            f->name=name;
            f->type=type_tag;
            f->tail=NULL;
            DefListStruct(node->children[3],type_tag);
            insertTable(SymbolTable,f);
            ano_struct_cnt++; // 记录一个匿名函数后就将计数器+1 确保匿名的名字不会重复

            Type t1=(Type)malloc(sizeof(struct Type_));
            t1->kind=STRUCTURE;
            t1->u.structure=type_tag->u.structure_tag;
            return t1;
        }
    }
    /* StructSpecifier -> STRUCT Tag */      // 使用了一个已定义结构体
    else{  
        char *name=Tag(node->children[1]);
        FieldList f=lookUp(SymbolTable,name);
        if (f!=NULL){
            Type t=(Type)malloc(sizeof(struct Type_));
            t->kind=STRUCTURE;
            t->u.structure=f->type->u.structure_tag;
            return t;
        }
        else{
            printErrorInfo(17,node->line,name);  // #17 直接使用了未定义的结构体
        }
    }
}

char* OptTag(TreeNode node){
    if (DEBUG) printf("OptTag\n");
    /* OptTag -> empty */
    if (node==NULL)
        return NULL;
    assert(node->children_num==1);
    /* OptTag -> ID */
    return semanticID(node->children[0]);
}

char* Tag(TreeNode node){
    if (DEBUG) printf("Tag\n");
    assert(node->children_num==1);
    /* Tag -> ID */
    return semanticID(node->children[0]);
}

void VarDecFunc(TreeNode node,Type type,FieldList field){
    if (DEBUG) printf("VarDecFunc\n");
    assert(node->children_num==1 || node->children_num==4);
    /* VarDec -> ID */
    if (node->children_num==1){
        char *name=semanticID(node->children[0]);
        FieldList f=(FieldList)malloc(sizeof(struct FieldList_));
        f->name=name;
        f->type=type;
        f->tail=NULL;
        insertValList(field,f);
        insertTable(SymbolTable,f);
    }  
    /* VarDec -> VarDec LB INT RB */
    else{
        Type t=(Type)malloc(sizeof(struct Type_));
        t->kind=ARRAY;
        t->u.array.elem=type;
        t->u.array.size=semanticINT(node->children[2]);
        VarDecFunc(node->children[0],t,field);
    }
}

void VarDecStruct(TreeNode node,Type type,Type struct_type){
    if (DEBUG) printf("VarDecStruct\n");
    assert(node->children_num==1 || node->children_num==4);
    /* VarDec -> ID */
    if (node->children_num==1){
        char *name=semanticID(node->children[0]);
        if (inStructType(struct_type,name)){
            printErrorInfo(15,node->line,name); // #15 与结构体中已定义的域重名
            char ano_var_name[32];
            sprintf(name,"%d",ano_var_name_cnt);
            FieldList f=(FieldList)malloc(sizeof(struct FieldList_));
            f->name=ano_var_name;
            f->type=type;
            f->tail=NULL;

            // 与结构体中已有变量重名，给定一个匿名的名字以便递归逻辑正常运行，但无需插入符号表
        }
        else if (lookUp(SymbolTable,name)!=NULL){
            printErrorInfo(16,node->line,name); // #16 与其他变量或者结构体重名
            char ano_var_name[32];
            sprintf(name,"%d",ano_var_name_cnt);
            FieldList f=(FieldList)malloc(sizeof(struct FieldList_));
            f->name=ano_var_name;
            f->type=type;
            f->tail=NULL;

        }
        else{
            FieldList f=(FieldList)malloc(sizeof(struct FieldList_));
            f->name=name;
            f->type=type;
            f->tail=NULL;
            insertStruct(struct_type,f);
            insertTable(SymbolTable,f);
        }
    }
    /* VarDec -> VarDec LB INT RB */
    else{
        Type t=(Type)malloc(sizeof(struct Type_));
        t->kind=ARRAY;
        t->u.array.elem=type;
        t->u.array.size=semanticINT(node->children[2]);
        VarDecStruct(node->children[0],t,struct_type);
    }
}

void insertStruct(Type struct_type,FieldList f){
    if (DEBUG) printf("insertStruct\n");
    FieldList p=struct_type->u.structure_tag;
    if (p==NULL){
        struct_type->u.structure_tag=f;
    }
    else{
        while(p->tail!=NULL){
            p=p->tail;
        }
        p->tail=f;
    }
}

void insertValList(FieldList field,FieldList f){
    if (DEBUG) printf("insertValList\n");
    FieldList p=field->type->u.function.paras;
    if (p==NULL){
        field->type->u.function.paras=f;
    }
    else{
        while(p->tail!=NULL){
            p=p->tail;
        }
        p->tail=f;
    }

}

bool inStructType(Type struct_type,char* name){  // 判断结构体中是否已经有同名域
    if (DEBUG) printf("inStructType\n");
    FieldList p=struct_type->u.structure_tag;
    while(p!=NULL){
        if (DEBUG) printf("%s",p->name);
        if (strcmp(p->name,name)==0)
            return true;
        p=p->tail;
    }
    return false;
}

void VarDec(TreeNode node,Type type){
    if (DEBUG) printf("VarDec\n");
    assert(node->children_num==1 || node->children_num==4);
    /* VarDec -> ID */
    if (node->children_num==1){
        char *name=semanticID(node->children[0]);
        if (lookUp(SymbolTable,name)!=NULL)
            printErrorInfo(3,node->line,name);          // #3 变量出现了重复定义
        else{
            FieldList f=(FieldList)malloc(sizeof(struct FieldList_));
            f->name=name;
            f->type=type;
            f->tail=NULL;
            insertTable(SymbolTable,f);
        }
    }  
    /* VarDec -> VarDec LB INT RB */
    else{
        Type t=(Type)malloc(sizeof(struct Type_));
        t->kind=ARRAY;
        t->u.array.elem=type;
        t->u.array.size=semanticINT(node->children[2]);
        VarDec(node->children[0],t);
    }
}

void FunDec(TreeNode node,FieldList field,bool dec_flag){ //dec_flag为true表示是声明，否则则为定义
    if (DEBUG) printf("FunDec\n");
    assert(node->children_num==3 || node->children_num==4);
    field->type->u.function.para_num=0;
    field->type->u.function.paras=NULL;
    char* func_name=semanticID(node->children[0]);
    if (DEBUG) printf("func_name:%s\n",func_name);
    field->name=func_name;
    FieldList exist_func=lookUp(SymbolTable,func_name);
    /* FunDec -> ID LP VarList RP */
    if (node->children_num==4){  
        if (exist_func==NULL){
            VarList(node->children[2],field);
            all_func[func_cnt]=func_name;
            //strcpy(all_func[func_cnt],func_name);
            func_cnt++;
            insertTable(SymbolTable,field);
        }
        else{
            if (dec_flag){  //多次声明 检测是否匹配
                VarList(node->children[2],field);
                if (!(exist_func->type->kind==FUNCTION && 
                typeEqual(exist_func->type->u.function.ret,field->type->u.function.ret) &&
                valListEqual(exist_func->type->u.function.paras,field->type->u.function.paras))){
                    printErrorInfo(19,node->line,field->name);  // #19 函数的多次声明发生冲突
                }
            }
            else{
                if (exist_func->type->u.function.defined_sign){  
                    printErrorInfo(4,node->line,field->name);  // #4 函数发生重复定义
                }
                else{
                    VarList(node->children[2],field);
                    if (!(exist_func->type->kind==FUNCTION && 
                    typeEqual(exist_func->type->u.function.ret,field->type->u.function.ret) &&
                    valListEqual(exist_func->type->u.function.paras,field->type->u.function.paras))){
                        printErrorInfo(19,node->line,field->name);  // #19 函数的声明与定义发生冲突
                    }
                    else{
                        exist_func->type->u.function.defined_sign=true;
                    }
                }
             }
        }
    }
    /* FunDec -> ID LP RP */
    else{
        if (exist_func==NULL){
            all_func[func_cnt]=func_name;
            //strcpy(all_func[func_cnt],func_name);
            func_cnt++;
            insertTable(SymbolTable,field);
        }
        else{
            if (dec_flag){  //多次声明 检测是否匹配
                VarList(node->children[2],field);
                if (!(exist_func->type->kind==FUNCTION && 
                typeEqual(exist_func->type->u.function.ret,field->type->u.function.ret) &&
                valListEqual(exist_func->type->u.function.paras,field->type->u.function.paras))){
                    printErrorInfo(19,node->line,field->name);  // #19 函数的多次声明发生冲突
                }
            }
            else{
                if (exist_func->type->u.function.defined_sign){  
                    printErrorInfo(4,node->line,field->name);  // #4 函数发生重复定义
                }
                else{
                    VarList(node->children[2],field);
                    if (!(exist_func->type->kind==FUNCTION && 
                    typeEqual(exist_func->type->u.function.ret,field->type->u.function.ret) &&
                    valListEqual(exist_func->type->u.function.paras,field->type->u.function.paras))){
                        printErrorInfo(19,node->line,field->name);  // #19 函数的声明与定义发生冲突
                    }
                    else{
                        exist_func->type->u.function.defined_sign=true;
                    }
                }
                
             }
        }
        return ;
    }
}

void VarList(TreeNode node,FieldList field){
    if (DEBUG) printf("VarList\n");
    /* VarList -> ParamDec COMMA VarList */
    if (node->children_num==3){
        ParamDec(node->children[0],field);
        VarList(node->children[1],field);
    }
    /* VarList -> ParamDec */
    else{
        field->type->u.function.para_num++;
        ParamDec(node->children[0],field);
    }
}



void ParamDec(TreeNode node,FieldList field){
    if (DEBUG) printf("ParamDec\n");
    assert(node->children_num==2);
    /* ParamDec -> Specifier VarDec */
    Type t=Specifier(node->children[0]);
    VarDecFunc(node->children[1],t,field);
}

void CompSt(TreeNode node,Type type_ret){
    if (DEBUG) printf("CompSt\n");
    assert(node->children_num==4);
    /* CompSt -> LC DefList StmtList RC */
    DefList(node->children[1]);
    StmtList(node->children[2],type_ret);
}

void StmtList (TreeNode node,Type type_ret){
    if (DEBUG) printf("StmtList\n");
    /* StmtList -> empty */
    if (node==NULL)
        return ;
    assert(node->children_num==2);
    /* StmtList -> Stmt StmtList */
    Stmt(node->children[0],type_ret);
    StmtList(node->children[1],type_ret);
}

void Stmt(TreeNode node,Type type_ret){
    if (DEBUG) printf("Stmt\n");
    /* Stmt -> Exp SEMI */
    if (node->children_num==2){
        Exp(node->children[0]);
    }
    /* Stmt -> CompSt */
    else if (node->children_num==1){
        CompSt(node->children[0],type_ret);
    }

    /* Stmt -> RETURN Exp SEMI */
    else if (node->children_num==3){
        if (!typeEqual(Exp(node->children[1]),type_ret))
            printErrorInfo(8,node->line,"error8");   // #8 返回值与函数类型不匹配
    }
    /* Stmt -> IF LP Exp RP Stmt ELSE Stmt */
    else if (node->children_num==7){
        Exp(node->children[2]);
        Stmt(node->children[4],type_ret);
        Stmt(node->children[6],type_ret);
    }
    /* Stmt -> WHILE LP Exp RP Stmt */
    else if (strcmp(node->children[0]->name,"WHILE")==0){
        Exp(node->children[2]);
        Stmt(node->children[4],type_ret);
    }
    /* Stmt -> IF LP Exp RP Stmt %prec LOWER_THAN_ELSE */
    else {
        Exp(node->children[2]);
        Stmt(node->children[4],type_ret);
    }
}

void DefListStruct(TreeNode node,Type type_struct){
    if (DEBUG) printf("DefListStruct\n");
    /* DefList -> empty */
    if (node==NULL)
        return ;
    assert(node->children_num==2);
    /* DefList -> Def DefList */
    DefStruct(node->children[0],type_struct);
    DefListStruct(node->children[1],type_struct);
}

void DefList(TreeNode node){ 
    if (DEBUG) printf("DefList\n");
    /* DefList -> empty */
    if (node==NULL)
        return ;
    assert(node->children_num==2);
    /* DefList -> Def DefList */
    Def(node->children[0]);
    DefList(node->children[1]);
}
void DefStruct(TreeNode node,Type type_struct){
    if (DEBUG) printf("DefStruct\n");
    /* Def -> Specifier DecList SEMI */
    Type t=Specifier(node->children[0]);
    DecListStruct(node->children[1],t,type_struct);
}

void Def(TreeNode node){
    if (DEBUG) printf("Def\n");
    /* Def -> Specifier DecList SEMI */
    Type t=Specifier(node->children[0]);
    DecList(node->children[1],t);
}

void DecListStruct(TreeNode node,Type type,Type type_struct){
    if (DEBUG) printf("DecListStruct\n");
    assert(node->children_num==1 || node->children_num==3);
    /* DecList -> Dec */
    if (node->children_num==1){
        DecStruct(node->children[0],type,type_struct);
    }
    /* DecList -> Dec COMMA DecList */
    else{
        DecStruct(node->children[0],type,type_struct);
        DecListStruct(node->children[2],type,type_struct);
        
    }
}

void DecList(TreeNode node,Type type){
    if (DEBUG) printf("DecList\n");
    assert(node->children_num==1 || node->children_num==3);
    /* DecList -> Dec */
    if (node->children_num==1){
        Dec(node->children[0],type);
    }
    /* DecList -> Dec COMMA DecList */
    else{
        Dec(node->children[0],type);
        DecList(node->children[2],type);
    }
}

void DecStruct(TreeNode node,Type type,Type type_struct){
    if (DEBUG) printf("DecStruct\n");
    assert(node->children_num==1 || node->children_num==3);
    /* Dec -> VarDec */
    if (node->children_num==1){
       VarDecStruct(node->children[0],type,type_struct);
    }
    /* Dec -> VarDec ASSIGNOP Exp */
    else{
        printErrorInfo(15,node->line,NULL);  // #15 结构体内定义变量不允许初始化
    }
}

void Dec(TreeNode node,Type type){
    if (DEBUG) printf("Dec\n");
    assert(node->children_num==1 || node->children_num==3);
    /* Dec -> VarDec */
    if (node->children_num==1){
        VarDec(node->children[0],type);
    }
    /* Dec -> VarDec ASSIGNOP Exp */
    else{
        VarDec(node->children[0],type);
        if (!typeEqual(type,Exp(node->children[2])))
            printErrorInfo(5,node->line,"error5");  // #5 赋值号两边表达式类型不同
    }
}

Type Exp(TreeNode node){
    /* Exp -> Exp ASSIGNOP EXP */
    if (node->children_num==3 && strcmp(node->children[1]->name,"ASSIGNOP")==0){
        if (DEBUG) printf("Exp ASSIGNOP EXP\n");
        if (!isLValue(node->children[0])){
            printErrorInfo(6,node->line,"error6"); // #6 赋值号左边出现一个只有右值的表达式
        }
        Type t1=Exp(node->children[0]);
        Type t2=Exp(node->children[2]);
        if (!typeEqual(t1,t2))
            printErrorInfo(5,node->line,"error5");   // #5 赋值号两边表达式类型不同
        return t1;
    }
    /* Exp -> Exp AND Exp */
    
    else if (node->children_num==3 && strcmp(node->children[1]->name,"AND")==0){
        if (DEBUG) printf("Exp AND EXP\n");
        Type t1=Exp(node->children[0]);
        Type t2=Exp(node->children[2]);
        return t1;
    }
    /* Exp -> Exp OR Exp */
    else if (node->children_num==3 && strcmp(node->children[1]->name,"OR")==0){
        if (DEBUG) printf("Exp OR EXP\n");
        Type t1=Exp(node->children[0]);
        Type t2=Exp(node->children[2]);
        return t1;
    }
    /* Exp -> Exp RELOP Exp */
    else if (node->children_num==3 && strcmp(node->children[1]->name,"RELOP")==0){
        if (DEBUG) printf("Exp RELOP EXP\n");
        Type t1=Exp(node->children[0]);
        Type t2=Exp(node->children[2]);
        return t1;
    }

    /* Exp -> Exp PLUS Exp */
    else if (node->children_num==3 && strcmp(node->children[1]->name,"PLUS")==0){
        if (DEBUG) printf("Exp PLUS EXP\n");
        Type t1=Exp(node->children[0]);
        Type t2=Exp(node->children[2]);
        if (!typeEqual(t1,t2)){
            printErrorInfo(7,node->line,"error7"); // #7 操作数类型不匹配
        }
        return t1;
    }
    /* Exp -> Exp MINUS Exp */
    else if (node->children_num==3 && strcmp(node->children[1]->name,"MINUS")==0){
        if (DEBUG) printf("Exp MINUS EXP\n");
        Type t1=Exp(node->children[0]);
        Type t2=Exp(node->children[2]);
        if (!typeEqual(t1,t2)){
            printErrorInfo(7,node->line,"error7"); // #7 操作数类型不匹配
        }
        return t1;
    }
    /* Exp -> Exp STAR Exp */
    else if (node->children_num==3 && strcmp(node->children[1]->name,"STAR")==0){
        if (DEBUG) printf("Exp STAR EXP\n");
        Type t1=Exp(node->children[0]);
        Type t2=Exp(node->children[2]);
        if (!typeEqual(t1,t2)){
            printErrorInfo(7,node->line,"error7"); // #7 操作数类型不匹配
        }
        return t1;
    }

    /* Exp -> Exp DIV Exp */
    else if (node->children_num==3 && strcmp(node->children[1]->name,"DIV")==0){
        if (DEBUG) printf("Exp DIV EXP\n");
        Type t1=Exp(node->children[0]);
        Type t2=Exp(node->children[2]);
        if (!typeEqual(t1,t2)){
            printErrorInfo(7,node->line,"error7"); // #7 操作数类型不匹配
        }
        return t1;
    }

    /* Exp -> LP Exp RP */
    else if (node->children_num==3 && strcmp(node->children[0]->name,"LP")==0){
        if (DEBUG) printf("LP EXP RP\n");
        return Exp(node->children[1]);
    }

    /* Exp -> MINUS Exp */
    else if (node->children_num==2 && strcmp(node->children[0]->name,"MINUS")==0){
        if (DEBUG) printf("MINUS EXP\n");
        return Exp(node->children[1]);
    }
    /* Exp -> NOT Exp */
    else if (node->children_num==2 && strcmp(node->children[0]->name,"NOT")==0){
        if (DEBUG) printf("NOT Exp\n");
        return Exp(node->children[1]);
    }
    /* Exp -> ID LP Args RP */   // func(a,b)
    else if (node->children_num==4 && strcmp(node->children[2]->name,"Args")==0){
        if (DEBUG)  printf("ID LP ARGS RP\n");
        char* func_name=semanticID(node->children[0]);
        FieldList f=lookUp(SymbolTable,func_name);
        if (f==NULL ||
            (f->type->kind==FUNCTION && f->type->u.function.defined_sign==false)){
                printErrorInfo(2,node->line,func_name); // #2 函数在调用时未定义
                return NULL;  
            }
        else if (f!=NULL && f->type->kind!=FUNCTION){
            printErrorInfo(11,node->line,func_name); // #11 对普通变量使用“(…)”或“()”（函数调用）操作符
        }
        else{
            if (valListEqual(f->type->u.function.paras,Args(node->children[2])))
                return f->type->u.function.ret;
            else{
                printErrorInfo(9,node->line,func_name); // #9 函数调用时参数列表与函数定义不匹配
            }
        }

    }
    /* Exp -> ID LP RP */       // func()
    else if (node->children_num==3 && strcmp(node->children[1]->name,"LP")==0){
        if (DEBUG) printf("ID LP RP\n");
        char* func_name=semanticID(node->children[0]);
        FieldList f=lookUp(SymbolTable,func_name);
        if (f==NULL ||
            (f->type->kind==FUNCTION && f->type->u.function.defined_sign==false)){
                printErrorInfo(2,node->line,func_name); // #2 函数在调用时未定义
                return NULL;  
            }
        else if (f!=NULL && f->type->kind!=FUNCTION){
            printErrorInfo(11,node->line,func_name); // #11 对普通变量使用“(…)”或“()”（函数调用）操作符
        }
            
        else{
            if (f->type->u.function.para_num!=0){
                printErrorInfo(9,node->line,func_name); // #9 函数调用时参数列表与函数定义不匹配
            }
            return f->type->u.function.ret;
        }
    }
    /* Exp -> Exp LB Exp RB */   // A[b]
    else if (node->children_num==4 && strcmp(node->children[1]->name,"LB")==0){
        if (DEBUG) printf("Exp LB EXP RB\n");
        Type t1=Exp(node->children[0]);
        if (t1->kind!=ARRAY){
            printErrorInfo(10,node->line,"error10"); // #10 对非数组型变量使用“[…]”（数组访问）操作符
            
        }
        Type t2=Exp(node->children[2]);
        if (t2->kind!=BASIC || t2->u.basic!=BASIC_INT){
            printErrorInfo(12,node->line,"error12"); // #12 数组访问操作符“[…]”中出现非整数
        }
        return t1->u.array.elem;
    }
    /* Exp -> Exp DOT ID */      //s.a
    else if (node->children_num==3 && strcmp(node->children[1]->name,"DOT")==0){
        if (DEBUG) printf("Exp DOT ID\n");
        Type t1=Exp(node->children[0]);
        if (t1->kind!=STRUCTURE){
            printErrorInfo(13,node->line,"error13"); // #13 对非结构体型变量使用“.”操作符
            return NULL;
        }
        else{
            char *name=semanticID(node->children[2]);
            if (!inStructType(t1,name)){
                printErrorInfo(14,node->line,name); // #14 访问结构体中未定义过的域
                return NULL;
            }
            return lookUp(SymbolTable,name)->type;
        }
    }   

    /* Exp -> ID */
    else if (strcmp(node->children[0]->name,"ID")==0){
        if (DEBUG) printf("ID\n");
        char* name=semanticID(node->children[0]);
        FieldList f=lookUp(SymbolTable,name);
        if (f==NULL){
            printErrorInfo(1,node->line,name);  // #1 变量在使用时未经定义
            return NULL;
        }
        else{
            return f->type;
        }
    }
    /* Exp -> INT */
    else if (strcmp(node->children[0]->name,"INT")==0){
        if (DEBUG) printf("INT\n");
        Type t=(Type)malloc(sizeof(struct Type_));
        t->kind=BASIC;
        t->u.basic=BASIC_INT;
        return t;
    }
    /* Exp -> FLOAT */
    else if (strcmp(node->children[0]->name,"FLOAT")==0){
        if (DEBUG) printf("FLOAT\n");
        Type t=(Type)malloc(sizeof(struct Type_));
        t->kind=BASIC;
        t->u.basic=BASIC_FLOAT;
        return t;
    }
}

FieldList Args(TreeNode node){
    if (DEBUG) printf("Args\n");
    assert(node->children_num==1 || node->children_num==3);
    /* Args -> Exp */
    if (node->children_num==1){
        FieldList f=(FieldList)malloc(sizeof(struct FieldList_));
        f->name=NULL;
        f->type=Exp(node->children[0]);
        f->tail=NULL;
        return f;
    }
    /* Args -> Exp COMMA Args */
    else{
        FieldList f=(FieldList)malloc(sizeof(struct FieldList_));
        f->name=NULL;
        f->type=Exp(node->children[0]);
        f->tail=Args(node->children[2]);
        return f;
    }
}

char* semanticID(TreeNode node){
    if (DEBUG) printf("semanticID\n");
    assert(node->data_type==TYPE_ID);
    return node->val.val_str;
}

int semanticINT(TreeNode node){
    if (DEBUG) printf("semanticINT\n");
    assert(node->data_type==TYPE_INT);
    return node->val.val_int;
}

float semanticFLOAT(TreeNode node){
    if (DEBUG) printf("semanticFLOAT\n");
    assert(node->data_type==TYPE_FLOAT);
    return node->val.val_float;
}

bool typeEqual(Type t1,Type t2){    //判断两个类型是否相同
    if (DEBUG) printf("typeEqual\n");
    if (t1==NULL && t2==NULL)
        return true;
    else if (t1==NULL || t2==NULL)
    {
        //printf("t1\n");
        return false;
    }
    else if (t1->kind!=t2->kind){
        //printf("t2\n");
        return false;
    }
    else{
        switch (t1->kind)
        {
        case BASIC:
            return (t1->u.basic==t2->u.basic);
        case ARRAY:
            return typeEqual(t1->u.array.elem,t2->u.array.elem);
        case STRUCTURE:
            return (strcmp(t1->u.structure->name,t2->u.structure->name)==0); // 名等价
        case FUNCTION:   // 输入不会出现这种情况
            return false;
        default:
            return false;
        }
    }
}

bool valListEqual(FieldList f1,FieldList f2){  // 判读两个FieldList是否相同
    if (DEBUG) printf("valListEqual\n");
    if (f1==NULL && f2==NULL){
        if (DEBUG) printf("f1=f2=NULL\n");
        return true;
    }
    else if (f1==NULL){
        if (DEBUG) printf("f1=NULL\n");
        return false;
    }
    else if (f2==NULL){
        if (DEBUG) printf("f2=NULL\n");
        return false;
    }
    else{
        if (typeEqual(f1->type,f2->type)){
            return valListEqual(f1->tail,f2->tail);
        }
        else
            return false;
    }
}

bool isLValue(TreeNode node){
    if (node->children_num==1){
        if (strcmp(node->children[0]->name,"ID")==0)
            return true;
    }
    else if (node->children_num==4){
        if (strcmp(node->children[1]->name,"LB")==0)
            return true;
    }
    else if (node->children_num==3){
        if (strcmp(node->children[1]->name,"DOT")==0)
            return true;
    }
    return false;
}

void checkFunDec(HashNode symbol_table,char**all_func,int func_cnt){
    if (DEBUG) printf("checkFunDec\n");
    for (int i=0;i<func_cnt;i++){
        if (DEBUG) printf("#%d: %s\n",i,all_func[i]);
        FieldList f=lookUp(symbol_table,all_func[i]);
        if (f->type->u.function.defined_sign==false){
            printErrorInfo(18,f->type->u.function.line,all_func[i]); // #18 函数进行了声明，但没有被定义
        }
    }
}

void printErrorInfo(int type,int line,char* message){
    switch (type)
    {
    case 1:
        printf("Error type 1 at Line %d: Undefined variable \"%s\".\n",line,message);
        break;
    case 2:
        printf("Error type 2 at Line %d: Undefined function \"%s\".\n",line,message);
        break;
    case 3:
        printf("Error type 3 at Line %d: Redefined variable \"%s\".\n",line,message);
        break;
    case 4:
        printf("Error type 4 at Line %d: ReDefined variable \"%s\".\n",line,message);
        break;
    case 5:
        printf("Error type 5 at Line %d: Type mismatched for assignment.\n",line);
        break;
    case 6:
        printf("Error type 6 at Line %d: The left-hand side of an assignment must be a variable.\n",line);
        break;
    case 7:
        printf("Error type 7 at Line %d: Type mismatched for operands.\n",line);
        break;
    case 8:
        printf("Error type 8 at Line %d: Type mismatched for return.\n",line);
        break;
    case 9:
        printf("Error type 9 at Line %d: Function \"%s\" is not applicable for arguments.\n",line,message);
        break;
    case 10:
        printf("Error type 10 at Line %d: is not an array.\n",line);
        break;
    case 11:
        printf("Error type 11 at Line %d: \"%s\" is not a function.\n",line,message);
        break;
    case 12:
        printf("Error type 12 at Line %d: is not an integer.\n",line);
        break;
    case 13:
        printf("Error type 13 at Line %d: Illegal use of \".\".\n",line);
        break;
    case 14:
        printf("Error type 14 at Line %d: Non-existent field \"%s\".\n",line,message);
        break;
    case 15:
        printf("Error type 15 at Line %d: Redefined field.\n",line);
        break;
    case 16:
        printf("Error type 16 at Line %d: Duplicated name \"%s\".\n",line,message);
        break;
    case 17:
        printf("Error type 17 at Line %d: Undefined structure \"%s\".\n",line,message);
        break;
    case 18:
        printf("Error type 18 at Line %d: Undefined function \"%s\".\n",line,message);
        break;
    default:
        printf("Error type 19 at Line %d: Inconsistent declaration of function \"%s\".\n",line,message);
        break;
    }
}