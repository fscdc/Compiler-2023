#include "Function.h"
#include "Unit.h"
#include "Type.h"
#include <list>
#include "BasicBlock.h"
#include <set>
#include "AsmBuilder.h"
//参考了https://blog.csdn.net/Dong_HFUT/article/details/121375025
extern FILE* yyout;

class BasicBlock;

Function::Function(Unit *u, SymbolEntry *s)
{
    u->insertFunc(this);
    entry = new BasicBlock(this);
    sym_ptr = s;
    parent = u;
    ((IdentifierSymbolEntry *)s)->setFunction(this);
}

Function::~Function()
{
    auto delete_list = block_list;
    for (auto &i : delete_list)
        delete i;
    parent->removeFunc(this);
}

// remove the basicblock bb from its block_list.
void Function::remove(BasicBlock *bb)
{
    block_list.erase(std::find(block_list.begin(), block_list.end(), bb));
}

void Function::output() const
{
    FunctionType* funcType = dynamic_cast<FunctionType*>(sym_ptr->getType());
    Type *retType = funcType->getRetType();

    // 添加函数参数输出逻辑，新版本输出
    std::vector<Type *> params = funcType->getParamsType();
    std::string paramstr;
    for (size_t i = 0; i < params.size(); ++i) {
        if (i > 0) {
            paramstr += ",";
        }
        paramstr += " " + params[i]->toStr() + " " + paramsOperand[i]->toStr();
    }

    std::string functionDeclaration = "define " + retType->toStr() + " " + sym_ptr->toStr() + "(" + paramstr + ") {\n";
    fprintf(yyout, "%s", functionDeclaration.c_str());
    
    std::set<BasicBlock *> v;
    std::list<BasicBlock *> q;
    q.push_back(entry);
    v.insert(entry);
    while (!q.empty())
    {
        auto bb = q.front();
        q.pop_front();
        bb->output();
        for (auto succ = bb->succ_begin(); succ != bb->succ_end(); succ++)
        {
            if (v.find(*succ) == v.end())
            {
                v.insert(*succ);
                q.push_back(*succ);
            }
        }
    }
    fprintf(yyout, "}\n");
}


int TreeNode::Num = 0;
//构建函数的DFS树
void Function::computeDFS()
{
    //重置节点编号和准备数据结构
    TreeNode::Num = 0;
    int len = block_list.size();
    fprintf(stderr,"func's blocknumber is %d\n",len);
    preOrder2DFS.resize(len);
    bool *visited = new bool[len]{};
    DFSTreeRoot = new TreeNode(entry);
    //将新创建的根节点存储在 preOrder2DFS 数组
    preOrder2DFS[DFSTreeRoot->num] = DFSTreeRoot;
    //调用 build 方法递归地构建DFS树
    buildDFS(DFSTreeRoot, visited);
    removeNullPointers(preOrder2DFS);
    delete[] visited;
}
//计算每个基本块的半支配（semi-dominance）节点，对直接支配节点的一种逼近
void Function::computeSdom()
{
    int len = block_list.size();
    sdoms.resize(len);
    int *ancestors = new int[len];
    for (int i = 0; i < len; i++)
    {
        sdoms[i] = i;
        ancestors[i] = -1;
    }
    //从DFS树的最后一个节点开始，逆向遍历DFS树，直到到达入口基本块
    for (auto it = preOrder2DFS.rbegin(); (*it)->block != entry; it++)
    {
        auto block = (*it)->block;
        int s = block->order;
        //遍历该基本块的所有前驱基本块
        for (auto it1 = block->pred_begin(); it1 != block->pred_end(); it1++)
        {
            //计算每个前驱的最小半支配者 
            int z = getlittle((*it1)->order, ancestors);
            //如果 z 的半支配值小于当前基本块的半支配值，则更新当前基本块的半支配值为 z 的值
            if (sdoms[z] < sdoms[s])
                sdoms[s] = sdoms[z];
        }
        //更新当前基本块的祖先信息
        ancestors[s] = (*it)->parent->num;
    }
    delete[] ancestors;
}
//计算每个基本块的立即支配者（Immediate Dominator）节点
void Function::computeIdom()
{
    int len = block_list.size();
    idoms.resize(len);
    DOMTreeRoot = new TreeNode(entry, 0);
    preOrder2dom.resize(len);
    preOrder2dom[entry->order] = DOMTreeRoot;
    idoms[entry->order] = 0;
    //从第二个基本块开始遍历DFS树中的所有节点（跳过入口基本块）
    for (auto it = preOrder2DFS.begin() + 1; it != preOrder2DFS.end(); it++)
    {
        //使用 LCA 函数找到当前基本块的父节点和其半支配者之间的最近公共祖先，即是立即支配者
        int p = LCA((*it)->parent->num, sdoms[(*it)->num]);
        idoms[(*it)->num] = p;
        //在支配树中添加节点
        auto parent = preOrder2dom[p];
        TreeNode *node = new TreeNode((*it)->block, 0);
        node->parent = parent;
        parent->addChild(node);
        preOrder2dom[(*it)->num] = node;
    }
}
//计算每个基本块的支配边界（Dominance Frontier）
void Function::computeDomFrontier()
{
    //遍历所有基本块
    for (auto block : block_list)
    {
        //处理有多个前驱的基本块
        if (block->getNumOfPred() >= 2)
        {
            // 遍历其所有前驱基本块
            for (auto it = block->pred_begin(); it != block->pred_end(); it++)
            {
                int runner = (*it)->order;
                fprintf(stderr,"runner is %d\n", runner);

                // 检查 runner 是否在合法范围内
                if (runner < 0 || runner >= (int)block_list.size()) {
                    fprintf(stderr, "Warning: runner (%d) is out of range, skipping\n", runner);
                    continue; // 跳过此次循环
                }

                while (runner != idoms[block->order])
                {
                    // 在这里，runner 的值已经保证是在合法范围内
                    preOrder2DFS[runner]->block->domFrontier.insert(block);
                    runner = idoms[runner];
                }
            }
        }
    }
}

