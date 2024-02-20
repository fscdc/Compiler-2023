#include "Unit.h"
#include "AsmBuilder.h"

void Unit::insertFunc(Function *f)
{
    func_list.push_back(f);
}

void Unit::removeFunc(Function *func)
{
    func_list.erase(std::find(func_list.begin(), func_list.end(), func));
}

void Unit::output() const
{
    for (auto &func : func_list)
        func->output();
}


Function *Unit::se2Func(SymbolEntry *se)
{
    auto it = se2func.find(se);
    if (it != se2func.end())
    {
        //如果找到了，直接返回对应的函数实体
        return se2func[se];
    }
    //如果没有找到，遍历所有函数实体
    for (auto func : func_list)
    {
        //检查每个函数实体的符号指针是否与参数相等
        if (func->getSymPtr() == se)
        {
            se2func[se] = func;
            return func;
        }
    }
    return nullptr;
}
void Unit::genMachineCode(MachineUnit* munit) 
{
    AsmBuilder* builder = new AsmBuilder();
    builder->setUnit(munit);
    munit->setGlobalVars(this->global_vars);
    for (auto &func : func_list)
        func->genMachineCode(builder);
}


Unit::~Unit()
{
    auto delete_list = func_list;
    for(auto &func:delete_list)
        delete func;
}
