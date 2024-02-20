#include "GraphColor.h"
#include "Type.h"
#include <queue>
extern FILE *yyout;

#define LOOPFIRST


void GraphColor::aggregate(std::set<MachineOperand *> &a, std::set<MachineOperand *> &b, std::set<MachineOperand *> &out)
{
    out.clear();
    std::set_union(a.begin(), a.end(), b.begin(), b.end(), inserter(out, out.begin()));
}

void GraphColor::aggregate(std::set<int> &a, std::set<int> &b, std::set<int> &out)
{
    out.clear();
    std::set_union(a.begin(), a.end(), b.begin(), b.end(), inserter(out, out.begin()));
}

void GraphColor::calDRGenKill(std::map<MachineBlock *, std::set<MachineOperand *>> &gen, std::map<MachineBlock *, std::set<MachineOperand *>> &kill)
{
    for (auto &block : func->getBlocks())
    {
        for (auto &inst : block->getInsts())
        {
            if (inst->getDef().size() > 0)
            {
                for (auto def : inst->getDef())
                {
                    // 学学新用法
                    auto argRegNode = isArgReg(def);
                    // 把已经分配的参数寄存器摘出来
                    if (argRegNode != -1)
                    {
                        var2Node[def] = argRegNode;
                        continue;
                    }
                    if (!def->isVReg() || var2Node.find(def) != var2Node.end())
                        continue;
                    nodes.emplace_back(def->isFReg(), def);
                    nodes.back().loopWeight = mlpa->getDepth(block);
                    var2Node[def] = nodes.size() - 1;
                    if (spilledRegs.count(def) > 0)
                        nodes[var2Node[def]].hasSpilled = true;
                    auto copyGen = gen[block];
                    for (auto op : copyGen)
                        if ((*op) == (*def))
                            gen[block].erase(op);
                    gen[block].insert(def);
                    kill[block].insert(def);
                }
            }
        }
    }
}

void GraphColor::calDRInOut(std::map<MachineBlock *, std::set<MachineOperand *>> &gen, std::map<MachineBlock *, std::set<MachineOperand *>> &kill, std::map<MachineBlock *, std::set<MachineOperand *>> &in, std::map<MachineBlock *, std::set<MachineOperand *>> &out)
{
    auto entry = func->getEntry();
    out[entry] = gen[entry];
    std::queue<MachineBlock *> workList;
    for (auto block : func->getBlocks())
        if (block != entry)
            workList.push(block);
    while (!workList.empty())
    {
        auto block = workList.front();
        workList.pop();
        std::set<MachineOperand *> In[2];

        if(block->getPreds().size()==0)
        {
            continue;
        }
        auto preds = block->getPreds();

        In[0] = out[preds[0]];

        int turn = 0;
        if (preds.size() > 0) {
          for (auto it = std::next(preds.begin()); it != preds.end(); ++it) {
            aggregate(out[*it], In[turn], In[turn ^ 1]);
            turn ^= 1;
          }
        }

        in[block] = In[turn];
        // 计算out
        std::set<MachineOperand *> Out;
        for (auto indef : in[block])
            for (auto def : kill[block])
                if (*indef == *def)
                    In[turn].erase(indef);
        std::set_union(gen[block].begin(), gen[block].end(), In[turn].begin(), In[turn].end(), inserter(Out, Out.begin()));
        if (out[block] != Out)
        {
            out[block] = Out;
            for (auto &succ : block->getSuccs())
                workList.push(succ);
        }
    }
}

int GraphColor::mergeTwoNodes(int no1, int no2)
{
    int dst = std::min(no1, no2);
    int src = std::max(no1, no2);
    bool spilled = (nodes[dst].hasSpilled || nodes[src].hasSpilled);
    assert(nodes[src].fpu == nodes[dst].fpu);
    if (dst == src)
        return dst;
    nodes[dst].hasSpilled = spilled;
    nodes[src].hasSpilled = spilled;
    nodes[dst].defs.insert(nodes[src].defs.begin(), nodes[src].defs.end());
    nodes[dst].uses.insert(nodes[src].uses.begin(), nodes[src].uses.end());
    nodes[dst].loopWeight = std::max(nodes[dst].loopWeight, nodes[src].loopWeight);
    for (auto &def : nodes[dst].defs)
        var2Node[def] = dst;
    for (auto &use : nodes[dst].uses)
        var2Node[use] = dst;
    return dst;
}