//用于从函数中移除所有的Phi指令
//将非常规SSA形成为常规SSA的关键边分割算法
/*
关键边是指连接两个基本块的边，满足以下两个条件：
1. 前驱基本块有多个后继基本块：
即这个基本块通过不同的条件跳转到多个不同的后续基本块。
2. 后继基本块有多个前驱基本块：
即这个基本块可以从多个不同的基本块跳转过来。
*/
void Function::lowerphi()
{
    std::map<BasicBlock *, std::vector<Instruction *>> pcopy;
    auto blocks = std::vector<BasicBlock *>(this->begin(), this->end());
    //遍历函数中的每个基本块 bb
    for (auto bb : blocks)
    {
        //如果基本块的第一条指令不是Phi指令，跳过该基本块
        if (!bb->begin()->isPhi())
            continue;
        //获取当前基本块的所有前驱基本块，并存储在 preds
        auto preds = std::vector<BasicBlock *>(bb->pred_begin(), bb->pred_end());
        for (auto &pred : preds)
        {
            //如果前驱基本块有多个后继。处理关键边
            if (pred->getNumOfSucc() > 1)
            {
                BasicBlock *splitBlock = new BasicBlock(this);
                //获取前驱基本块 pred 的最后一条指令转为条件分支指令
                CondBrInstruction *branch = (CondBrInstruction *)(pred->rbegin());
                //根据条件分支指令的真假分支目标来更新分支目标为新创建的 splitBlock
                if (branch->getTrueBranch() == bb)
                    branch->setTrueBranch(splitBlock);
                else
                    branch->setFalseBranch(splitBlock);
                //更新前驱和后继基本块
                pred->addSucc(splitBlock);
                pred->removeSucc(bb);
                splitBlock->addPred(pred);
                new UncondBrInstruction(bb, splitBlock);
                splitBlock->addSucc(bb);
                bb->addPred(splitBlock);
                bb->removePred(pred);
                //遍历基本块的指令，替换Phi指令
                for (auto i = bb->begin(); i != bb->end() && i->isPhi(); i = i->getNext())
                {
                    //移除对Phi指令的使用，在映射 pcopy 中对应 splitBlock 的列表中添加一个新的二元加法指令
                    //这实际上是将Phi指令的功能移动到了新的 splitBlock 中
                    auto def = i->getDef();
                    auto src = ((PhiInstruction *)i)->getEdge(pred);
                    src->removeUse(i);
                    pcopy[splitBlock].push_back(new BinaryInstruction(
                        BinaryInstruction::ADD, def, src, new Operand(new ConstantSymbolEntry(def->getType(), 0))));
                }
            }
            //如果前驱基本块只有一个后继，处理仅需要进行替换Phi指令的部分同上
            else
            {
                for (auto i = bb->begin(); i != bb->end() && i->isPhi(); i = i->getNext())
                {
                    auto def = i->getDef();
                    auto src = ((PhiInstruction *)i)->getEdge(pred);
                    src->removeUse(i);
                    pcopy[pred].push_back(new BinaryInstruction(
                        BinaryInstruction::ADD, def, src, new Operand(new ConstantSymbolEntry(def->getType(), 0))));
                }
            }
        }
        //清除所有Phi指令
        while (bb->begin() != bb->end())
        {
            auto i = bb->begin();
            if (!i->isPhi())
                break;
            bb->remove(i);
        }
    }
    //遍历 pcopy 映射
    for (auto &&[block, ins] : pcopy)
    {
        //遍历指令列表 ins，将每个指令定义的操作数添加到 defs 集合中
        std::set<Operand *> defs;
        for (auto &in : ins)
            defs.insert(in->getDef());
        //遍历 ins，检查每个指令的第一个使用操作数是否在 defs 中
        //如果不在，将该指令移动到 temp 并从 ins 中移除
        std::vector<Instruction *> temp;
        for (auto it = ins.begin(); it != ins.end();)
        {
            if (defs.count((*it)->getUse()[0]) == 0)
            {
                temp.push_back(*it);
                it = ins.erase(it);
            }
            else
            {
                it++;
            }
        }
        //对于每个指令，检查 temp 中是否有使用它定义的操作数的指令
        //如果有，将当前指令插入到该使用指令之后
        //如果没有，将其插入到 temp 开头
        for (auto &in : ins)
        {
            bool flag = false;
            auto def = in->getDef();
            for (auto it = temp.begin(); it != temp.end(); it++)
            {
                if ((*it)->getUse()[0] == def)
                {
                    temp.insert(it + 1, in);
                    flag = true;
                    break;
                }
            }
            if (flag)
                continue;
            temp.insert(temp.begin(), in);
        }
        //将排序后的指令插入到对应的基本块
        auto endIns = block->rbegin();
        for (auto &in : temp)
            block->insertBefore(in, endIns);
    }
}

