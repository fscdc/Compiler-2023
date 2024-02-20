#ifndef __MEM2REG_H__
#define __MEM2REG_H__

#include "Unit.h"
#include <assert.h>
#include <stack>
#include <queue>
#include <map>
#include <set>

/*
 * A. 对每个基本块B，确定其前置基本块集合Preds。
 * B. 对每个变量x，遍历Preds里的每个基本块以找出x最近的赋值点，记录在LatestDefs[x]字典中。
 * C. 在每个基本块B内，逐个处理Phi函数。
 * D. 对每个Phi函数中的变量x，利用LatestDefs[x]找到x的最新赋值，并更新Phi函数，添加对应的最新定义。
 */

/*
 * A. 进行遍历，识别出所有基本块内的加载指令和Phi函数，记录下这些指令和函数引用的内存地址。
 * B. 为基本块中使用到的每个内存地址引用分配一个新的寄存器变量，扩充函数的变量列表。
 * C. 在基本块内部，将所有涉及内存引用的指令更新为新分配的寄存器变量。
 * D. 对Phi函数进行相同的处理，将内存引用替换为对应的寄存器变量。
 * E. 移除所有未被引用的内存变量，以清理代码中的冗余部分。
 */
class Mem2Reg
{
private:
    Unit *unit;
    std::vector<AllocaInstruction *> allins;//存储所有分配指令的列表
    std::map<Operand *, std::stack<Operand *>> stacksforop;//每个操作数对应的操作数栈，用于重命名变量
    std::vector<BinaryInstruction *> addzeroins;//存储二元加法零操作指令的列表
    std::vector<PhiInstruction *> phiNodes;//存储Phi指令的列表

    //在函数中插入Phi指令
    void insertphi(Function *function);
    //重命名基本块中的变量
    void rename(BasicBlock *block);
    //重命名函数中的变量
    void rename(Function *function);
    //用于为变量创建一个新的名称，确保每个变量只被赋值一次
    Operand *newName(Operand *old)
    {
        //如果是临时的创建一个新的 Operand，否则直接复制原操作数
        Operand *new_name = old->getSymbolEntry()->isTemporary()
                                ? new Operand(new TemporarySymbolEntry(old->getSymbolEntry()->getType(), SymbolTable::getLabel()))
                                : new Operand(*old);
        stacksforop[old].push(new_name);//新创建的压入栈
        return new_name;
    }


    //清理函数中的加法指令
    void elimadd(Function *function);
    //条件分支指令优化
    void condop(Function *func)
    {
        //遍历函数 func 中的所有基本块 block
        for (auto &block : func->getBlockList())
        {
            //获取每个基本块的最后一条指令
            auto in = block->rbegin();
            //判断这条指令是否是条件分支
            if (in->isCond())
            {
                auto cond = (CondBrInstruction *)in;
                auto trueBlock = cond->getTrueBranch();
                auto falseBlock = cond->getFalseBranch();
                if (trueBlock == falseBlock)
                {
                    //如果真分支和假分支指向同一个基本块，则替换成无条件分支指令
                    block->removeSucc(trueBlock);
                    trueBlock->removePred(block);
                    new UncondBrInstruction(trueBlock, block);
                    block->remove(in);
                }
            }
        }
    }
    //检查并处理函数中的Phi节点
    void elimphinode(Function *function);

public:
    Mem2Reg(Unit *unit) : unit(unit) {}//构造函数
    void pass();
};

#endif