void GraphColor::genNodes()
{
    // 预留前4+16个参数寄存器
    for (int i = 0; i < rArgRegNum; i++) //用于为整数寄存器生成节点。
    {
        //使用emplace_back在nodes容器尾部添加一个新的节点，该节点表示一个整数寄存器，
        //false表示不是浮点寄存器，nullptr表示该节点当前未关联到具体的操作数。
        nodes.emplace_back(false, nullptr); 
        nodes[i].color = i; //设置新添加的节点的颜色为当前循环变量i的值，表示寄存器的编号。
    }
    for (int i = 0; i < sArgRegNum; i++)
    {
        nodes.emplace_back(true, nullptr);
        nodes[i + rArgRegNum].color = i;
    }
    nodes.emplace_back(false, nullptr);
    nodes.back().color = 12;
    nodes.emplace_back(false, nullptr);
    nodes.back().color = 14;

    std::map<MachineBlock *, std::set<MachineOperand *>> gen;
    std::map<MachineBlock *, std::set<MachineOperand *>> kill;
    std::map<MachineBlock *, std::set<MachineOperand *>> in;
    std::map<MachineBlock *, std::set<MachineOperand *>> out;
    calDRGenKill(gen, kill); //通过calDRGenKill计算数据流的gen和kill。
    calDRInOut(gen, kill, in, out); //通过calDRInOut计算数据流的in和out。
    // 这个主要就是一个运算数可能有多个def都可以到达他
    std::map<MachineOperand, std::set<int>> op2Def; //将操作数映射到定义（def）的节点集合
    for (auto &block : func->getBlocks()) //遍历函数的基本块
    {
        op2Def.clear();
        for (auto &def : in[block])//遍历当前基本块的in集合，获取其中的每个定义。
            op2Def[*def].insert(var2Node[def]);//将当前定义对应的节点加入op2Def映射中。
        for (auto &inst : block->getInsts())
        {
            //遍历当前基本块内的指令
            for (auto &use : inst->getUse())
            { 
                //遍历当前指令的使用（use）操作数
                //调用isArgReg函数判断使用的操作数是否是参数寄存器，返回其编号。
                auto argRegNode = isArgReg(use);
                // 把已经分配的参数寄存器摘出来
                if (argRegNode != -1) //如果是参数寄存器。
                {
                    var2Node[use] = argRegNode; // 将使用的操作数映射到相应的参数寄存器节点。
                    continue;
                }
                //如果使用的不是虚拟寄存器（即不是程序中的变量）。
                if (!use->isVReg()) 
                    continue;
                //这里如果小于等于零，表示当前use的定值不存在，不太可能
                assert(op2Def[*use].size() > 0);
                auto it = op2Def[*use].begin();
                auto overPos = op2Def[*use].end();
                int no1 = *it;
                it++;
                for (; it != overPos; it++) //遍历剩余的定义节点。
                {
                    int no2 = *it;
                    no1 = mergeTwoNodes(no1, no2); //调用mergeTwoNodes函数合并两个节点。
                }
                op2Def[*use].clear(); //清空当前使用操作数的定义节点集合。
                op2Def[*use].insert(no1); //将合并后的节点重新插入到定义节点
                nodes[no1].uses.insert(use); //将合并后的节点记录为使用当前操作数的节点，并将当前使用的操作数加入到该节点的使用集合中。
                //计算当前节点的循环权重，选择较大的值更新到节点的loopWeight属性中。
                nodes[no1].loopWeight = std::max(mlpa->getDepth(block), nodes[no1].loopWeight);
                var2Node[use] = no1; //将当前使用的操作数映射到合并后的节点。
            }
            if (inst->getDef().size() > 0) // 如果当前指令有定义（def）的操作数。
            {
                for (auto def : inst->getDef()) //历当前指令的定义操作数。
                {
                    if (def->isVReg()) //如果定义的操作数是虚拟寄存器。
                    {
                        op2Def.erase(*def); //移除之前op2Def映射中该操作数的定义节点集合。
                        op2Def[*def].insert(var2Node[def]); //将新的定义节点插入到op2Def映射中。
                    }
                }
            }
        }
    }
}