void Function::computeRDFSTree(BasicBlock *exit)
{
    //这个就类似之前的DFS，只不过是逆向的
    TreeNode::Num = 0;
    int len = block_list.size();
    preOrder2DFS.resize(len);
    bool *visited = new bool[len]{};
    DFSTreeRoot = new TreeNode(exit);
    preOrder2DFS[DFSTreeRoot->num] = DFSTreeRoot;
    reversebuild(DFSTreeRoot, visited);//调用reverseSearch来逆向递归构建的 
    removeNullPointers(preOrder2DFS);
    delete[] visited;
}

//逆向DFS搭建DFS树preOrder2DFS，过程类似正向的
void Function::reversebuild(TreeNode *node, bool *visited)
{
    int n = getIndex(node->block);
    visited[n] = true;
    auto block = block_list[n];
    for (auto it = block->pred_begin(); it != block->pred_end(); it++)
    {
        int idx = getIndex(*it);
        if (!visited[idx])
        {
            TreeNode *child = new TreeNode(*it);
            preOrder2DFS[child->num] = child;
            child->parent = node;
            node->addChild(child);
            reversebuild(child, visited);
        }
    }
}

//逆向直接支配
void Function::computeRIdom(BasicBlock *exit)
{
    int len = block_list.size();
    idoms.resize(len);
    DOMTreeRoot = new TreeNode(exit, 0);
    preOrder2dom.resize(len);
    preOrder2dom[exit->order] = DOMTreeRoot;
    idoms[exit->order] = 0;
    for (auto it = preOrder2DFS.begin() + 1; it != preOrder2DFS.end(); it++)
    {
        int p = LCA((*it)->parent->num, sdoms[(*it)->num]);
        idoms[(*it)->num] = p;
        auto parent = preOrder2dom[p];
        TreeNode *node = new TreeNode((*it)->block, 0);
        node->parent = parent;
        parent->addChild(node);
        preOrder2dom[(*it)->num] = node;
    }
}

