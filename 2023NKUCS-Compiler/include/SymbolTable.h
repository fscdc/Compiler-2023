#ifndef __SYMBOLTABLE_H__
#define __SYMBOLTABLE_H__

#include <string>
#include <map>
#include <iostream>
#include <vector>
#include <algorithm>
#include <assert.h>
//#include "Function.h"
//#include "Type.h"
class Type;
class Operand;
class Function;


class SymbolEntry
{
private:
    int kind;
    SymbolEntry *next;//链接重载的同名函数
    double value;
    bool is_array = false; //标识是否是数组
    std::vector<double> arrVals;

protected:
    enum {CONSTANT, VARIABLE, TEMPORARY};
    Type *type;

public:
    SymbolEntry(Type *type, int kind);
    virtual ~SymbolEntry() {};
    bool isConstant() const {return kind == CONSTANT;};
    bool isTemporary() const {return kind == TEMPORARY;};
    bool isVariable() const {return kind == VARIABLE;};
    Type* getType() {return type;};
    void setType(Type *type) {this->type = type;};
    virtual std::string toStr() = 0;
    void setNext(SymbolEntry *se);
    SymbolEntry *getNext() { return next; };
    // You can add any function you need here.
    double getValue() { return value; };
    void setValue(double val) { value = val; };
    std::vector<double> &getArrVals() { return arrVals; };
    void setArray() {this->is_array = true;};
    bool isArray() {return this->is_array;};
    // int getNonZeroCnt()
    // {
    //     int ans = 0;
    //     assert(type->isArray());
    //     for (auto &val : arrVals)
    //         if (val)
    //             ans++;
    //     return ans;
    // };
};


/*  
    Symbol entry for literal constant. Example:

    int a = 1;

    Compiler should create constant symbol entry for literal constant '1'.
*/
class ConstantSymbolEntry : public SymbolEntry
{
private:
    double value;

public:
    ConstantSymbolEntry(Type *type, double value);
    virtual ~ConstantSymbolEntry() {};
    double getValue() const {return value;};
    std::string toStr();
    // You can add any function you need here.
};


/* 
    Symbol entry for identifier. Example:

    int a;
    int b;
    void f(int c)
    {
        int d;
        {
            int e;
        }
    }

    Compiler should create identifier symbol entries for variables a, b, c, d and e:

    | variable | scope    |
    | a        | GLOBAL   |
    | b        | GLOBAL   |
    | c        | PARAM    |
    | d        | LOCAL    |
    | e        | LOCAL +1 |
*/
class IdentifierSymbolEntry : public SymbolEntry
{
private:
    enum {GLOBAL, PARAM, LOCAL};    
    std::string name;
    int scope;
    Operand *addr;  //标识符的地址
    // You can add any field you need here.

    Operand *argAddr;
    double value;
    int label;
    bool initial;
    bool sysy;
    double *arrayValue;
    bool constant = false;
    Function *func = nullptr;
    
public:
    IdentifierSymbolEntry(Type *type, std::string name, int scope, bool sysy = false, int argNum = 0);
    virtual ~IdentifierSymbolEntry() {};
    std::string toStr();
    bool isGlobal() const {return scope == GLOBAL;};
    bool isParam() const {return scope == PARAM;};
    bool isLocal() const {return scope >= LOCAL;};
    int getScope() const {return scope;};
    void setAddr(Operand *addr) {this->addr = addr;};
    Operand* getAddr() {return addr;};
    Operand *getArgAddr() { return argAddr; }
    // You can add any function you need here.
    std::string getName() { return name; };
    void setConstant() { constant = true; };
    void setArrayValue(double *arrayValue) { this->arrayValue = arrayValue; };
    bool isSysy() const { return sysy; }
    Function *getFunction() { return func; };
    void setFunction(Function *func) { this->func = func; }

    double *getArrayValue() { return this->arrayValue; };
    bool isInitial() { return (initial || arrayValue != nullptr); };
    bool isAllZero();
};


/* 
    Symbol entry for temporary variable created by compiler. Example:

    int a;
    a = 1 + 2 + 3;

    The compiler would generate intermediate code like:

    t1 = 1 + 2
    t2 = t1 + 3
    a = t2

    So compiler should create temporary symbol entries for t1 and t2:

    | temporary variable | label |
    | t1                 | 1     |
    | t2                 | 2     |
*/
class TemporarySymbolEntry : public SymbolEntry
{
private:
    int stack_offset = 0;
    int label;
    int selfArgNum;
    bool isPara = false;
public:
    TemporarySymbolEntry(Type *type, int label);
    virtual ~TemporarySymbolEntry() {};
    std::string toStr();
    int getLabel() const {return label;};
    // You can add any function you need here.
    void setIsParam(bool pa) { this->isPara = pa; };
    bool isParam() { return this->isPara; };
    void setSelfArgNum(int selfArgNum) { this->selfArgNum = selfArgNum; };
    int getArgNum() { return this->selfArgNum; };
    void setOffset(int offset) { this->stack_offset = offset; };
    int getOffset() { return this->stack_offset; };
};

// symbol table managing identifier symbol entries
class SymbolTable
{
private:
    std::map<std::string, SymbolEntry*> symbolTable;
    SymbolTable *prev;
    int level;
    static int counter;
public:
    SymbolTable();
    SymbolTable(SymbolTable *prev);
    void install(std::string name, SymbolEntry* entry);
    SymbolEntry* lookup(std::string name);
    SymbolEntry* lookuponlyforcurrent(std::string name);//重新实现lookup只在当前作用域下面查
    SymbolTable* getPrev() {return prev;};
    int getLevel() {return level;};
    static int getLabel() {return counter++;};
        void print() {
        printf("start to print this table:size:%d\n",(int)symbolTable.size());
        for (const auto& entry : symbolTable) {
            std::cout << entry.first << " => " << entry.second << std::endl;
        }
        printf("print end\n\n");
    }
    int getSize() {
        return (int)symbolTable.size();
    }
    std::map<std::string, SymbolEntry*>* getTable() {
        return &symbolTable;
    }
};




extern SymbolTable *identifiers;
extern SymbolTable *globals;
// extern TableList tables;
#endif
