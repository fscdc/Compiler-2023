#ifndef __GRAPH_COLOR_H_
#define __GRAPH_COLOR_H_
#include "MachineLoopAnalyser.h"
#include "MachineCode.h"
#include <set>
#include <map>
#include <vector>
#include <stack>
#include <unordered_set>

class GraphColor
{
private:
    struct Node // 这个表示图的一个节点
    {
        int color;
        bool spill;
        bool hasSpilled; // 判断这个是否是当前轮次被溢出的
        int disp;        // displacement in stack
        int loopWeight;  // 优化溢出判断
        std::set<MachineOperand *> defs;
        std::set<MachineOperand *> uses;
        bool fpu; // 是不是浮点寄存器，可以把浮点数的图和普通寄存器的图分开
        Node()
        {
            color = -1;
            spill = false;
            hasSpilled = false;
            fpu = false;
            disp = 0;
            loopWeight = 0;
            defs.clear();
            uses.clear();
        }
        Node(bool freg, MachineOperand *def)
        {
            color = -1;
            spill = false;
            hasSpilled = false;
            fpu = false;
            disp = 0;
            loopWeight = 0;
            defs.clear();
            uses.clear();
            fpu = freg;
            defs.insert(def);
        }
    };
    MachineUnit *unit;
    MachineFunction *func;
    MachineLoopAnalyser *mlpa;
    std::vector<Node> nodes;                      // 存总的节点集合
    std::map<int, std::unordered_set<int>> graph; // 存图
    std::map<MachineOperand *, int> var2Node;     // 将虚拟寄存器映射到节点
    std::vector<int> spillNodes;                  // 加这个变量也是希望能够快一点，不过最后好像差不多
    std::stack<int> colorSeq;
    std::set<MachineOperand *> spilledRegs; // 如果所有寄存器都溢出一遍还不行，这个程序还有救吗
    const int rArgRegNum = 4;
    const int sArgRegNum = 16;
    const int rbase = 0;
    const int sbase = 0;
    const int rRegNum = 11;
    const int sRegNum = 32;
    std::set<int> allUsableRRegs;
    std::set<int> allUsableSRegs;
    bool isCall(MachineInstruction *);
    
    void aggregate(std::set<MachineOperand *> &, std::set<MachineOperand *> &, std::set<MachineOperand *> &);
    void aggregate(std::set<int> &, std::set<int> &, std::set<int> &);
    int isArgReg(MachineOperand *op);
    void calDRGenKill(std::map<MachineBlock *, std::set<MachineOperand *>> &, std::map<MachineBlock *, std::set<MachineOperand *>> &);
    void calDRInOut(std::map<MachineBlock *, std::set<MachineOperand *>> &, std::map<MachineBlock *, std::set<MachineOperand *>> &, std::map<MachineBlock *, std::set<MachineOperand *>> &, std::map<MachineBlock *, std::set<MachineOperand *>> &);
    int mergeTwoNodes(int, int);
    void genNodes();
    std::pair<int, int> findFuncUseArgs(MachineOperand *);
    void calLVGenKill(std::map<MachineBlock *, std::set<int>> &, std::map<MachineBlock *, std::set<int>> &);
    void calLVInOut(std::map<MachineBlock *, std::set<int>> &, std::map<MachineBlock *, std::set<int>> &, std::map<MachineBlock *, std::set<int>> &, std::map<MachineBlock *, std::set<int>> &);
    void genInterfereGraph();
    void coalescing();
    void genColorSeq();
    int findMinValidColor(int);
    bool tryColor();
    bool tryColoring();
    bool graphColorRegisterAlloc();
    void genSpillCode();
    void modifyCode();

public:
    GraphColor(MachineUnit *munit) : unit(munit)
    {
        for (int i = 0; i < 11; i++)
            allUsableRRegs.insert(i);
        allUsableRRegs.insert(12);
        allUsableRRegs.insert(14);
        for (int i = 0; i < 32; i++)
            allUsableSRegs.insert(i);
        mlpa = new MachineLoopAnalyser(munit);
    };
    ~GraphColor()
    {
        delete mlpa;
    }
    void allocateRegisters();
};

#endif