void Function::computeRSdom(BasicBlock *exit)
{
    int len = block_list.size();
    sdoms.resize(len);

    // 创建一个数组来存储每个节点的祖先节点
    int *ancestors = new int[len];

    // 初始化sdoms和ancestors数组
    for (int i = 0; i < len; i++)
    {
        sdoms[i] = i; // 初始时，每个节点的半支配节点设为它自己
        ancestors[i] = -1; // 祖先节点初始设置为-1，表示没有祖先
    }

    // 以逆向前序的方式处理每一个节点
    for (auto it = preOrder2DFS.rbegin(); (*it)->block != exit; it++)
    {
        auto block = (*it)->block; // 获取当前基本块
        int s = block->order; // 获取当前基本块的序号

        // 遍历当前基本块的每一个后继（也就是逆向DFS的每一个前驱）
        for (auto it1 = block->succ_begin(); it1 != block->succ_end(); it1++)
        {
            // 获取当前后继节点的最小半支配节点
            int z = getlittle((*it1)->order, ancestors);

            // 更新半支配节点
            if (sdoms[z] < sdoms[s])
                sdoms[s] = sdoms[z];
        }

        // 更新祖先节点
        ancestors[s] = (*it)->parent->num;
    }

    // 释放祖先节点数组
    delete[] ancestors;
}

// 逆向支配边界计算
void Function::computeRDF()
{
    preOrder2DFS.clear();
    //给函数的每一个有ret语句的基本块都链接到同一个基本块exit
    BasicBlock *exit = new BasicBlock(this);
    for (auto b : block_list)
    {
        if (b->rbegin()->isRet())
        {
            b->addSucc(exit);
            exit->addPred(b);
        }
    }

    //逆向DFS，就是之前正向的反过来，用exit当成根来做，类似的
    computeRDFSTree(exit);
    computeRSdom(exit);
    computeRIdom(exit);
    for (auto block : block_list)
    {
        //检查有多个后继的基本块
        if (block->getNumOfSucc() >= 2)
        {
            //遍历后继节点
            for (auto it = block->succ_begin(); it != block->succ_end(); it++)
            {
                //计算支配边界
                int runner = (*it)->order;
                while (runner != idoms[block->order])
                {
                    preOrder2DFS[runner]->block->domFrontier.insert(block);
                    runner = idoms[runner];
                }
            }
        }
    }
    delete exit;
}

BasicBlock *Function::getMarkBranch(BasicBlock *block)
{
    std::set<BasicBlock *> blocks;
    while (1)
    {
        auto order = idoms[block->order];
        block = preOrder2dom[order]->block;
        if (blocks.count(block))
            return nullptr;
        blocks.insert(block);
        if (block->getMark())
            return block;
    }
}

void Function::addCallPred(Instruction *in)
{
    auto func = in->getParent()->getParent();
    this->callPreds.push_back(in);
}

void Function::genMachineCode(AsmBuilder* builder) 
{
    auto curunit = builder->getUnit();
    auto curfunc = new MachineFunction(curunit, this->sym_ptr);
    builder->setFunction(curfunc);
    std::map<BasicBlock *, MachineBlock *> reflect;

    for (auto block : block_list)
    {
        block->genMachineCode(builder);
        reflect[block] = builder->getBlock();
        if (block == this->getEntry())
            curfunc->setEntry(reflect[block]);
    }
    //建立前驱后继关系
    for (auto block : block_list)
    {
        auto mblock = reflect[block];
        for (auto pred = block->pred_begin(); pred != block->pred_end(); pred++)
            mblock->addPred(reflect[*pred]);
        for (auto succ = block->succ_begin(); succ != block->succ_end(); succ++)
            mblock->addSucc(reflect[*succ]);
    }
    curunit->InsertFunc(curfunc);
}