void GraphColor::calLVGenKill(std::map<MachineBlock *, std::set<int>> &gen, std::map<MachineBlock *, std::set<int>> &kill)
{
    for (auto &block : func->getBlocks())
    {
        for (auto instIt = block->rbegin(); instIt != block->rend(); instIt++)
        {
            auto inst = *instIt;
            if (isCall(inst))
            {
                // call指令相当于对所有的r和s参数寄存器都定值了
                for (int i = 0; i < rArgRegNum + sArgRegNum + 2; i++)
                {
                    gen[block].erase(i);
                    kill[block].insert(i);
                }
                // call指令会用到参数寄存器个数
                auto [rnum, snum] = findFuncUseArgs(inst->getUse()[0]);
                for (int i = 0; i < rnum; i++)
                    gen[block].insert(i);
                for (int i = 0; i < snum; i++)
                    gen[block].insert(i + rArgRegNum);
                continue;
            }
            if (inst->getDef().size() > 0)
            {
                for (auto def : inst->getDef())
                {
                    if (def->isVReg() || isArgReg(def) != -1)
                    {
                        gen[block].erase(var2Node[def]);
                        kill[block].insert(var2Node[def]);
                    }
                }
            }
            for (auto &use : inst->getUse())
            {
                if (!use->isVReg() && isArgReg(use) == -1)
                    continue;
                gen[block].insert(var2Node[use]);
            }
        }
    }
}

void GraphColor::calLVInOut(std::map<MachineBlock *, std::set<int>> &gen, std::map<MachineBlock *, std::set<int>> &kill, std::map<MachineBlock *, std::set<int>> &in, std::map<MachineBlock *, std::set<int>> &out)
{
    std::queue<MachineBlock *> workList;
    for (auto &block : func->getBlocks())
    {
        if (block->getSuccs().size() == 0)
        {
            in[block] = gen[block];
            continue;
        }
        workList.push(block);
    }
    while (!workList.empty())
    {
        auto block = workList.front();
        workList.pop();
        std::set<int> Out[2];
        Out[0] = in[block->getSuccs()[0]];
        auto it = block->getSuccs().begin() + 1;
        auto overPos = block->getSuccs().end();
        int turn = 0;
        for (; it != overPos; it++)
        {
            aggregate(in[*it], Out[turn], Out[turn ^ 1]);
            turn ^= 1;
        }
        out[block] = Out[turn];
        // 计算out
        std::set<int> In;
        std::set<int> midDif;
        std::set_difference(out[block].begin(), out[block].end(), kill[block].begin(), kill[block].end(), inserter(midDif, midDif.begin()));
        std::set_union(gen[block].begin(), gen[block].end(), midDif.begin(), midDif.end(), inserter(In, In.begin()));
        if (in[block] != In)
        {
            in[block] = In;
            for (auto &pred : block->getPreds())
                workList.push(pred);
        }
    }
}

