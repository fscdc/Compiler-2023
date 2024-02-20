#include "Mem2Reg.h"
#include "BasicBlock.h"
#include "Instruction.h"
#include "ParamHandler.h"
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <queue>
#include "Type.h"
using namespace std;

void Mem2Reg::insertphi(Function *function)
{
    addzeroins.clear();
    allins.clear();
    phiNodes.clear();
    vector<AllocaInstruction *> allarr;
    vector<Instruction *> uselessins;
    
    BasicBlock *entry = function->getEntry();
    //遍历入口基本块中的指令
    for (auto i = entry->begin(); i != entry->end(); i = i->getNext())
    {
        //如果不是分配指令跳出循环
        if (!i->isAlloca())
            break;
        auto alloca = (AllocaInstruction *)i;
        //如果不是数组或者指针，将其添加到 allins 列表
        if (!alloca->getEntry()->getType()->isArray() && !alloca->getEntry()->getType()->isPTR())
            allins.push_back(alloca);
        else
        {
            //获取使用这个分配指令的操作数的使用者列表
            auto &v = alloca->getDef()->getUse();
            //如果分配指令定义的操作数有使用的话
            if (!v.empty())
            {
                //如果第一个使用是存储指令，将该分配指令添加到 allarr
                if (v[0]->isStore())
                    allarr.push_back(alloca);
            }
            //如果没有使用情况，将该分配指令视为无用
            else
            {
                uselessins.push_back(alloca);
            }
        }
    }

    queue<BasicBlock *> worklist;
    unordered_set<BasicBlock *> inWorklist, inserted, assigns;
    //遍历所有分配指令
    for (auto &i : allins)
    {
        queue<BasicBlock *>().swap(worklist);
        inWorklist.clear();
        inserted.clear();
        assigns.clear();
        //获取当前指令的父基本块并从中移除该指令
        auto block = i->getParent();
        block->remove(i);
        //处理操作数并创建新的操作数
        auto operand = i->getDef();
        operand->setDef(nullptr);
        Operand *newOperand = new Operand(new TemporarySymbolEntry(
            ((PointerType *)(operand->getType()))->getValType(),
            SymbolTable::getLabel()));
        i->replaceDef(newOperand);
        //处理操作数的所有使用点
        while (operand->use_begin() != operand->use_end())
        {
            auto use = operand->use_begin();
            //如果使用点是一个存储指令
            if ((*use)->isStore())
            {
                //新旧操作数不同
                if (newOperand != (*use)->getUse()[1])
                {
                    //创建一个新的二元指令assignIns（添加操作），并将其插入到使用点之前
                    auto assignIns = new BinaryInstruction(BinaryInstruction::ADD, newOperand, (*use)->getUse()[1],new Operand(new ConstantSymbolEntry(newOperand->getType(), 0)));
                    addzeroins.push_back(assignIns);
                    (*use)->getParent()->insertBefore(assignIns, *use);
                    assigns.insert((*use)->getParent());
                    (*use)->getUse()[1]->removeUse(*use);
                }
            }
            //移除原使用点并更新操作数
            auto dst = (*use)->getDef();
            (*use)->getParent()->remove(*use);
            if (dst && dst != newOperand){
                while (dst->use_begin() != dst->use_end())
                {
                    auto u = *(dst->use_begin());
                    u->replaceUse(dst, newOperand);
                }
            }
            operand->removeUse(*use);
        }
        //处理影响到的基本块
        for (auto &block : assigns)
        {
            worklist.push(block);
            inWorklist.insert(block);
            while (!worklist.empty())
            {
                BasicBlock *n = worklist.front();
                worklist.pop();
                //对于每个基本块，如果尚未插入Phi指令，则在其前面插入一个新的Phi指令，并更新相关集合和列表。
                for (auto m : n->domFrontier)
                {
                    if (inserted.find(m) == inserted.end())
                    {
                        auto phi = new PhiInstruction(newOperand);
                        phiNodes.push_back(phi);
                        m->insertFront(phi, false);
                        inserted.insert(m);
                        if (inWorklist.find(m) == inWorklist.end())
                        {
                            inWorklist.insert(m);
                            worklist.push(m);
                        }
                    }
                }
            }
        }
    }
    //遍历allarr列表中的每个元素
    for (auto &alloca : allarr)
    {
        auto defArray = alloca->getDef();
        auto storeArray = alloca->getDef()->getUse()[0]; // parameter => defArray
        //如果storeArray的第一个使用（即alloca和store之后）没有其他用途，那么这些指令被视为无用
        if (storeArray->getUse()[0]->getUse().size() == 1)
        {
            uselessins.push_back(storeArray);
            uselessins.push_back(alloca);
            continue;
        }
        //处理剩余的指令
        auto paramArray = storeArray->getUse()[1];
        auto bitcast = new BitcastInstruction(defArray, paramArray); 
        entry->insertBefore(bitcast, storeArray);                  
        entry->remove(storeArray);
        delete storeArray; 
        entry->remove(alloca);
        delete alloca; 
        defArray->setDef(bitcast);
        auto uses = defArray->getUse(); 
        for (auto &&load_inst : uses)
        {
            if (load_inst == bitcast || !load_inst->isLoad())
                continue;
            auto use_def = load_inst->getDef();
            for (auto &&i : use_def->getUse())
            {
                i->replaceUse(use_def, defArray);
            }
            load_inst->getParent()->remove(load_inst);
            delete load_inst;
        }
        bitcast->getDef()->getSymbolEntry()->setType(paramArray->getType());
    }
    for (auto inst : uselessins)
    {
        entry->remove(inst);
        delete inst;
    }
}

