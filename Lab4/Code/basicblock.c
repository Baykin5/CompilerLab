#include"basicblock.h"
#include<assert.h>
#include<stdlib.h>
const bool DEBUG5=true;

int total_fb_num = 0, total_bb_num = 0;

void setUpBasicBlock(){
    initLeader();  // 设置好所有首指令
    initBasicBlock();  // 根据设置好的首指令信息建立基本块链表
    initFunctionBlock();
    initBBArray();
    initFBArray();

    //setVarUseLine();
    if (DEBUG5) printBBList();
    if (DEBUG5) printFBList();
}

void initLeader(){
    IRNode p=InterCodes;
    assert(p!=NULL);
    p->is_leader=true;  // 第一条指令
    while(p!=NULL){
        if (p->code->kind==IR_LABEL || p->code->kind==IR_FUNCTION){  // label结点一定是某个跳转指令的目标指令
            p->is_leader=true;
        }
        else if (p->code->kind==IR_GOTO || p->code->kind==IR_IF 
              || p->code->kind==IR_CALL || p->code->kind==IR_RETURN
              || p->code->kind==IR_READ || p->code->kind==IR_WRITE ){  // 跳转指令的下一个指令一定是首指令
            IRNode q=p->next;
            if (q!=NULL)
                q->is_leader=true;
        }
        p=p->next;
    }
}

void initBasicBlock(){
    IRNode p=InterCodes;
    while(p!=NULL){
        // 建立基本块
        BasicBlock bb=(BasicBlock)malloc(sizeof(struct BasicBlock_));  
        bb->block_no=total_bb_num;
        bb->head=p;
        p->code->basicblock_no=total_bb_num;
        while(p->next!=NULL && p->next->is_leader!=true){
            p->code->basicblock_no=total_bb_num;
            p=p->next;
        }
        bb->last=p;
        p->code->basicblock_no=total_bb_num;
        p=p->next;
        total_bb_num++;
        // 将建立的基本块插入基本块链表中
        BasicBlockList bblist=(BasicBlockList)malloc(sizeof(struct BasicBlockList_)); 
        bblist->basic_block=bb;
        bblist->next=NULL;
        if (BBList==NULL){
            BBList=bblist;
            bblist->prev=NULL;
        }
        else{
            BasicBlockList q=BBList;
            while(q->next!=NULL)
                q=q->next;
            q->next=bblist;
            bblist->prev=q;
        }
    }
}


void initFunctionBlock(){
    IRNode p=InterCodes;
    while(p!=NULL){
        // 建立函数块
        if (p->code->kind==IR_FUNCTION){
            FunctionBlock fb=(FunctionBlock)malloc(sizeof(struct FunctionBlock_));
            fb->fb_no=total_fb_num;
            fb->head_bb_no=p->code->basicblock_no;
            while(p->next!=NULL && p->next->code->kind!=IR_FUNCTION){
                p=p->next;
            }
            fb->last_bb_no=p->code->basicblock_no;
            fb->frame_size=0;  
            //strcpy(fb->func_name,p->code->u.one_op.op->u.func_name);
            total_fb_num++;
            // 将建立的函数块插入函数块链表中
            FunctionBlockList fblist=(FunctionBlockList)malloc(sizeof(struct FunctionBlockList_));
            fblist->func_block=fb;
            fblist->next=NULL;
            if (FBList==NULL){
                FBList=fblist;
                fblist->prev=NULL;
            }
            else{
                FunctionBlockList q=FBList;
                while(q->next!=NULL)
                    q=q->next;
                q->next=fblist;
                fblist->prev=q;
            }
        }
        p=p->next;
    }
}


void initBBArray(){
    // 建立便于访问的基本块链表数组
    BBArray=(BasicBlockList*)malloc(total_bb_num*sizeof(BasicBlock));
    BasicBlockList temp=BBList;
    for (int i=0;i<total_bb_num;i++){
        BBArray[i]=temp;
        temp=temp->next;
    }
}

void initFBArray(){
    // 建立便于访问的函数块链表数组
    FBArray=(FunctionBlockList*)malloc(total_fb_num*sizeof(FunctionBlock));
    FunctionBlockList temp=FBList;
    for (int i=0;i<total_fb_num;i++){
        FBArray[i]=temp;
        temp=temp->next;
    }
}


void printBBList(){
    /*
    BasicBlockList p =BBList;
    while(p!=NULL){
        BasicBlock bb=p->basic_block;
        printf("BasicBlock %d:\n",bb->block_no);
        printCode(bb->head->code);
        printCode(bb->last->code);
        p=p->next;
    }
    */
    
    
    for (int i=0;i<total_bb_num;i++){
        BasicBlockList bbl=BBArray[i];
        BasicBlock bb=bbl->basic_block;
        printf("BasicBlock %d:\n",bb->block_no);
        printCode(bb->head->code);
        printCode(bb->last->code);
    }
    
    
}

void printFBList(){
    /*
    FunctionBlockList p=FBList;
    while(p!=NULL){
        FunctionBlock fb=p->func_block;
        printf("FunctionBlock %d:\n",fb->fb_no);
        printf("head:%d last:%d\n",fb->head_bb_no,fb->last_bb_no);
        p=p->next;
    }
    */
    for (int i=0;i<total_fb_num;i++){
        FunctionBlock fb=FBArray[i]->func_block;
        printf("FunctionBlock %s:\n",fb->func_name);
        printf("head:%d last:%d\n",fb->head_bb_no,fb->last_bb_no);
    }
    
    
}