void GraphColor::genInterfereGraph()
{
    std::map<MachineBlock *, std::set<int>> gen;
    std::map<MachineBlock *, std::set<int>> kill;
    std::map<MachineBlock *, std::set<int>> in;
    std::map<MachineBlock *, std::set<int>> out;
    calLVGenKill(gen, kill); //计算每个基本块内的gen和kill。
    calLVInOut(gen, kill, in, out); //计算每个基本块的数据流的in和out。

    //遍历基本块和指令，通过connectEdge和connectEdge2建立干涉图。

    //定义了一个 lambda 函数connectEdge，用于连接寄存器间的边。
    auto connectEdge = [&](MachineOperand *op, MachineBlock *block)
    {
        auto use1 = var2Node[op];
        graph[use1];
        for (auto &use2 : out[block])
        {
            if (nodes[use1].fpu == nodes[use2].fpu)
            {
                graph[use1].insert(use2);
                graph[use2].insert(use1);
            }
        }
    };

    //定义了另一个 lambda 函数connectEdge2，用于连接寄存器间的边，其中使用了寄存器的编号。
    auto connectEdge2 = [&](int nodeNo, MachineBlock *block)
    {
        auto use1 = nodeNo;
        graph[use1];
        for (auto &use2 : out[block])
        {
            if (nodes[use1].fpu == nodes[use2].fpu)
            {
                graph[use1].insert(use2);
                graph[use2].insert(use1);
            }
        }
    };

    for (auto &block : func->getBlocks())
    {//遍历函数中的基本块
        for (auto instIt = block->rbegin(); instIt != block->rend(); instIt++)
        {//倒序遍历基本块内的指令
            auto inst = *instIt;
            if (isCall(inst))
            {//判断当前指令是否是函数调用（call）。
                // call指令相当于对所有的r和s参数寄存器都定值了
                for (int i = 0; i < rArgRegNum + sArgRegNum + 2; i++)
                {
                    connectEdge2(i, block); //调用 connectEdge2 函数，将参数寄存器 i 与当前基本块建立连接。
                    out[block].erase(i); //从当前基本块的 out 集合中移除寄存器 i。
                }
                // call指令会用到参数寄存器个数,获取函数调用指令的参数寄存器使用情况。
                auto [rnum, snum] = findFuncUseArgs(inst->getUse()[0]);
                for (int i = 0; i < rnum; i++)
                {//对于函数调用使用的 r 寄存器，建立连接。
                    connectEdge2(i, block);
                    out[block].insert(i);
                }
                for (int i = 0; i < snum; i++)
                {//对于函数调用使用的 s 寄存器，建立连接。
                    connectEdge2(i + rArgRegNum, block);
                    out[block].insert(i + rArgRegNum);
                }
                continue;
            }
            if (inst->getDef().size() > 0)
            {//如果当前指令有定义操作数，遍历当前指令的所有定义操作数。
                for (auto def : inst->getDef())
                {
                    if (def->isVReg() || isArgReg(def) != -1)
                    {//如果定义操作数是虚拟寄存器或者是函数参数寄存器
                        connectEdge(def, block); //调用 connectEdge 函数，将定义操作数与当前基本块建立连接。
                        graph[var2Node[def]];
                    }
                }
                for (auto def : inst->getDef())
                    out[block].erase(var2Node[def]);//从当前基本块的 out 集合中移除所有定义操作数对应的节点。
            }
            for (auto &use : inst->getUse())
            {//遍历当前指令的所有使用操作数。
                if (!use->isVReg() && isArgReg(use) == -1)
                //如果使用操作数既不是虚拟寄存器，也不是函数参数寄存器，则继续下一轮循环。
                    continue;
                connectEdge(use, block); //调用 connectEdge 函数，将使用操作数与当前基本块建立连接。
                out[block].insert(var2Node[use]); //将使用操作数对应的节点加入当前基本块的 out 集合。
            }
        }
    }
    // 不能有自环，删除干涉图中的自环边。
    for (auto &node : graph)
        node.second.erase(node.first);
    // 要在这里做聚合操作，因为之后会把r和s删掉
    coalescing();
    // 把图里面的r和s寄存器删掉
    for (int i = 0; i < rArgRegNum + sArgRegNum + 2; i++)
        graph.erase(i);
}