//重命名函数中的变量
void Mem2Reg::rename(Function *function)
{
    stacksforop.clear();
    //遍历所有分配指令，将其定义操作数存起来
    for (auto &i : allins)
    {
        auto operand = i->getDef();
        stacksforop[operand] = stack<Operand *>();
    }
    //调用另一个重载函数来具体重命名
    rename(function->getEntry());
}
//具体的重命名函数，传参是函数的入口基本块
void Mem2Reg::rename(BasicBlock *block)
{
    std::unordered_map<Operand *, int> counter;//跟踪每个操作数在当前基本块中被重新定义的次数
    //遍历基本块中的每个指令
    for (auto i = block->begin(); i != block->end(); i = i->getNext())
    {
        Operand *def = i->getDef();
        //获取当前指令定义的操作数，替换当前指令中定义的操作数为新命名的操作数
        if (def && stacksforop.find(def) != stacksforop.end())
        {
            counter[def]++;
            Operand *new_ = newName(def);
            i->replaceDef(new_);
        }
        //如果当前指令不是Phi指令，遍历其所有使用的操作数
        if (!i->isPhi())
            for (auto &u : i->getUse())
                //如果操作数在 stacksforop 中有记录且栈不为空，则用栈顶元素（最新的操作数）替换当前指令中的使用
                if (stacksforop.find(u) != stacksforop.end() && !stacksforop[u].empty())
                    i->replaceUse(u, stacksforop[u].top());
    }
    //遍历当前基本块的所有后继基本块，为每个基本块的Phi指令添加适当的边
    for (auto it = block->succ_begin(); it != block->succ_end(); it++)
    {
        for (auto i = (*it)->begin(); i != (*it)->end(); i = i->getNext())
        {
            if (!i->isPhi())
                break;
            auto phi = (PhiInstruction *)i;
            Operand *o = phi->getAddr();
            //如果操作数栈不为空，则使用栈顶元素；否则，使用新创建的对应类型的0值
            if (!stacksforop[o].empty())
            {
                phi->addEdge(block, stacksforop[o].top());
            }
            else
            {
                phi->addEdge(block, new Operand(new ConstantSymbolEntry(o->getType(), 0)));
            }
        }
    }
    //递归重命名子节点
    auto func = block->getParent();
    auto node = func->getDomNode(block);
    for (auto &child : node->children)
        rename(child->block);
    //清理栈
    for (auto &it : counter)
        for (int i = 0; i < it.second; i++)
            stacksforop[it.first].pop();
}




//清理函数中添加的零操作指令
void Mem2Reg::elimadd(Function *func)
{
    //遍历 addzeroins 集合中的所有零操作指令
    for (auto i : addzeroins)
    {
        //获取当前指令的使用的操作数
        auto use = i->getUse()[0];
        //如果当前指令是基本块的第一条指令且下一条指令是无条件跳转，则跳过当前指令
        if (i->getParent()->begin() == i && i->getNext()->isUncond())
        {
            continue;
        }
        //如果使用的操作数是一个变量，则跳过当前指令
        if (use->getSymbolEntry()->isVariable())
        {
            continue;
        }
        //获取当前指令定义的操作数
        auto def = i->getDef();
        //遍历 def 的所有使用点，将它们的使用从 def 替换为 use
        while (def->use_begin() != def->use_end())
        {
            auto u = *(def->use_begin());
            u->replaceUse(def, use);
        }
        //从其所在的基本块中移除当前指令
        i->getParent()->remove(i);
        //移除 use 对当前指令的使用关系
        use->removeUse(i);
        delete i;
    }
}
//检查并清理函数中的Phi节点
void Mem2Reg::elimphinode(Function *function)
{
    //遍历在 Mem2Reg 过程中收集的所有Phi节点（存储在 phiNodes 集合中）
    for (auto it = phiNodes.begin(); it != phiNodes.end(); it++)
    {
        auto &phi = *it;
        //检查 phi 定义的操作数是否没有被使用
        if (phi->getDef()->getUse().empty())
        {   
            //获取Phi节点的所有源操作数 srcs
            auto &srcs = phi->getOperands();
            //遍历这些源操作数，从每个操作数的使用列表中移除对当前Phi节点的引用
            for (size_t i = 1; i < srcs.size(); i++)
            {
                srcs[i]->removeUse(phi);
            }
            //从Phi节点所在的基本块中移除该Phi节点
            phi->getParent()->remove(phi);
            delete phi;
        }
    }
}

void Mem2Reg::pass()
{
    //遍历程序中的所有函数
    for (auto it = unit->begin(); it != unit->end(); it++)
    {
        //条件分支指令优化，这个小优化直接一个函数写进来了，要什么pass
        condop(*it);
        //计算一下DFS和DOM，重点是DOM计算（reference：PPT第十章）
        (*it)->computeDFS();
        (*it)->computeSdom();
        (*it)->computeIdom();
        (*it)->computeDomFrontier();

        //具体开始处理了
        insertphi(*it);
        rename(*it);
        elimadd(*it);
        elimphinode(*it);
    }
    auto ph = new ParamHandler(unit);
    ph->pass();//处理一下参数，这地方是在另一个类里实现的，ParamHandler这个类
}