#ifndef __PHIELIM_H__
#define __PHIELIM_H__

#include "Unit.h"
#include <assert.h>
#include <stack>
#include <queue>
#include <map>
#include <set>

class PhiElimination
{
private:
    Unit *unit;

public:
    PhiElimination(Unit *unit) : unit(unit) {}
    void pass()
    {
        for (auto it = unit->begin(); it != unit->end(); it++)
        {
            (*it)->lowerphi();
        }
    }
};

#endif
