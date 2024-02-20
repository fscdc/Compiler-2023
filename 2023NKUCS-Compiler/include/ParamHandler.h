#ifndef __PARAMHANDLER_H__
#define __PARAMHANDLER_H__

#include "Unit.h"

//这里是用于参数处理的一个类，辅助实现mem2reg优化的
class ParamHandler
{
private:
    Unit *unit;
    void process(Function*);

public:
    ParamHandler(Unit *unit) : unit(unit){};
    void pass()
    {
        //遍历所有函数进行处理
        for (auto func = unit->begin(); func != unit->end(); func++)
        {
            process(*func);
        }
    }
};

#endif