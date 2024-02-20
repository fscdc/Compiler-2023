#include "IRPeepHole.h"
#include "Type.h"

const int maxint = 2147483647;
extern FILE *yyout;

void IRPeepHole::subpass(Function *func)
{
    //下面都是case+solvecase的形式，分别表示一种特定的优化条件和对应的优化处理
    //case1：识别和优化连续的加法操作，其中每个加法操作的一个操作数是常量
    //给出一个例子：
    //%t1 = add i32 %a, 5     ; 指令1: %a 加 5，结果存储在 %t1
    //%t2 = add i32 %t1, 3    ; 指令2: %t1 加 3，结果存储在 %t2
    //优化后：%t2 = add i32 %a, 8    ; 优化后的指令: %a 加 8，结果存储在 %t2
    auto case1 = [&](Instruction *inst)
    {
        auto nextInst = inst->getNext();
        bool cond1 = (inst->isBinary() && inst->getOpCode() == BinaryInstruction::ADD && inst->getUse()[1]->getSymbolEntry()->isConstant());
        bool cond2 = (nextInst->isBinary() && nextInst->getOpCode() == BinaryInstruction::ADD && nextInst->getUse()[1]->getSymbolEntry()->isConstant());
        bool cond3 = cond1 && cond2 && (inst->getDef() == nextInst->getUse()[0] );
        //注意一下，要保证cond4
        bool cond4 = cond3 && (inst->getDef()->usersNum() == 1);
        return cond4;
    };
    auto solveCase1 = [&](Instruction *inst)
    {
        //下面就是把两条指令合在一起并处理一下关系等
        auto nextInst = inst->getNext();
        inst->getUse()[0]->removeUse(inst);
        nextInst->replaceUse(nextInst->getUse()[0], inst->getUse()[0]);
        auto value1 = static_cast<ConstantSymbolEntry *>(inst->getUse()[1]->getSymbolEntry())->getValue();
        auto value2 = static_cast<ConstantSymbolEntry *>(nextInst->getUse()[1]->getSymbolEntry())->getValue();
        bool floatV = static_cast<ConstantSymbolEntry *>(nextInst->getUse()[1]->getSymbolEntry())->getType()->isFloat();
        nextInst->replaceUse(nextInst->getUse()[1], new Operand(new ConstantSymbolEntry(floatV ? TypeSystem::floatType : TypeSystem::intType, floatV ? (float)value1 + (float)value2 : value1 + value2)));
        auto bb = inst->getParent();
        bb->remove(inst);
        return nextInst->getPrev();
    };
    //case2：处理了连续的 ADD 指令
    //给出一个例子：
    //%t1 = add i32 %a, 1
    // %t2 = add i32 %t1, 1
    // %t3 = add i32 %t2, 1
    // %t4 = add i32 %t3, 1
    // %t5 = add i32 %t4, 1
    //优化后：%tmp = mul i32 1, 5
    //%t5 = add i32 %a, %tmp
    auto case2 = [&](Instruction *inst)
    {
        bool cond1 = inst->isBinary() && inst->getOpCode() == BinaryInstruction::ADD;
        if (!cond1)
            return 0;
        auto bb = inst->getParent();
        int addNum = 1;
        for (auto ins = inst->getNext(); ins != bb->end(); ins = ins->getNext())
        {
            auto prevIns = ins->getPrev();
            if (!(ins->isBinary() && ins->getOpCode() == BinaryInstruction::ADD))
                break;
            if (prevIns->getDef() == ins->getUse()[0] && prevIns->getUse()[1] == ins->getUse()[1] && prevIns->getDef()->usersNum() == 1)
                addNum++;
            else
                break;
        }
        return addNum;
    };
    auto solveCase2 = [&](Instruction *inst)
    {
        auto bb = inst->getParent();
        int addNum = 1;
        auto ins = inst->getNext();
        auto src1 = inst->getUse()[0];
        auto src2 = inst->getUse()[1];
        src1->removeUse(inst);
        for (; ins != bb->end(); ins = ins->getNext())
        {
            auto prevIns = ins->getPrev();
            if (!(ins->isBinary() && ins->getOpCode() == BinaryInstruction::ADD))
                break;
            if (prevIns->getDef() == ins->getUse()[0] && prevIns->getUse()[1] == ins->getUse()[1] && prevIns->getDef()->usersNum() == 1)
            {
                src2->removeUse(prevIns);
                bb->remove(prevIns);
                addNum++;
            }
            else
                break;
        }
        assert(addNum >= 5);
        ins = ins->getPrev();
        ins->replaceUse(ins->getUse()[0], src1);
        auto newOp = new Operand(new TemporarySymbolEntry(ins->getDef()->getType(), SymbolTable::getLabel()));
        auto newMul = new BinaryInstruction(BinaryInstruction::MUL, newOp, src2, new Operand(new ConstantSymbolEntry(TypeSystem::intType, addNum)));
        bb->insertBefore(newMul, ins);
        ins->replaceUse(ins->getUse()[1], newOp);
        return ins;
    };
    //case3：处理的是其中第一条 ADD 指令的第二个操作数是常数 0，且其结果仅在紧随其后的另一条 ADD 指令中使用
    //给出一个例子：
    //%t1 = add i32 %a, 0
    //%t2 = add i32 %t1, %b
    //优化后：%t2 = add i32 %a, %b
    auto case3 = [&](Instruction *inst)
    {
        auto nextInst = inst->getNext();
        bool cond1 = (inst->isBinary() && inst->getOpCode() == BinaryInstruction::ADD && inst->getUse()[1]->getSymbolEntry()->isConstant() && static_cast<ConstantSymbolEntry *>(inst->getUse()[1]->getSymbolEntry())->getValue() == 0);
        bool cond2 = (nextInst->isBinary() && nextInst->getOpCode() == BinaryInstruction::ADD);
        bool cond3 = cond1 && cond2 && (nextInst->getUse()[0] == inst->getDef());
        bool cond4 = cond3 && (inst->getDef()->usersNum() == 1);
        return cond4;
    };
    auto solveCase3 = [&](Instruction *inst)
    {
        auto nextInst = inst->getNext();
        inst->getUse()[0]->removeUse(inst);
        nextInst->replaceUse(nextInst->getUse()[0], inst->getUse()[0]);
        auto bb = inst->getParent();
        bb->remove(inst);
        return nextInst->getPrev();
    };
    //case4：处理的是一条 ADD 指令的一个操作数是常数 0，且该指令不在函数的入口基本块中
    //给出一个例子：
    // %t1 = add i32 %a, 0
    // %t2 = mul i32 %t1, %b
    //优化后：%t2 = mul i32 %a, %b
    auto case4 = [&](Instruction *inst)
    {
        bool cond1 = (inst->isBinary() && inst->getOpCode() == BinaryInstruction::ADD && inst->getUse()[1]->getSymbolEntry()->isConstant() && static_cast<ConstantSymbolEntry *>(inst->getUse()[1]->getSymbolEntry())->getValue() == 0);
        bool cond2 = (inst->isBinary() && inst->getOpCode() == BinaryInstruction::ADD && inst->getUse()[0]->getSymbolEntry()->isConstant() && static_cast<ConstantSymbolEntry *>(inst->getUse()[0]->getSymbolEntry())->getValue() == 0);
        bool cond3 = (inst->getParent() != inst->getParent()->getParent()->getEntry());
        return (cond1 || cond2) && cond3;
    };
    auto solveCase4 = [&](Instruction *inst)
    {
        auto prev = inst->getPrev();
        bool cond1 = (inst->isBinary() && inst->getOpCode() == BinaryInstruction::ADD && inst->getUse()[1]->getSymbolEntry()->isConstant() && static_cast<ConstantSymbolEntry *>(inst->getUse()[1]->getSymbolEntry())->getValue() == 0);
        std::vector<Instruction *> uses(inst->getDef()->getUse());
        auto replaceUse = cond1 ? inst->getUse()[0] : inst->getUse()[1];
        replaceUse->removeUse(inst);
        for (auto use : uses)
            use->replaceUse(inst->getDef(), replaceUse);
        inst->getParent()->remove(inst);
        return prev;
    };
    //case5：不想描述了，直接给例子：
    // %t1 = add i32 %a, 5
    // %t2 = add i32 %t1, 10
    // 优化后：%t2 = add i32 %a, 15
    auto case5 = [&](Instruction *inst)
    {
        auto [useOp, constOp] = isBinaryConst(inst, BinaryInstruction::ADD);
        if (useOp == nullptr || useOp->getDef() == nullptr)
            return false;
        if (useOp->getDef()->getUse().size() != 1)
            return false;
        auto [useOp2, constOp2] = isBinaryConst(useOp->getDef(), BinaryInstruction::ADD);
        if (useOp2 == nullptr)
            return false;
        if (useOp2->getDef()->getParent() == func->getEntry())
            return false;
        return true;
    };
    auto solveCase5 = [&](Instruction *inst)
    {
        auto [useOp, constOp] = isBinaryConst(inst, BinaryInstruction::ADD);
        auto [useOp2, constOp2] = isBinaryConst(useOp->getDef(), BinaryInstruction::ADD);
        useOp->removeUse(inst);
        inst->replaceUse(useOp, useOp2);
        auto value1 = static_cast<ConstantSymbolEntry *>(constOp->getSymbolEntry())->getValue();
        auto value2 = static_cast<ConstantSymbolEntry *>(constOp2->getSymbolEntry())->getValue();
        bool floatV = static_cast<ConstantSymbolEntry *>(useOp2->getSymbolEntry())->getType()->isFloat();
        inst->replaceUse(constOp, new Operand(new ConstantSymbolEntry(floatV ? TypeSystem::floatType : TypeSystem::intType, floatV ? (float)value1 + (float)value2 : value1 + value2)));
        auto bb = useOp->getDef()->getParent();
        bb->remove(useOp->getDef());
        return inst->getPrev();
    };
    //case6：处理先乘后除，懒得描述直接给例子：
    // %t1 = mul i32 %a, 10
    // %t2 = div i32 %t1, 2
    // 优化后：%t2 = mul i32 %a, 5
    auto case6 = [&](Instruction *inst)
    {
        auto [mulUseOp, mulConstOp] = isBinaryConst(inst, BinaryInstruction::MUL);
        if (mulUseOp == nullptr || mulConstOp == nullptr || inst->getDef() == nullptr || inst->getDef()->getUse().size() != 1)
            return false;
        auto [divUseOp, divConstOp] = isBinaryConst(inst->getDef()->getUse()[0], BinaryInstruction::DIV);
        if (divUseOp == nullptr || divConstOp == nullptr)
            return false;
        if (!mulConstOp->getType()->isInt())
            return false;
        if (int(static_cast<ConstantSymbolEntry *>(mulConstOp->getSymbolEntry())->getValue()) % int(static_cast<ConstantSymbolEntry *>(divConstOp->getSymbolEntry())->getValue()) == 0)
            return true;
        return false;
    };
    auto solveCase6 = [&](Instruction *inst)
    {
        auto prevInst = inst->getPrev();
        auto divInst = inst->getDef()->getUse()[0];
        auto mulBB = inst->getParent();
        auto divBB = divInst->getParent();
        auto [mulUseOp, mulConstOp] = isBinaryConst(inst, BinaryInstruction::MUL);
        auto [divUseOp, divConstOp] = isBinaryConst(divInst, BinaryInstruction::DIV);
        auto mulConstValue = int(static_cast<ConstantSymbolEntry *>(mulConstOp->getSymbolEntry())->getValue());
        auto divConstValue = int(static_cast<ConstantSymbolEntry *>(divConstOp->getSymbolEntry())->getValue());
        divUseOp->removeUse(divInst);
        divConstOp->removeUse(divInst);
        inst->setDef(divInst->getDef());
        inst->replaceUse(mulConstOp, new Operand(new ConstantSymbolEntry(mulConstOp->getType(), mulConstValue / divConstValue)));
        mulBB->remove(inst);
        divBB->insertBefore(inst, divInst);
        divBB->remove(divInst);
        return prevInst;
    };
    //case7：懒得写直接给例子：
    //原始代码
    // int x = a * 2;
    // int y = x + a;
    //经过优化后
    // int y = a * 3;  
    auto case7 = [&](Instruction *inst)
    {
        auto [mulUseOp, mulConstOp] = isBinaryConst(inst, BinaryInstruction::MUL);
        if (mulUseOp == nullptr || mulConstOp == nullptr || inst->getDef() == nullptr || inst->getDef()->getUse().size() != 1)
            return false;
        auto addInst = inst->getDef()->getUse()[0];
        if (addInst->isBinary() && addInst->getOpCode() == BinaryInstruction::ADD)
        {
            Operand *baseOp = nullptr;
            if (addInst->getUse()[0] == inst->getDef())
                baseOp = addInst->getUse()[1];
            else if (addInst->getUse()[1] == inst->getDef())
                baseOp = addInst->getUse()[0];
            if (baseOp == nullptr || baseOp != mulUseOp || !mulConstOp->getType()->isInt() || static_cast<ConstantSymbolEntry *>(mulConstOp->getSymbolEntry())->getValue() >= maxint)
                return false;
            else
                return true;
        }
        return false;
    };
    auto solveCase7 = [&](Instruction *inst)
    {
        auto prevInst = inst->getPrev();
        auto addInst = inst->getDef()->getUse()[0];
        auto mulBB = inst->getParent();
        auto addBB = addInst->getParent();
        auto [mulUseOp, mulConstOp] = isBinaryConst(inst, BinaryInstruction::MUL);
        Operand *baseOp = nullptr;
        Operand *passOp = inst->getDef();
        if (addInst->getUse()[0] == passOp)
            baseOp = addInst->getUse()[1];
        else
            baseOp = addInst->getUse()[0];
        auto mulConstValue = int(static_cast<ConstantSymbolEntry *>(mulConstOp->getSymbolEntry())->getValue());
        passOp->removeUse(addInst);
        baseOp->removeUse(addInst);
        inst->setDef(addInst->getDef());
        inst->replaceUse(mulConstOp, new Operand(new ConstantSymbolEntry(mulConstOp->getType(), mulConstValue + 1)));
        mulBB->remove(inst);
        addBB->insertBefore(inst, addInst);
        addBB->remove(addInst);
        return prevInst;
    };
    //case8：就是优化乘1这种，不举例子了
    auto case8 = [&](Instruction *inst)
    {
        auto [mulUseOp, mulConstOp] = isBinaryConst(inst, BinaryInstruction::MUL);
        if (mulUseOp == nullptr || mulConstOp == nullptr || !mulConstOp->getType()->isInt() || static_cast<ConstantSymbolEntry *>(mulConstOp->getSymbolEntry())->getValue() != 1)
            return false;
        return true;
    };
    auto solveCase8 = [&](Instruction *inst)
    {
        auto prev = inst->getPrev();
        std::vector<Instruction *> uses(inst->getDef()->getUse());
        auto [mulUseOp, mulConstOp] = isBinaryConst(inst, BinaryInstruction::MUL);
        mulConstOp->removeUse(inst);
        mulUseOp->removeUse(inst);
        for (auto use : uses)
            use->replaceUse(inst->getDef(), mulUseOp);
        inst->getParent()->remove(inst);
        return prev;
    };
    //case9：专注于优化形如 a = b + c; d = a - e; 的指令序列，其中 c 和 e 是常数
    //懒得给例子了，就是把c和e合一起
    auto case9 = [&](Instruction *inst)
    {
        auto [addUseOp, addConstOp] = isBinaryConst(inst, BinaryInstruction::ADD);
        if (addUseOp == nullptr || addConstOp == nullptr || inst->getDef() == nullptr || inst->getDef()->getUse().size() != 1)
            return false;
        auto [subUseOp, subConstOp] = isBinaryConst(inst->getDef()->getUse()[0], BinaryInstruction::SUB);
        if (subUseOp == nullptr || subConstOp == nullptr || subConstOp != inst->getDef()->getUse()[0]->getUse()[1])
            return false;
        if (!addConstOp->getType()->isInt())
            return false;
        return true;
    };
    auto solveCase9 = [&](Instruction *inst)
    {
        auto prevInst = inst->getPrev();
        auto subInst = inst->getDef()->getUse()[0];
        auto addBB = inst->getParent();
        auto subBB = subInst->getParent();
        auto [addUseOp, addConstOp] = isBinaryConst(inst, BinaryInstruction::ADD);
        auto [subUseOp, subConstOp] = isBinaryConst(subInst, BinaryInstruction::SUB);
        auto addConstValue = int(static_cast<ConstantSymbolEntry *>(addConstOp->getSymbolEntry())->getValue());
        auto subConstValue = int(static_cast<ConstantSymbolEntry *>(subConstOp->getSymbolEntry())->getValue());
        if (addConstValue >= subConstValue)
        {
            subUseOp->removeUse(subInst);
            subConstOp->removeUse(subInst);
            inst->setDef(subInst->getDef());
            inst->replaceUse(addConstOp, new Operand(new ConstantSymbolEntry(addConstOp->getType(), addConstValue - subConstValue)));
            addBB->remove(inst);
            subBB->insertBefore(inst, subInst);
            subBB->remove(subInst);
        }
        else
        {
            addUseOp->removeUse(inst);
            addConstOp->removeUse(inst);
            subInst->replaceUse(subUseOp, addUseOp);
            subInst->replaceUse(subConstOp, new Operand(new ConstantSymbolEntry(addConstOp->getType(), subConstValue - addConstValue)));
            addBB->remove(inst);
        }
        return prevInst;
    };
    //case10：优化加减0，不举例子了
    auto case10 = [&](Instruction *inst)
    {
        auto [addUseOp, addConstOp] = isBinaryConst(inst, BinaryInstruction::ADD);
        if (addUseOp == nullptr || addConstOp == nullptr || static_cast<ConstantSymbolEntry *>(addConstOp->getSymbolEntry())->getValue() != 0)
            return false;
        auto [subUseOp, subConstOp] = isBinaryConst(inst, BinaryInstruction::SUB);
        if (subUseOp == nullptr || subConstOp == nullptr || static_cast<ConstantSymbolEntry *>(subConstOp->getSymbolEntry())->getValue() != 0)
            return false;
        return true;
    };
    auto solveCase10 = [&](Instruction *inst)
    {
        auto bb = inst->getParent();
        auto prevInst = inst->getPrev();
        auto [addUseOp, addConstOp] = isBinaryConst(inst, BinaryInstruction::ADD);
        auto [subUseOp, subConstOp] = isBinaryConst(inst, BinaryInstruction::SUB);
        auto op = addUseOp == nullptr ? subUseOp : addUseOp;
        for (auto useInst : inst->getDef()->getUse())
        {
            useInst->replaceUse(inst->getDef(), op);
        }
        bb->strongRemove(inst);
        return prevInst;
    };
    //case11有点类似case5，但是优化的侧重点不同，我给出一个例子就好说了：
    // 指令序列
    // %t0 = add i32 %a, 10   // 第一个加法指令
    // %t1 = add i32 %t0, 5   // 第二个加法指令
    // %t2 = add i32 %t1, 3   // 第三个加法指令
    // case5优化后的指令序列
    // %t1 = add i32 %a, 15  // 合并了%t0和%t1
    // %t2 = add i32 %t1, 3  // 保持不变
    // case11优化后的指令序列
    // %t0 = add i32 %a, 10  // 保持不变
    // %t2 = add i32 %t0, 8  // 合并了%t1和%t2
    auto case11 = [&](Instruction *inst)
    {
        auto [addUseOp, addConstOp] = isBinaryConst(inst, BinaryInstruction::ADD);
        if (addUseOp == nullptr || addConstOp == nullptr || inst->getDef() == nullptr || inst->getDef()->getUse().size() != 1)
            return false;
        auto nextInst = inst->getDef()->getUse()[0];
        auto [addUseOp1, addConstOp1] = isBinaryConst(nextInst, BinaryInstruction::ADD);
        if (addUseOp1 == nullptr || addConstOp1 == nullptr)
            return false;
        return true;
    };
    auto solveCase11 = [&](Instruction *inst)
    {
        auto bb = inst->getParent();
        auto preInst = inst->getPrev();
        auto nextInst = inst->getDef()->getUse()[0];
        auto [addUseOp, addConstOp] = isBinaryConst(inst, BinaryInstruction::ADD);
        auto [addUseOp1, addConstOp1] = isBinaryConst(nextInst, BinaryInstruction::ADD);
        addUseOp->removeUse(inst);
        nextInst->replaceUse(addUseOp1, addUseOp);
        auto value1 = static_cast<ConstantSymbolEntry *>(addConstOp->getSymbolEntry())->getValue();
        auto value2 = static_cast<ConstantSymbolEntry *>(addConstOp1->getSymbolEntry())->getValue();
        bool floatV = static_cast<ConstantSymbolEntry *>(addConstOp1->getSymbolEntry())->getType()->isFloat();
        nextInst->replaceUse(addConstOp1, new Operand(new ConstantSymbolEntry(floatV ? TypeSystem::floatType : TypeSystem::intType, floatV ? (float)value1 + (float)value2 : value1 + value2)));
        bb->strongRemove(inst);
        return preInst;
    };
    //下面就是遍历尝试进行优化了，之所以叫窥孔优化，也就是说这个优化的关注范围很小
    //所以其实逻辑是清晰和简单的，我愿称为最简单的一个优化
    bool change = true;
    while (change)
    {
        change = false;
        for (auto bb = func->begin(); bb != func->end(); bb++)
        {
            for (auto inst = (*bb)->begin(); inst != (*bb)->end(); inst = inst->getNext())
            {
                if (case1(inst))
                {
                    inst = solveCase1(inst);
                    change = true;
                }
                if (case2(inst) >= 5)//注意这个是大于5个才优化
                {
                    inst = solveCase2(inst);
                    change = true;
                }
                if (case3(inst))
                {
                    inst = solveCase3(inst);
                    change = true;
                }
                if (case4(inst))
                {
                    inst = solveCase4(inst);
                    change = true;
                }
                if (case5(inst))
                {
                    inst = solveCase5(inst);
                    change = true;
                }
                if (case6(inst))
                {
                    inst = solveCase6(inst);
                    change = true;
                }
                if (case7(inst))
                {
                    inst = solveCase7(inst);
                    change = true;
                }
                if (case8(inst))
                {
                    inst = solveCase8(inst);
                    change = true;
                }
                if (case9(inst))
                {
                    inst = solveCase9(inst);
                    change = true;
                }
                if (case10(inst))
                {
                    inst = solveCase10(inst);
                    change = true;
                }
                if (case11(inst))
                {
                    inst = solveCase11(inst);
                    change = true;
                }
            }
        }
    }
}



