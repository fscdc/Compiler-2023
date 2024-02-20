#ifndef __TYPE_H__
#define __TYPE_H__
#include <vector>
#include <string>
#include <assert.h>
#include "SymbolTable.h"
class Type
{
private:
    int kind;
public:
    enum {INT, FLOAT,VOID,FUNC, PTR, ARRAY, STRING};
    long long size;
    Type(int kind) : kind(kind) {};
    virtual ~Type() {};
    virtual std::string toStr() = 0;
    virtual bool isConst() const { return false; }
    bool isInt() const { return kind == INT; };
    bool isBool() const { return kind == INT && getSize() == 1;};
    bool isFloat() const { return kind == FLOAT; };
    bool isVoid() const { return kind == VOID; };
    bool isFunc() const { return kind == FUNC; };
    bool isPTR() const { return kind == PTR; };
    bool isArray() const { return kind == ARRAY; };

    int getKind() { return kind; };
    int getSize() const
    {
        //assert(size > 0);
        return (size - 1) / 8 + 1;
    }; // 单位是字节数

};

class IntType : public Type
{
private:
    bool constant;
public:
    IntType(int size,bool constant = false) : Type(Type::INT) , constant(constant){ this->size = size; };
    bool isConstant() { return constant; }

    std::string toStr();
};

class FloatType : public Type
{
private:
    bool constant;
public:
    FloatType(int size,bool constant = false) : Type(Type::FLOAT) , constant(constant){ this->size = size; };
    bool isConstant() { return constant; }

    std::string toStr();
};

class VoidType : public Type
{
public:
    VoidType() : Type(Type::VOID){};
    std::string toStr();
};


class FunctionType : public Type
{
private:
    Type *returnType;
    std::vector<Type *> paramsType;
    std::vector<SymbolEntry*> paramss;//参数列表

public:
    FunctionType(Type* returnType, std::vector<Type*> paramsType , std::vector<SymbolEntry*> paramss = {}) : 
    Type(Type::FUNC), returnType(returnType), paramsType(paramsType),paramss(paramss){};
    Type *getRetType() { return returnType; };
    void setParamsType(std::vector<Type *> paramsType)
    {
        this->paramsType = paramsType;
    };
    std::vector<Type *> &getParamsType() { return paramsType; }
    std::string toStr();
    long long getSize() const { return returnType->getSize(); }
};
class ArrayType;
class PointerType;


class StringType : public Type
{
private:
    int length;

public:
    StringType(int length) : Type(Type::STRING), length(length){};
    int getLength() const { return length; };
    std::string toStr();
};



class TypeSystem
{
private:
    static IntType commonInt;
    static FloatType commonFloat;
    static IntType commonBool;
    static VoidType commonVoid;
    static IntType commonConstInt;//多加这两个const便于后续使用
    static FloatType commonConstFloat;
    static IntType byteInt;
    static PointerType byteIntPtr;

public:
    static Type *intType;
    static Type *floatType;
    static Type *boolType;
    static Type *voidType;
    static Type *constIntType;
    static Type *constFloatType;
    static Type *int8Type;
    static Type *int8PtrType;
    static Type* getMaxType(Type* type1, Type* type2);
    static bool needCast(Type* type1, Type* type2);
};

class ArrayType : public Type
{
protected:
    std::vector<int> indexs;
    Type *baseType;

public:
    ArrayType(std::vector<int> indexs, Type* baseType = TypeSystem::intType)
        : Type(Type::ARRAY), indexs(indexs), baseType(baseType) 
    {
        this->size = 1;
        for(auto index : indexs) {
            this->size *=index;
        }
        this->size *=32;
    }
    std::vector<int> getIndexs() { return indexs; };
    Type* getBaseType() {return baseType;};
    bool isConst() const {return baseType->isConst();};
    std::string toStr();
};
class PointerType : public Type
{
private:
    Type *valueType;
public:
    PointerType(Type* valueType) : Type(Type::PTR) {this->valueType = valueType;};
    Type *getValType() { return this->valueType; };
    Type* getBaseType() {
        if(valueType->isArray()) {
            return ((ArrayType*)valueType)->getBaseType();
        }
        else {
            return getValType();
        }
    };
    std::string toStr();
};

#endif
