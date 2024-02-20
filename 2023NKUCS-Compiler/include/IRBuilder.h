//中间代码构造辅助类
#ifndef __IRBUILDER_H__
#define __IRBUILDER_H__

class Unit;
class Function;
class BasicBlock;

class IRBuilder
{
private:
    Unit *unit;//代表整个编译单元
    BasicBlock *insertBB;//指令应该插入的当前基本块
    bool duplicate = false;


public:
    IRBuilder(Unit*unit) : unit(unit){};
    void setInsertBB(BasicBlock*bb){insertBB = bb;};
    Unit* getUnit(){return unit;};
    BasicBlock* getInsertBB(){return insertBB;};
    void setDuplicate(bool dup) { duplicate = dup; };
    bool isDuplicate() { return duplicate; };
};

#endif