void GraphColor::coalescing()
{
    for (auto &block : func->getBlocks())
    {
        for (auto &inst : block->getInsts())
        {
            if (inst->isMov() && inst->getUse()[0]->isVReg() && inst->getDef()[0]->isVReg())
            {
                auto k = inst->getDef()[0]->isFReg() ? allUsableSRegs.size() : allUsableRRegs.size();
                auto x = var2Node[inst->getDef()[0]];
                auto y = var2Node[inst->getUse()[0]];
                if (x == y || graph[x].count(y) > 0) // 如果是一个，跳过就行
                    continue;
                bool couldMerge = true;
                for (auto t : graph[y])
                {
                    if (graph[t].size() >= k && graph[x].count(t) <= 0)
                    {
                        couldMerge = false;
                        break;
                    }
                }
                if (couldMerge)
                {
                    auto newNode = mergeTwoNodes(x, y);
                    auto otherNode = (newNode == x) ? y : x;
                    for (auto t : graph[otherNode])
                    {
                        graph[newNode].insert(t);
                        graph[t].erase(otherNode);
                        graph[t].insert(newNode);
                    }
                    graph.erase(otherNode);
                }
            }
        }
    }
}

typedef std::pair<int, int> ele;

struct cmp
{
    bool operator()(const ele &a, const ele &b)
    {
        return a.second > b.second;
    }
};

void GraphColor::genColorSeq()
{
    auto tmpGraph = graph;//创建一个临时变量 tmpGraph，将 graph 复制给它，这样在整个过程中我们可以修改 tmpGraph 而不影响原始的图。

    //定义一个 lambda 函数 eraseNode，接受一个迭代器 it 作为参数，用于从图中删除一个节点。
    auto eraseNode = [&](std::map<int, std::unordered_set<int>>::iterator it)
    {
        int nodeNo = (*it).first;
        auto tos = tmpGraph[nodeNo];
        for (auto &to : tos)
        {
            if (to >= rArgRegNum + sArgRegNum + 2)
                tmpGraph[to].erase(nodeNo);
        }
        return tmpGraph.erase(it);
    };

    while (!tmpGraph.empty()) // 进入循环，直到临时图为空
    {
        bool blocked = true; // 表示是否还可能有节点可以染色
        auto maxEdgesIt = tmpGraph.begin(); //初始化迭代器 maxEdgesIt，指向图中的第一个节点。
        int minLoopWeight = 0x3f3f3f3f; //初始化最小循环权重为一个较大的值。
        int maxEdges = 0; //初始化最大边数为0
        for (auto it = tmpGraph.begin(); it != tmpGraph.end();)
        {//开始迭代图中的节点
            int nodeNo = (*it).first; //获取当前迭代器指向的节点编号。
            int edges = (*it).second.size(); //获取当前节点的邻接边数。
            int loopWeight = nodes[nodeNo].loopWeight; //获取当前节点的循环权重。
            //获取当前节点可以使用的最大颜色数，取决于节点是浮点寄存器还是整数寄存器。
            int maxColors = nodes[nodeNo].fpu ? allUsableSRegs.size() : allUsableRRegs.size();
            if (!nodes[nodeNo].hasSpilled)
            {//当前节点未溢出（spill），
            //则根据循环权重和邻接边数选择一个优先级较高的节点。
                if (loopWeight < minLoopWeight)
                {
                    maxEdges = edges;
                    maxEdgesIt = it;
                    minLoopWeight = loopWeight;
                }
                else if (loopWeight == minLoopWeight && edges > maxEdges)
                {
                    maxEdges = edges;
                    maxEdgesIt = it;
                    minLoopWeight = loopWeight;
                }
            }
            //如果当前节点的邻接边数小于它可以使用的最大颜色数，将当前节点入栈，
            //从图中删除，并将 blocked 置为 false。
            //否则，继续迭代下一个节点。
            if (edges < maxColors)
            {
                blocked = false;
                colorSeq.push(nodeNo);
                it = eraseNode(it);
            }
            else
                it++;
        }
        //如果没有阻塞（所有节点都可以染色），则继续下一轮迭代。
        if (!blocked)
            continue;
        colorSeq.push((*maxEdgesIt).first); //将具有最高优先级的节点入栈。
        eraseNode(maxEdgesIt); //从图中删除这个节点。
    }
    assert(colorSeq.size() == graph.size());
}

