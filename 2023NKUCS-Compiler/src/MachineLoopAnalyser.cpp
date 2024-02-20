#include "MachineLoopAnalyser.h"
#include <queue>
#include <stack>
// 计算机器函数中每个块的支配者

void MachineLoopAnalyser::computeDoms(MachineFunction *func)
{
    std::set<MachineBlock *> U;
    // 将所有块添加到集合U中
    for (auto block : func->getBlocks())
        U.insert(block);

    // 初始化每个块的支配者集合为U（即所有块）
    for (auto block : func->getBlocks())
        doms[block] = U;

    // 设置入口块的支配者仅为其自身
    auto entry = func->getEntry();
    doms[entry].clear();
    doms[entry].insert(entry);

    // 初始化工作列表，除入口块外的所有块都需要处理
    std::queue<MachineBlock *> workList;
    for (auto block : func->getBlocks())
        if (block != entry)
            workList.push(block);

    // 处理工作列表中的块
    while (!workList.empty())
    {
        auto block = workList.front();
        workList.pop();

        std::set<MachineBlock *> dom[2]; // 用于计算支配者的临时集合

        // 初始化第一个前驱的支配者集合
        if (block->getPreds().size() > 0)
            dom[0] = doms[block->getPreds()[0]];
            
        // 计算当前块所有前驱的支配者的交集
        auto preds = block->getPreds();
        int turn = 0;
        if (preds.size() > 0) {
            for (auto it = std::next(preds.begin()); it != preds.end(); ++it) {
                intersection(doms[*it], dom[turn], dom[turn ^ 1]);
                turn ^= 1;
            }
        }

        // 将当前块添加到其支配者集合中
        dom[turn].insert(block);

        // 如果当前块的支配者集合发生了变化，更新集合，并将其所有后继块添加到工作列表中
        if (dom[turn] != doms[block])
        {
            doms[block] = dom[turn];
            for (auto succ : block->getSuccs())
                workList.push(succ);
        }
    }
}

// 分析并查找机器函数中的后向边
// 后向边在循环分析中非常重要，因为它们指示了循环的存在

void MachineLoopAnalyser::lookForBackEdge(MachineFunction *func)
{
    // 遍历函数中的所有块
    for (auto block : func->getBlocks())
    {
        // 对于每个块，遍历其所有后继块
        for (auto succ : block->getSuccs())
        {
            // 如果当前块（block）支配其后继块（succ），则存在一个后向边
            // 支配意味着所有到达后继块的路径都必须经过当前块
            if (doms[block].find(succ) != doms[block].end())
            {
                // 在后向边映射中记录这个发现
                // 将后继块作为键，将当前块添加到与这个键关联的集合中
                backEdges[succ].insert(block);
            }
        }
    }
}


// 计算机器函数中的循环
// 使用之前识别的后向边来标识循环

void MachineLoopAnalyser::computeLoops(MachineFunction *func)
{
    // 遍历所有后向边
    for (auto [d, ns] : backEdges)
    {
        // 对于每个后向边，创建一个新的循环
        loops.push_back({});
        auto &l = loops.back(); // 获取刚刚创建的循环

        // 遍历与当前后向边相关联的所有节点
        for (auto n : ns)
        {
            std::stack<MachineBlock *> st; // 使用栈来执行深度优先搜索
            std::set<MachineBlock *> loop({n, d}); // 初始化循环集合，包含后向边的节点

            // 开始深度优先搜索
            st.push(n);
            while (!st.empty())
            {
                auto m = st.top();
                st.pop();
                // 遍历当前节点的所有前驱节点
                for (auto pred : m->getPreds())
                {
                    // 如果前驱节点不在循环集合中，则将其添加到集合中，并将其放入栈中以进一步搜索
                    if (loop.find(pred) == loop.end())
                    {
                        loop.insert(pred);
                        st.push(pred);
                    }
                }
            }
            // 将找到的循环节点添加到当前循环中
            l.insert(loop.begin(), loop.end());
        }
    }
}


// 计算机器函数中每个循环的深度
// 循环的深度是指循环嵌套的层级

void MachineLoopAnalyser::computeDepth(MachineFunction *func)
{
    // 定义比较函数，用于根据循环大小进行排序
    auto cmp = [=](const std::set<MachineBlock *> &a, const std::set<MachineBlock *> &b)
    {
        return a.size() < b.size();
    };

    // 按循环大小对循环进行排序
    std::sort(loops.begin(), loops.end(), cmp);

    // 初始化循环深度数组，默认深度为0
    std::vector<int> loopDepth(loops.size(), 0);

    // 计算每个循环的深度
    for (auto i = 0; i < loops.size(); i++)
    {
        // 如果循环尚未分配深度，则分配深度1
        if (loopDepth[i] == 0)
            loopDepth[i] = 1;

        // 检查是否有其他循环嵌套在当前循环内部
        for (auto j = i + 1; j < loops.size(); j++)
        {
            // 如果循环j包含循环i，则增加循环j的深度
            if (std::includes(loops[j].begin(), loops[j].end(), loops[i].begin(), loops[i].end()))
                loopDepth[j] = std::max(loopDepth[i] + 1, loopDepth[j]);
        }
    }

    // 创建映射，用于记录每个块所在的循环深度
    std::map<MachineBlock *, std::set<int>> blk2depth;
    for (auto i = 0; i < loops.size(); i++)
        for (auto blk : loops[i])
            blk2depth[blk].insert(loopDepth[i]);

    // 记录每个块的最大循环深度
    for (auto &[blk, layers] : blk2depth)
        depths[blk] = layers.size();
}







