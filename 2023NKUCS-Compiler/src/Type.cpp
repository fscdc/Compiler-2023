#include "Type.h"
#include <sstream>
//TypeSystem 类的静态成员初始化，多加了两个const
IntType TypeSystem::commonInt = IntType(32);
FloatType TypeSystem::commonFloat = FloatType(32);
IntType TypeSystem::commonBool = IntType(1);
VoidType TypeSystem::commonVoid = VoidType();
IntType TypeSystem::commonConstInt = IntType(32, true);
FloatType TypeSystem::commonConstFloat = FloatType(32,true);
IntType TypeSystem::byteInt = IntType(8);
PointerType TypeSystem::byteIntPtr = PointerType(&byteInt);

Type* TypeSystem::intType = &commonInt;
Type *TypeSystem::floatType = &commonFloat;
Type* TypeSystem::boolType = &commonBool;
Type* TypeSystem::voidType = &commonVoid;
Type *TypeSystem::constIntType = &commonConstInt;
Type *TypeSystem::constFloatType = &commonConstFloat;
Type* TypeSystem::int8Type = &byteInt;
Type* TypeSystem::int8PtrType = &byteIntPtr;



bool contains_i32(const std::string& str) {
    return str.find("i32") != std::string::npos;
}

bool contains_float(const std::string& str) {
    return str.find("float") != std::string::npos;
}

//返回更高级的类型
Type* TypeSystem::getMaxType(Type* type1, Type* type2){

    if(contains_float(type1->toStr()) || contains_float(type2->toStr())) {
        return floatType;
    }
    if(contains_i32(type1->toStr()) || contains_i32(type2->toStr())){
        return intType;
    }
    else {
        return boolType;
    }
}


//如果相等就不用转了
bool TypeSystem::needCast(Type* src, Type* target) {
    if(src->toStr() == target->toStr()) {
        return false;
    }
    return true;
}


std::string IntType::toStr()
{
    std::ostringstream buffer;
    buffer << "i" << size;
    return buffer.str();
}

std::string FloatType::toStr()
{
    return "float";
}

std::string VoidType::toStr()
{
    return "void";
}

std::string ArrayType::toStr()
{
    std::ostringstream buffer;
    for (auto index : indexs)
    {
        buffer << "[" << index << " x ";
    }
    // 结束的时候，tmp应该是基本类型
    buffer << baseType->toStr();
    for (unsigned long int i = 0; i < indexs.size(); i++)
    {
        buffer << "]";
    }
    return buffer.str();
}

std::string FunctionType::toStr()
{
    std::ostringstream buffer;
    buffer << returnType->toStr() << "(";
    for (size_t i = 0; i != paramsType.size(); i++)
    {
        if (i)
            buffer << ",";
        buffer << paramsType[i]->toStr();
    }
    buffer << ")";
    return buffer.str();
}


std::string PointerType::toStr()
{
    std::ostringstream buffer;
    buffer << valueType->toStr() << "*";
    return buffer.str();
}

std::string StringType::toStr()
{
    std::ostringstream buffer;
    buffer << "const char[" << length << "]";
    return buffer.str();
}