int GraphColor::findMinValidColor(int nodeNo)
{
    std::set<int> usedColor;
    for (auto to : graph[nodeNo])
        if (nodes[to].color != -1)
            usedColor.insert(nodes[to].color);
    auto validColor = (nodes[nodeNo].fpu ? allUsableSRegs.begin() : allUsableRRegs.begin());
    auto maxValidColor = (nodes[nodeNo].fpu ? allUsableSRegs.end() : allUsableRRegs.end());
    for (; validColor != maxValidColor && usedColor.find(*validColor) != usedColor.end(); validColor++)
        ;
    if (validColor == maxValidColor)
        return -1;
    return *validColor;
}

bool GraphColor::tryColor()
{
    bool success = true;

    while (!colorSeq.empty())
    {//进入一个循环，只要颜色序列非空就继续循环。
        int nodeNo = colorSeq.top(); //从颜色序列中取出栈顶的节点编号，即待染色的节点。
        colorSeq.pop(); //弹出颜色序列的栈顶元素，即移除已处理的节点。
        int validColor = findMinValidColor(nodeNo); //函数找到当前节点可用的最小颜色，将其存储在 validColor 变量中。
        if (validColor != -1)
        {//如果找到了可用的颜色（不为 -1 表示找到了可用的颜色）。
            nodes[nodeNo].color = validColor;//将找到的可用颜色赋给当前节点。
        }
        else
        { //如果找不到可用的颜色。
            success = false;
            nodes[nodeNo].spill = true; //将当前节点标记为需要溢出（spill）。
            spillNodes.push_back(nodeNo); //将当前节点的编号添加到 spillNodes 向量中，记录需要溢出的节点。
        }
    }
    return success;
}

bool GraphColor::tryColoring()
{
    genColorSeq(); //调用genColorSeq()生成图着色的顺序。
    return tryColor(); //调用tryColor()进行实际的图着色。
}

bool GraphColor::graphColorRegisterAlloc()
{
    nodes.clear();
    graph.clear();
    var2Node.clear();
    spillNodes.clear();
    std::stack<int>().swap(colorSeq);
    genNodes(); //调用genNodes()生成节点。
    genInterfereGraph(); //调用genInterfereGraph()生成干涉图。
    return tryColoring(); //调用tryColoring()进行图着色。
}

void GraphColor::modifyCode()
{
    //根据着色结果修改函数的机器码，替换虚拟寄存器为分配的物理寄存器。
    for (int i = rArgRegNum + sArgRegNum + 2; i < nodes.size(); i++)
    {
        auto &node = nodes[i];
        if (node.color == -1)
            continue;
        // 这里，参数寄存器不需要保存
        if ((node.fpu && node.color >= sArgRegNum && node.color != 12 && node.color != 14) || (!node.fpu && node.color >= rArgRegNum))
            func->addSavedRegs(node.color);
        for (auto def : node.defs)
        {
            def->setReg(node.color);
        }
        for (auto use : node.uses)
        {
            use->setReg(node.color);
        }
    }
}

void GraphColor::allocateRegisters()
{
    spilledRegs.clear(); //清空之前保存的溢出寄存器集合。
    int counter = 0;
    for (auto &f : unit->getFuncs())
    {
        mlpa->analyse(f); //调用mlpa的analyse方法，对当前函数f进行分析。
        func = f;
        bool success = false;
        while (!success) //进入循环，直到成功分配寄存器：
        {
            success = graphColorRegisterAlloc();//调用graphColorRegisterAlloc函数进行寄存器分配，将结果保存在success中。
            counter++;
            if (!success)
                genSpillCode(); //调用genSpillCode函数，生成用于处理溢出寄存器的代码。
            else
                modifyCode(); //调用modifyCode函数，对函数的机器码进行修改，将虚拟寄存器替换为分配的物理寄存器。
        }
    }
}

