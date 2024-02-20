#ifndef __IRCOMSUBEXPRELIM_H__
#define __IRCOMSUBEXPRELIM_H__

#include "Unit.h"
#include <unordered_map>
#include "Type.h"

struct Expr
{
    Instruction *inst;
    Expr(Instruction *inst) : inst(inst){};
    // 用于调用find函数
    // TODO: 判断两个表达式是否相同
    // 两个表达式相同 <==> 两个表达式对应的指令的类型和操作数均相同
    bool operator==(const Expr &other) const//这是一个重载的等号运算符
    {
        //首先检查两个指令的类型和操作码是否相同
        if (inst->getType() != other.inst->getType() || inst->getOpCode() != other.inst->getOpCode())
            return false;
            
        auto ops1 = inst->getUse();
        auto ops2 = other.inst->getUse();
        //判断一下操作数大小是否相等
        if (ops1.size() != ops2.size())
            return false;
        auto op2 = ops2.begin();
        //遍历ops1集合中的每一个元素
        for (auto op1 : ops1)
        {
            //获取指向的符号条目
            auto se1 = op1->getSymbolEntry();
            auto se2 = (*op2)->getSymbolEntry();
            //检查两个符号条目是否都是常量
            if (se1->isConstant() && se2->isConstant())
            {
                //分别判断符号条目的值是否相等
                if (se1->getType()->isFloat() && se2->getType()->isFloat())
                {
                    if (static_cast<float>(static_cast<ConstantSymbolEntry *>(se1)->getValue()) != static_cast<float>(static_cast<ConstantSymbolEntry *>(se2)->getValue()))
                        return false;
                }
                else if (se1->getType()->isInt() && se2->getType()->isInt())
                {
                    if (static_cast<int>(static_cast<ConstantSymbolEntry *>(se1)->getValue()) != static_cast<int>(static_cast<ConstantSymbolEntry *>(se2)->getValue()))
                        return false;
                }
                else
                    return false;
            }
            //检查op1和op2指向的元素是否不相等
            else if (op1 != (*op2))
                return false;
            op2++;//op2迭代
        }
        return true;
    };
};

class IRComSubExprElim
{
private:
    Unit *unit;

    std::vector<Expr> exprVec;
    std::map<Instruction *, int> ins2Expr;
    std::map<BasicBlock *, std::set<int>> genBB;
    std::map<BasicBlock *, std::set<int>> killBB;
    std::map<BasicBlock *, std::set<int>> inBB;
    std::map<BasicBlock *, std::set<int>> outBB;

    //记录添加的load和store指令对
    std::vector<std::pair<Instruction *, Instruction *>> addload;
    //用来记录一个基本块gen的表达式的结果寄存器
    std::map<BasicBlock *, std::map<int, Operand *>> expr2Op;
    //用于追踪每个基本块结束时活跃的、可以被后续基本块使用的表达式和操作数
    std::map<BasicBlock *, std::set<std::pair<int, Operand *>>> outBBOp;

    // 跳过无需分析的指令
    bool skip(Instruction *inst)
    {
        /**
         * 判断当前指令是否可以当成一个表达式
         * 实现了将二元运算指令及一些一元指令和隐式转换指令也可当作表达式
         */
        //目前没有加上纯函数
        if (dynamic_cast<BinaryInstruction *>(inst) != nullptr || inst->isLoad() || inst->isGEP() || inst->isZext() || inst->isNot() || inst->isIntFloatCast())
            return false;
        return true;
    }
    // 局部公共子表达式消除
    bool localCSE(Function *);
    // 全局公共子表达式消除
    bool globalCSE(Function *);

    void calGenKill(Function*);//数据流分析，计算基本块的gen集合和set集合
    void calInOut(Function*);//数据流分析，计算基本块的in集合和out集合
    bool removeGlobalCSE(Function*);

public:
    IRComSubExprElim(Unit *unit){ this->unit = unit;};
    void pass()
    {
        addload.clear();
        exprVec.clear();
        genBB.clear();
        killBB.clear();
        inBB.clear();
        outBB.clear();
        ins2Expr.clear();
        expr2Op.clear();
        outBBOp.clear();
        for (auto func = unit->begin(); func != unit->end(); func++)
        {
            while (!localCSE(*func) || !globalCSE(*func))
                ;
        }
    }

};

#endif