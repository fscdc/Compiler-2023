#include "ParamHandler.h"
#include "Type.h"
void ParamHandler::process(Function *func)
{
    auto entry = func->getEntry();
    //创建两个零常量操作数，一个用于整数 (intZero)，另一个用于浮点数 (floatZero)
    auto intZero = new Operand(new ConstantSymbolEntry(TypeSystem::constIntType, 0));
    auto floatZero = new Operand(new ConstantSymbolEntry(TypeSystem::constFloatType, 0));
    
    //初始化用于追踪整数和浮点数参数数量的计数器
    int intParamNum = 0;
    int floatParamNum = 0;

    //前四个整数用r0-r3，前十六个浮点数用s0-s15
    for (auto param : func->getParams())
    {
        //如果参数是指针类型，或者整数参数超过4个，或者浮点数参数超过16个，则跳过当前参数
        if (param->getType()->isPTR() || (param->getType()->isInt() && intParamNum >= 4) || (param->getType()->isFloat() && floatParamNum >= 16))
            continue;
        //如果整数和浮点数参数的数量都已达到上限，则结束循环
        if (intParamNum >= 4 && floatParamNum >= 16)
            break;
        
        auto baseType = param->getType();
        auto dst = new Operand(new TemporarySymbolEntry(baseType, SymbolTable::getLabel()));
        //创建一个新的二元加法指令 newInst，通过加零指令实现2reg
        auto newInst = new BinaryInstruction(BinaryInstruction::ADD, dst, param, (baseType->isInt() ? intZero : floatZero));
        //将这个新指令插入到函数入口基本块的开始位置
        entry->insertFront(newInst, false);
        auto useInsts = param->getUse();
        //遍历所有使用该参数的指令 useInst，对于每个使用，除了新创建的加法指令之外，将对原参数的引用替换为新的临时操作数 dst
        for (auto useInst : useInsts)
        {
            if (useInst == newInst)
                continue;
            useInst->replaceUse(param, dst);
        }
        //更新参数计数器
        if (baseType->isInt())
            intParamNum++;
        else
            floatParamNum++;
    }
}