void GraphColor::genSpillCode()
{
    for (auto nodeNo : spillNodes)
    {
        auto node = nodes[nodeNo];
        if (!node.spill)
            continue;
        // 在栈中分配内存
        node.disp = func->AllocSpace(4) * -1;
        // 在use前插入ldr
        for (auto use : node.uses)
        {
            spilledRegs.insert(use);
            auto cur_bb = use->getParent()->getParent();
            MachineInstruction *cur_inst = nullptr;
            auto newUse = new MachineOperand(*use);
            spilledRegs.insert(newUse);
            if (node.fpu)
            {
                if (node.disp >= -1020)
                {
                    cur_inst = new LoadMInstruction(cur_bb,
                                                    LoadMInstruction::VLDR,
                                                    newUse,
                                                    new MachineOperand(MachineOperand::REG, 11),
                                                    new MachineOperand(MachineOperand::IMM, node.disp));
                    cur_bb->insertBefore(cur_inst, use->getParent());
                }
                else
                {
                    auto internal_reg = new MachineOperand(MachineOperand::VREG, SymbolTable::getLabel());
                    cur_inst = new LoadMInstruction(cur_bb,
                                                    LoadMInstruction::LDR,
                                                    internal_reg,
                                                    new MachineOperand(MachineOperand::IMM, node.disp));
                    cur_bb->insertBefore(cur_inst, use->getParent());
                    cur_inst = new BinaryMInstruction(cur_bb,
                                                      BinaryMInstruction::ADD,
                                                      new MachineOperand(*internal_reg),
                                                      new MachineOperand(*internal_reg),
                                                      new MachineOperand(MachineOperand::REG, 11));
                    cur_bb->insertBefore(cur_inst, use->getParent());
                    cur_inst = new LoadMInstruction(cur_bb,
                                                    LoadMInstruction::VLDR,
                                                    newUse,
                                                    new MachineOperand(*internal_reg));
                    cur_bb->insertBefore(cur_inst, use->getParent());
                }
            }
            else
            {
                // https://developer.arm.com/documentation/dui0473/m/arm-and-thumb-instructions/ldr--immediate-offset-?lang=en
                if (node.disp >= -4095)
                {
                    cur_inst = new LoadMInstruction(cur_bb,
                                                    LoadMInstruction::LDR,
                                                    newUse,
                                                    new MachineOperand(MachineOperand::REG, 11),
                                                    new MachineOperand(MachineOperand::IMM, node.disp));
                    cur_bb->insertBefore(cur_inst, use->getParent());
                }
                else
                {
                    auto internal_reg = new MachineOperand(MachineOperand::VREG, SymbolTable::getLabel());
                    cur_inst = new LoadMInstruction(cur_bb,
                                                    LoadMInstruction::LDR,
                                                    internal_reg,
                                                    new MachineOperand(MachineOperand::IMM, node.disp));
                    cur_bb->insertBefore(cur_inst, use->getParent());
                    cur_inst = new LoadMInstruction(cur_bb,
                                                    LoadMInstruction::LDR,
                                                    newUse,
                                                    new MachineOperand(MachineOperand::REG, 11),
                                                    new MachineOperand(*internal_reg));
                    cur_bb->insertBefore(cur_inst, use->getParent());
                }
            }
        }

        // 在def后插入str
        for (auto def : node.defs)
        {
            spilledRegs.insert(def);
            auto cur_bb = def->getParent()->getParent();
            MachineInstruction *cur_inst = nullptr;
            auto newDef = new MachineOperand(*def);
            spilledRegs.insert(newDef);
            if (node.fpu)
            {
                if (node.disp >= -1020)
                {
                    cur_inst = new StoreMInstruction(cur_bb,
                                                     StoreMInstruction::VSTR,
                                                     newDef,
                                                     new MachineOperand(MachineOperand::REG, 11),
                                                     new MachineOperand(MachineOperand::IMM, node.disp));
                    cur_bb->insertAfter(cur_inst, def->getParent());
                }
                else
                {
                    auto internal_reg = new MachineOperand(MachineOperand::VREG, SymbolTable::getLabel());
                    cur_inst = new LoadMInstruction(cur_bb,
                                                    LoadMInstruction::LDR,
                                                    internal_reg,
                                                    new MachineOperand(MachineOperand::IMM, node.disp));
                    cur_bb->insertAfter(cur_inst, def->getParent());
                    auto cur_inst1 = new BinaryMInstruction(cur_bb,
                                                            BinaryMInstruction::ADD,
                                                            new MachineOperand(*internal_reg),
                                                            new MachineOperand(*internal_reg),
                                                            new MachineOperand(MachineOperand::REG, 11));
                    cur_bb->insertAfter(cur_inst1, cur_inst);
                    auto cur_inst2 = new StoreMInstruction(cur_bb,
                                                           StoreMInstruction::VSTR,
                                                           newDef,
                                                           new MachineOperand(*internal_reg));

                    cur_bb->insertAfter(cur_inst2, cur_inst1);
                }
            }
            else
            {
                // https://developer.arm.com/documentation/dui0473/m/arm-and-thumb-instructions/ldr--immediate-offset-?lang=en
                if (node.disp >= -4095)
                {
                    cur_inst = new StoreMInstruction(cur_bb,
                                                     StoreMInstruction::STR,
                                                     newDef,
                                                     new MachineOperand(MachineOperand::REG, 11),
                                                     new MachineOperand(MachineOperand::IMM, node.disp));
                    cur_bb->insertAfter(cur_inst, def->getParent());
                }
                else
                {
                    auto internal_reg = new MachineOperand(MachineOperand::VREG, SymbolTable::getLabel());
                    cur_inst = new LoadMInstruction(cur_bb,
                                                    LoadMInstruction::LDR,
                                                    internal_reg,
                                                    new MachineOperand(MachineOperand::IMM, node.disp));
                    cur_bb->insertAfter(cur_inst, def->getParent());
                    auto cur_inst1 = new StoreMInstruction(cur_bb,
                                                           StoreMInstruction::STR,
                                                           newDef,
                                                           new MachineOperand(MachineOperand::REG, 11),
                                                           new MachineOperand(*internal_reg));
                    cur_bb->insertAfter(cur_inst1, cur_inst);
                }
            }
        }
    }
}

