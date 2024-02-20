#ifndef IRPEEPHOLE
#define IRPEEPHOLE

#include "Unit.h"

class IRPeepHole
{
private:
    Unit *unit;

public:
    IRPeepHole(Unit *unit) : unit(unit){};//构造函数
    //简单封装一下子
    void subpass(Function *);
    void pass(){
        for (auto func = unit->begin(); func != unit->end(); func++){
            subpass(*func);
        }
    }
    
    //这是除法转膜的时候去判断除数的
    static int yeahorno(int imm) {
        if (imm <= 0 || (imm & (imm - 1)) != 0) {
            return -1; // 不是 2 的幂次方或非正整数
        }
        // 计算 2 的幂次方的指数
        int exponent = 0;
        while (imm != 1) {
            imm >>= 1;
            exponent++;
        }
        return exponent;
    }   

    //目的是检查一个指令对象是否是一个二元操作且该操作的其中一个操作数是常量
    std::pair<Operand*, Operand*> isBinaryConst(Instruction* inst, int opCode) {
        bool is1 = (inst->isBinary() && inst->getOpCode() == opCode && inst->getUse()[1]->getSymbolEntry()->isConstant());
        bool is2 = (inst->isBinary() && inst->getOpCode() == opCode && inst->getUse()[0]->getSymbolEntry()->isConstant());
        Operand *useOp = nullptr;
        Operand *constOp = nullptr;
        if (is1 && !is2) {
            useOp = inst->getUse()[0];
            constOp = inst->getUse()[1];
        } else if (is2 && !is1) {
            useOp = inst->getUse()[1];
            constOp = inst->getUse()[0];
        }
        return std::make_pair(useOp, constOp);
    }

};

#endif