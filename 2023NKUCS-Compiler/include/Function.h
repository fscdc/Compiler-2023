//函数
#ifndef __FUNCTION_H__
#define __FUNCTION_H__

#include <vector>
#include <map>
#include <set>
#include <list>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <iostream>
#include "BasicBlock.h"
#include "SymbolTable.h"



class Unit;

//可以用于实现多种树型数据结构：DFS树和DOM树
//DFS树用于遍历和搜索算法
//DOM树则用于表示基本块间的支配关系
struct TreeNode
{
    static int Num;
    //num只是用于DFS树构建的
    int num;

    BasicBlock *block;
    std::vector<TreeNode *> children;
    TreeNode *parent = nullptr;
    //用于DFS树的构造函数
    TreeNode(BasicBlock *block) : block(block)
    {
        num = Num++;
        block->order = num;
    }
    //用于DOM树的构造函数
    TreeNode(BasicBlock *block, int num) : block(block)
    {
        this->num = block->order;
    }
    void addChild(TreeNode *child) 
    { 
        children.push_back(child); 
    }
    //getHeight仅用于DOM树节点
    int getHeight()
    {
        int height = 0;
        TreeNode *temp = this;
        while (temp)
        {
            height++;
            temp = temp->parent;
        }
        return height;
    }

};


class Function
{
    typedef std::vector<BasicBlock *>::iterator iterator;
    typedef std::vector<BasicBlock *>::reverse_iterator reverse_iterator;

private:
    std::vector<BasicBlock *> block_list;
    SymbolEntry *sym_ptr;
    BasicBlock *entry;
    Unit *parent;

    std::vector<Operand *> paramsOperand; // 函数中的参数所对应的操作数
    std::vector<Instruction *> callPreds; // 调用了当前函数的那些call指令
    int critical = -1;                    // 表征函数是否关键,-1表示没有被标识
public:
    Function(Unit *, SymbolEntry *);
    ~Function();
    void insertBlock(BasicBlock *bb) { block_list.push_back(bb); };
    BasicBlock *getEntry() { return entry; };
    void remove(BasicBlock *bb);
    void output() const;
    std::vector<BasicBlock *> &getBlockList(){return block_list;};
    iterator begin() { return block_list.begin(); };
    iterator end() { return block_list.end(); };
    reverse_iterator rbegin() { return block_list.rbegin(); };
    reverse_iterator rend() { return block_list.rend(); };
    SymbolEntry *getSymPtr() { return sym_ptr; };
    void pushParamsOperand(Operand *operand) { paramsOperand.push_back(operand); };
    std::vector<Operand *> &getParams() { return paramsOperand; }

    std::vector<Instruction *> &getCallPred() { return callPreds; };
    int getCritical();
    void computeRDF();
    void computeRDFSTree(BasicBlock *exit);
    void computeRSdom(BasicBlock *exit);
    void computeRIdom(BasicBlock *exit);
    void reversebuild(TreeNode *node, bool *visited);
    BasicBlock *getMarkBranch(BasicBlock *block);

    void removeNullPointers(std::vector<TreeNode*>& vec) {
        auto newEnd = std::remove(vec.begin(), vec.end(), nullptr);
        vec.erase(newEnd, vec.end());
    }

    void addCallPred(Instruction *in);
    void genMachineCode(AsmBuilder*);

public:
    //两个树的根节点
    TreeNode *DFSTreeRoot;
    TreeNode *DOMTreeRoot;
    //将每个基本块的预排序编号映射到DFS树中对应的节点
    std::vector<TreeNode *> preOrder2DFS;
    //将每个基本块的预排序编号映射到支配树中对应的节点
    std::vector<TreeNode *> preOrder2dom;
    //存储每个基本块的半支配者的预排序编号
    std::vector<int> sdoms;
    //存储每个基本块的立即支配者的预排序编号
    std::vector<int> idoms;

    //获取基本块在block_list中的编号
    int getIndex(BasicBlock *block)
    {
        return std::find(block_list.begin(), block_list.end(), block) -
               block_list.begin();
    }
    //构建DFS
    void buildDFS(TreeNode *node, bool *visited)
    {
        int n = getIndex(node->block);
        visited[n] = true;

        auto block = block_list[n];
        //遍历 block 的所有后继基本块
        for (auto it = block->succ_begin(); it != block->succ_end(); it++)
        {
            int idx = getIndex(*it);
            //如果这个后继基本块未被访问，把其设置成当前节点的孩子节点
            if (!visited[idx])
            {
                TreeNode *child = new TreeNode(*it);
                preOrder2DFS[child->num] = child;
                child->parent = node;
                node->addChild(child);
                //递归调用继续构建
                fprintf(stderr, "Creating Child Node: %d for Parent Node: %d\n", idx, n);
                buildDFS(child, visited);
            }
        }
    }
    //查找给定基本块的最小半支配者
    int getlittle(int v, int *ancestors) {
        // 获取基本块 v 的直接祖先
        for (int a = ancestors[v]; a != -1; a = ancestors[a]) {
            // 如果祖先 a 的半支配值更小，则更新
            if (sdoms[v] > sdoms[a])
                v = a;
        }
        return v;
    }
    //用于找出两个基本块在支配树中的最近公共祖先。
    int LCA(int i, int j) {
        TreeNode *n1 = preOrder2dom[i];
        TreeNode *n2 = preOrder2dom[j];

        // 调整 n1 和 n2 至同一高度
        while (n1 && n2 && n1->getHeight() != n2->getHeight()) {
            if (n1->getHeight() > n2->getHeight())
                n1 = n1->parent;
            else
                n2 = n2->parent;
        }

        // 同时遍历 n1 和 n2 的祖先，直到找到它们的最近公共祖先
        while (n1 && n2) {
            if (n1 == n2)
                return n1->num;
            n1 = n1->parent;
            n2 = n2->parent;
        }
        return -1;
    }

    //查找一个基本块在DOM中对应的节点
    TreeNode *getDomNode(BasicBlock *b) { return preOrder2dom[b->order]; }


    void computeDFS();
    void computeSdom();
    void computeIdom();
    void computeDomFrontier();


    void lowerphi();
};

#endif
