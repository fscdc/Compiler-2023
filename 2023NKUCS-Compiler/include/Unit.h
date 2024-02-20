//编译单元
#ifndef __UNIT_H__
#define __UNIT_H__

#include <vector>
#include "Function.h"
#include "MachineCode.h"

class Unit
{
    typedef std::vector<Function *>::iterator iterator;
    typedef std::vector<Function *>::reverse_iterator reverse_iterator;

private:
    std::vector<Function *> func_list;

    std::map<SymbolEntry *, Function *> se2func;//用来记录每个函数符号表项对应的函数，便于找到

    std::vector<SymbolEntry *> global_vars;
public:
    Unit() = default;
    ~Unit() ;
    void insertFunc(Function *);
    void removeFunc(Function *);
    void output() const;
    iterator begin() { return func_list.begin(); };
    iterator end() { return func_list.end(); };
    reverse_iterator rbegin() { return func_list.rbegin(); };
    reverse_iterator rend() { return func_list.rend(); };
    

    Function *se2Func(SymbolEntry *se);

    void genMachineCode(MachineUnit *munit);

    void addGlobalVar(SymbolEntry *se) { global_vars.push_back(se); };

};

#endif