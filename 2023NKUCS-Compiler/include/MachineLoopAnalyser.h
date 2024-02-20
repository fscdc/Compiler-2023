#ifndef _MACHINE_LOOP_ANALYSER_H_
#define _MACHINE_LOOP_ANALYSER_H_

#include "MachineCode.h"

class MachineLoopAnalyser
{
private:
    MachineUnit *unit;
    std::map<MachineBlock *, std::set<MachineBlock *>> doms;
    std::map<MachineBlock *, std::set<MachineBlock *>> backEdges; // first是支配节点
    std::vector<std::set<MachineBlock *>> loops;                  // 是支配节点
    std::map<MachineBlock *, int> depths;

    void computeDoms(MachineFunction *);
    void lookForBackEdge(MachineFunction *);
    void computeLoops(MachineFunction *);
    void computeDepth(MachineFunction *);
    
    void intersection(std::set<MachineBlock *> &a, std::set<MachineBlock *> &b, std::set<MachineBlock *> &out)
    {
        out.clear();
        std::set_intersection(a.begin(), a.end(), b.begin(), b.end(), inserter(out, out.begin()));
    }
public:
    MachineLoopAnalyser(MachineUnit *munit) : unit(munit){};//构造函数
    void analyse(MachineFunction *func)
    {
        doms.clear();
        backEdges.clear();
        loops.clear();
        depths.clear();
        computeDoms(func);
        lookForBackEdge(func);
        computeLoops(func);
        computeDepth(func);
    }
    int getDepth(MachineBlock *block)
    {
        return depths[block];
    }
};

#endif