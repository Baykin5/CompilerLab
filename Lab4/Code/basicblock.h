#ifndef BASIC_BLOCK
#define BASIC_BLOCK

#include "intercode.h"

typedef struct BasicBlock_* BasicBlock;
typedef struct BasicBlockList_* BasicBlockList;
typedef struct FunctionBlock_* FunctionBlock;
typedef struct FunctionBlockList_* FunctionBlockList;


BasicBlockList BBList;    // 基本块链表头结点
FunctionBlockList FBList; // 函数块链表头结点

BasicBlockList* BBArray; // 基本块链表数组 便于访问
FunctionBlockList* FBArray;  // 函数块链表数组 便于访问


int total_fb_num,total_bb_num;

struct BasicBlock_{  // 基本块
    int block_no;

    IRNode head;
    IRNode last;
};

struct FunctionBlock_{  // 函数块
    int head_bb_no;  // 头基本块编号
    int last_bb_no;  // 尾基本块编号

    int fb_no;  // 函数块编号
    char func_name[32]; // 函数名称
    int frame_size;  // 函数栈帧的大小

};


struct BasicBlockList_{
    BasicBlock basic_block;
    BasicBlockList prev;
    BasicBlockList next;
};

struct FunctionBlockList_{
    FunctionBlock func_block;
    FunctionBlockList prev;
    FunctionBlockList next;
};

void setUpBasicBlock();  // 根据中间代码链表建立基本块链表
void initLeader();
void initBasicBlock();
void initFunctionBlock();
void initBBArray();
void initFBArray();

void printBBList();
void printFBList();

#endif