int GraphColor::isArgReg(MachineOperand *op)
{
    int res = -1;
    if (op->isReg())
    {
        if (!op->isFReg() && op->getReg() < rArgRegNum)
            res = op->getReg();
        if (op->isFReg() && op->getReg() < sArgRegNum)
            res = rArgRegNum + op->getReg();
    }
    return res;
}

std::pair<int, int> GraphColor::findFuncUseArgs(MachineOperand *funcOp)
{
    int rnum = 0, snum = 0;
    auto funcName = funcOp->getLabel();
    funcName = funcName.substr(1, funcName.size() - 1);
    if (funcName == "memset")
        return std::make_pair(3, 0);
    else if (funcName == "__aeabi_ldivmod")
        return std::make_pair(4, 0);
    auto funcSe = globals->lookup(funcName);
    auto funcType = static_cast<FunctionType *>(static_cast<IdentifierSymbolEntry *>(funcSe)->getType());
    auto &paramsType = funcType->getParamsType();
    for (auto &paramType : paramsType)
    {
        if (paramType->isFloat())
            snum++;
        else
            rnum++;
    }
    rnum = std::min(rnum, rArgRegNum);
    snum = std::min(snum, sArgRegNum);
    return std::pair<int, int>(rnum, snum);
}

bool GraphColor::isCall(MachineInstruction *inst)
{
    // 如果是bl指令或者尾调用的b指令，返回true
    if (inst->isCall() )
        return true;
    return false;
}
