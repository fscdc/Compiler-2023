#ifndef __DEAD_CODE_ELIMINATION_H__
#define __DEAD_CODE_ELIMINATION_H__

#include "Unit.h"
#include <vector>

class IRDeadCodeElim {
private:
    Unit* unit;
public:
    std::vector<Instruction*> worklist; //关键指令集
    //删除没有前驱的块
    void unreachbbelim(Function *func) {
        bool again = true;
        while (again) {
            again = false;
            std::vector<BasicBlock *> temp;
            for (auto block : func->getBlockList())
                if (block->getNumOfPred() == 0 && block != func->getEntry())
                    temp.push_back(block);
            if (temp.size())
                again = true;
            for (auto block : temp) {
                delete block;
            }
        }
    }
    void markforins(Function* function);
    bool removeins(Function* function);
    void initalize(Function *func);
    IRDeadCodeElim(Unit* unit) : unit(unit){};
    void pass() {
        for (auto &element : *unit) {
            Function *func = element;
            bool change;
            unreachbbelim(func);
            do {
                initalize(func);
                markforins(func);
                change = removeins(func);
                unreachbbelim(func);
            } while (change);
        }
    }
};

#endif