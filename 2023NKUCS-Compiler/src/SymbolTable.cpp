#include "SymbolTable.h"
#include <iostream>
#include <sstream>
#include "Type.h"
#include <assert.h>
#include <iomanip>
#include <cstring>
#include "Operand.h"

SymbolEntry::SymbolEntry(Type *type, int kind) 
{
    this->type = type;
    this->kind = kind;
    this->next = nullptr;
}

//里面添加了对重载函数检查的逻辑，如果是参数完全一致的重名函数则会拒绝重载（类型检查进阶要求1）
void SymbolEntry::setNext(SymbolEntry *se)
{
    SymbolEntry *temp = this;
    FunctionType *seType = dynamic_cast<FunctionType*>(se->getType());
    // 遍历当前的符号表来查找重载冲突
    while (temp)
    {
        FunctionType *tempType = dynamic_cast<FunctionType*>(temp->getType());
        if (tempType)
        {
            // 检查参数数量是否相同
            if (seType->getParamsType().size() == tempType->getParamsType().size())
            {
                bool sameParams = true;
                // 检查每个参数的类型是否相同
                for (size_t i = 0; i < seType->getParamsType().size(); ++i)
                {
                    if (seType->getParamsType()[i] != tempType->getParamsType()[i])
                    {
                        sameParams = false;
                        break;
                    }
                }
                // 如果参数数量和类型都相同，则认为是重复定义
                if (sameParams)
                {
                    fprintf(stderr, "Function does not meet overload requirements: function \"%s\" is defined twice\n", se->toStr().c_str());
                    assert(!sameParams);
                }
            }
        }
        temp = temp->getNext();
    }
    // 将se添加到符号表的末尾
    temp = this;
    while (temp->getNext())
        temp = temp->getNext();
    temp->next = se;
}


ConstantSymbolEntry::ConstantSymbolEntry(Type *type, double value) : SymbolEntry(type, SymbolEntry::CONSTANT)
{
    this->value = value;
}

std::string ConstantSymbolEntry::toStr()
{
    //如果是浮点数，则用IEEE754格式
    if(type->isFloat()) {
        static_assert(sizeof(double) == 8, "double must be 8 bytes");
        std::stringstream ss;
        uint64_t temp;
        std::memcpy(&temp, &value, sizeof(double));  // 使用memcpy来避免违反严格别名规则
        ss << "0x" << std::hex << std::uppercase << std::setfill('0') << std::setw(16) << temp;
        return ss.str();
    }
    // 整数则可以直接转字符串
    else {
        std::ostringstream buffer;
        buffer << (int)value;
        return buffer.str();
    }
}

IdentifierSymbolEntry::IdentifierSymbolEntry(Type *type, std::string name, int scope, bool sysy, int argNum) : SymbolEntry(type, SymbolEntry::VARIABLE), name(name), sysy(sysy)
{
    this->scope = scope;
    this->arrayValue = nullptr;
    // 如果是param，留一个Operand作为参数
    if (scope == PARAM)
    {
        TemporarySymbolEntry *se = new TemporarySymbolEntry(type, SymbolTable::getLabel());
        se->setIsParam(true);
        se->setSelfArgNum(argNum);
        argAddr = new Operand(se);
    }
    addr = nullptr;
    initial = false;
}

std::string IdentifierSymbolEntry::toStr()
{
    return "@" + name;
}

bool IdentifierSymbolEntry::isAllZero()
{
    if (!isInitial() || !type->isArray())
        return false;
    int size = static_cast<ArrayType*>(type)->getSize() / TypeSystem::intType->getSize();
    for (int i = 0; i < size; i++)
        if (arrayValue[i] != 0)
            return false;
    return true;
};

TemporarySymbolEntry::TemporarySymbolEntry(Type *type, int label) : SymbolEntry(type, SymbolEntry::TEMPORARY)
{
    this->label = label;
}

std::string TemporarySymbolEntry::toStr()
{
    std::ostringstream buffer;
    buffer << "%t" << label;
    return buffer.str();
}

SymbolTable::SymbolTable()
{
    prev = nullptr;
    level = 0;
}
SymbolTable::SymbolTable(SymbolTable *prev)
{
    this->prev = prev;
    this->level = prev->level + 1;
}


/*
    Description: lookup the symbol entry of an identifier in the symbol table
    Parameters: 
        name: identifier name
    Return: pointer to the symbol entry of the identifier

    hint:
    1. The symbol table is a stack. The top of the stack contains symbol entries in the current scope.
    2. Search the entry in the current symbol table at first.
    3. If it's not in the current table, search it in previous ones(along the 'prev' link).
    4. If you find the entry, return it.
    5. If you can't find it in all symbol tables, return nullptr.
*/
SymbolEntry* SymbolTable::lookup(std::string name)
{
    SymbolTable *current = identifiers;
    while (current != nullptr)
        // symbolTable为map类型的成员变量
        if (current->symbolTable.find(name) != current->symbolTable.end())
            return current->symbolTable[name];
        else
            // 向下一个SymbolTable去找
            current = current->prev;
    return nullptr;
}

//重新实现lookup只在当前作用域下面查
SymbolEntry* SymbolTable::lookuponlyforcurrent(std::string name){
    SymbolTable *current = identifiers;
    while (current != nullptr)
        // symbolTable为map类型的成员变量
        if (current->symbolTable.find(name) != current->symbolTable.end())
            return current->symbolTable[name];
        else
            break;
    return nullptr;
}

// install the entry into current symbol table.
void SymbolTable::install(std::string name, SymbolEntry* entry)
{
    //检查是否有函数的重定义，如果有同名的函数，调用setNext函数尝试链入符号表项（里面会对重载函数进行检查）
    if (symbolTable.find(name) != symbolTable.end() && symbolTable[name]->getType()->isFunc())
    {
        symbolTable[name]->setNext(entry);
    }
    else
    {
        symbolTable[name] = entry;
    }
}

int SymbolTable::counter = 0;
static SymbolTable t;
SymbolTable *identifiers = &t;
SymbolTable *globals = &t;
// TableList tables;
