#ifndef __AST_H__
#define __AST_H__

#include <fstream>
#include <vector>
#include <iostream>
#include <string>
#include <math.h>
#include "Type.h"
#include "SymbolTable.h"
#include "Operand.h"
#include "Instruction.h"


class SymbolEntry;
class Unit;
class Function;
class BasicBlock;
class Instruction;
class IRBuilder;


// 结点大类，框架给出
class Node
{
private:
    static int counter;
    int seq;
protected:
    //改成基于指令的truelist和falselist
    std::vector<Instruction *> true_list;
    std::vector<Instruction *> false_list;
    static IRBuilder *builder;
    void backPatch(std::vector<Instruction *> &list, BasicBlock *target, bool branch);
    std::vector<Instruction *> merge(std::vector<Instruction *> &list1, std::vector<Instruction *> &list2);
    Operand* typeCast(Type* targetType, Operand* operand);
public:
    Node *next;
    Node();
    void setNext(Node *node);// 添加next结点
    Node *getNext();// 获取当前结点的next结点
    int getSeq() const {return seq;};
    static void setIRBuilder(IRBuilder*ib) {builder = ib;};
    virtual void output(int level) = 0;
    virtual void typeCheck() = 0;
    virtual void genCode() = 0;
    std::vector<Instruction *> &trueList() { return true_list; }
    std::vector<Instruction *> &falseList() { return false_list; }
};

//表达式节点类
class ExprNode : public Node
{
protected:
    SymbolEntry *symbolEntry;
    Operand *dst;   // The result of the subtree is stored into dst.
    Type *type;

    bool isCond;
    Operand *dupDst;

public:
    ExprNode(SymbolEntry *symbolEntry) : symbolEntry(symbolEntry), isCond(false) {type = symbolEntry->getType();};
    Operand* getOperand() {return dst;};
    SymbolEntry* getSymPtr() {return symbolEntry;};
    Type *getType() { return type; };
    virtual double getValue() {return -1;};

    void genDupOperand() { dupDst = new Operand(new TemporarySymbolEntry(dst->getType(), SymbolTable::getLabel())); };
    Operand *getDupOperand() { return dupDst; };
    bool isConde() const { return isCond; };
    void setIsCond(bool isCond) { this->isCond = isCond; };

};

class BinaryExpr : public ExprNode
{
private:
    int op;
    ExprNode *expr1, *expr2;
public:
    enum {ADD, SUB, MUL, DIV, MOD, AND, OR, LESS, GREATER, LESSEQL, GREATEREQL, EQL, NOTEQL};
    BinaryExpr(SymbolEntry *se, int op, ExprNode*expr1, ExprNode*expr2) : ExprNode(se), op(op), expr1(expr1), expr2(expr2){
        dst = new Operand(se); 
    };
    void output(int level);
    void typeCheck();
    void genCode();
    void setType(Type *type) { this->type = type; };
    double getValue();
};

class UnaryExpr : public ExprNode
{
private:
    int op;
    ExprNode *expr;
public:
    enum {PLUS,MINUS, NOT};
    UnaryExpr(SymbolEntry *se, int op, ExprNode*expr) : ExprNode(se), op(op), expr(expr){
        dst = new Operand(se); 
    };
    void output(int level);
    void typeCheck();
    void genCode();
    double getValue();
};

class Constant : public ExprNode
{
public:
    Constant(SymbolEntry *se) : ExprNode(se){dst = new Operand(se);};
    void output(int level);
    void typeCheck();
    void genCode();
    double getValue();
};

class Id : public ExprNode
{
    ExprNode *index;
    bool isPointer = false;
public:
    Id(SymbolEntry *se, ExprNode *index = nullptr) : ExprNode(se), index(index)
    {
        this->type = se->getType();
        if (se->getType()->isArray())
        {
            std::vector<int> indexs = ((ArrayType *)se->getType())->getIndexs();
            SymbolEntry *temp = nullptr;
            ExprNode *expr = index;
            while (expr)
            {
                expr = (ExprNode *)expr->getNext();
                indexs.erase(indexs.begin());
            }
            if (indexs.size() <= 0)
            { // 如果索引和数组定义时候的维度一致，是引用某个数组元素
                if (((ArrayType *)se->getType())->getBaseType()->isInt())
                    this->type = TypeSystem::intType;
                else if (((ArrayType *)se->getType())->getBaseType()->isFloat())
                    this->type = TypeSystem::floatType;
                temp = new TemporarySymbolEntry(this->type, SymbolTable::getLabel());
            }
            else
            { // 索引个数小于数组定义时候的维度，应该作为函数参数传递，传递的是一个指针
                isPointer = true;
                indexs.erase(indexs.begin());
                if (((ArrayType *)se->getType())->getBaseType()->isInt())
                    this->type = new PointerType(new ArrayType(indexs, TypeSystem::intType));
                else if (((ArrayType *)se->getType())->getBaseType()->isFloat())
                    this->type = new PointerType(new ArrayType(indexs, TypeSystem::floatType));
                temp = new TemporarySymbolEntry(this->type, SymbolTable::getLabel());
            }
            dst = new Operand(temp);
        }
        else if (se->getType()->isPTR())
        {
            ArrayType *arrType = (ArrayType *)((PointerType *)se->getType())->getValType();
            SymbolEntry *temp = nullptr;
            if (index == nullptr)
            {
                this->type = se->getType();
                temp = new TemporarySymbolEntry(se->getType(), SymbolTable::getLabel());
            }
            else
            {
                std::vector<int> indexs = arrType->getIndexs();
                indexs.push_back(0);
                ExprNode *expr = index;
                while (expr)
                {
                    expr = (ExprNode *)expr->getNext();
                    indexs.erase(indexs.begin());
                }
                if (indexs.size())
                {
                    // 应该是作为指针传给其他函数了
                    isPointer = true;
                    indexs.erase(indexs.end() - 1);
                    if (arrType->getBaseType()->isInt())
                        this->type = new PointerType(new ArrayType(indexs, TypeSystem::intType));
                    else if (arrType->getBaseType()->isFloat())
                        this->type = new PointerType(new ArrayType(indexs, TypeSystem::floatType));
                    temp = new TemporarySymbolEntry(this->type, SymbolTable::getLabel());
                }
                else
                {
                    if (arrType->getBaseType()->isInt())
                        this->type = TypeSystem::intType;
                    else if (arrType->getBaseType()->isFloat())
                        this->type = TypeSystem::floatType;
                    temp = new TemporarySymbolEntry(this->type, SymbolTable::getLabel());
                }
            }
            if(temp == nullptr) {
                printf("temp不应为null\n");
            }
            dst = new Operand(temp);
        }
        else
        {
            this->type = se->getType();
            SymbolEntry *temp = new TemporarySymbolEntry(this->type, SymbolTable::getLabel());
            dst = new Operand(temp);
        }
    };
    Type* getType() {
        return this->symbolEntry->getType();
    }
    void output(int level);
    void typeCheck();
    void genCode();
    double getValue();
    ExprNode *getIndex() { return index; };
};

//语句节点类
class StmtNode : public Node
{
public:
    //添加一个虚函数，从而在其继承类中实现，最后主要递归到return中去检查（类型检查基础要求5）
    virtual int checkRet(Type *retType) { return -1; };
    void typeCheck(){};
    void genCode(){};
};


class ExprStmt : public StmtNode
{
private:
    ExprNode *expr;
public:
    ExprStmt(ExprNode *expr) : expr(expr){};
    void output(int level);
    void typeCheck();
    void genCode() { expr->genCode(); };
    int checkRet(Type *retType) { return -1; }
};



// class InitVal : public StmtNode
// {
// private:
//     ExprNode *leaf;
//     std::vector<InitVal*> leaves;
// public:
//     InitVal(ExprNode* expr = nullptr): leaf(expr) {}
//     void addLeaf(InitVal* leaf) {
//         this->leaves.push_back(leaf);
//     }
//     std::vector<InitVal*> getLeaves(){ return leaves;};
//     ExprNode* getLeaf() {return leaf;};
//     void output(int level){};
//     void genCode(){};
//     void typeCheck(){};
//     int checkRet(Type *retType) { return -1; };
//     void handle(Operand* addr, int *index, int *dim, BasicBlock* bb, Id* id) {
//         //index初始的值应该是0，dim的初始值应该是-1
//         Type* type = id->getType();
//         std::vector<int> decl_dim = dynamic_cast<ArrayType*>(type)->fetch();
//         int max_index = 0;
//         for(int i=0; i<(int)decl_dim.size(); i++) {
//             if(i==0) {
//                 max_index = decl_dim[(int)decl_dim.size() - 1 - i];
//             }
//             else {
//                 max_index *= decl_dim[(int)decl_dim.size() - 1 - i];
//             }
//         }
//         if(leaf!=nullptr) {
//             std::vector<int> index_list;
//             for(int i=0; i<=(int)decl_dim.size(); i++) index_list.push_back(0);
//             int temp = 0;

//             for(;;) {
//                 temp++;
//                 if(temp > *index) break;
//                 index_list[(int)decl_dim.size()]++;
//                 for(int i=(int)decl_dim.size(); i>=0; i--) {
//                     if(index_list[i] == decl_dim[i-1]) {
//                         index_list[i] = 0;
//                         index_list[i-1] ++;
//                         continue;
//                     }
//                     break;
//                 }
//             }

//             leaf->genCode();
//             SymbolEntry* temp_se = new TemporarySymbolEntry(type, SymbolTable::getLabel());
//             Operand* temp_op = new Operand(temp_se);

//             Operand *leaf_op = leaf->getOperand();
//             new GetElePtrInstruction(temp_op, addr, index_list ,bb);
//             new StoreInstruction(temp_op, leaf_op, bb);
//             *index = *index + 1;
//         }
//         else {
//             *dim = *dim + 1;
//             for(int i=0; i<(int)leaves.size(); i++) {
//                 leaves[i]->handle(addr, index, dim, bb, id);
//             }
            
//             int t = 0;
//             for(int j=0; j<(int)decl_dim.size() - *dim; j++) {
//                 if(j==0) {
//                     t = decl_dim[(int)decl_dim.size()-1-j];
//                 }
//                 else {
//                     t *= decl_dim[(int)decl_dim.size()-1-j];
//                 }
//             }
//             if(t == 0) return;
//             int pre_index = *index;
//             if((int)leaves.size() == 0) {
//                 *index = *index + t;
//             }
//             else {
//                 *index = (ceil((double)*index / t) * t);
//             }
//             for(int i=pre_index; i<*index && i<max_index; i++) {
//                 std::vector<int> index_list;
//                 for(int i=0; i<=(int)decl_dim.size(); i++) index_list.push_back(0);
//                 int temp = 0;

//                 for(;;) {
//                     temp++;
//                     if(temp > i) break;
//                     index_list[(int)decl_dim.size()]++;
//                     for(int i=(int)decl_dim.size(); i>=0; i--) {
//                         if(index_list[i] == decl_dim[i-1]) {
//                             index_list[i] = 0;
//                             index_list[i-1] ++;
//                             continue;
//                         }
//                         break;
//                     }
//                 }

//                 SymbolEntry *se_const0 = new ConstantSymbolEntry(TypeSystem::intType, 0);
//                 ExprNode *temp_const0 = new Constant(se_const0);
//                 Operand *src0_const0 = temp_const0->getOperand();

//                 SymbolEntry* temp_se = new TemporarySymbolEntry(type, SymbolTable::getLabel());
//                 Operand* temp_op = new Operand(temp_se);

//                 new GetElePtrInstruction(temp_op, addr, index_list ,bb);
//                 new StoreInstruction(temp_op, src0_const0, bb);
//             }
//             *dim = *dim - 1;
//         }
//     };
// };

class DeclStmt : public StmtNode
{
private:
    Id *id;
    ExprNode *expr;//用于获取给标识符赋值的表达式，如果为空，代表仅声明没赋初值
    ExprNode ** exprArray; //存储数组中每个元素的初值对应的表达式
public:
    DeclStmt(Id *id, ExprNode *expr = nullptr) 
        : id(id), expr(expr){};
    void output(int level);
    Id *getId() { return id; }
    void typeCheck();
    void genCode();
    void setInitArray(ExprNode** exprArray);
    int checkRet(Type *retType) { return -1; }
};



class NullStmt : public StmtNode
{
public:
    NullStmt(){};
    void output(int level);
    void typeCheck();
    void genCode();    
    int checkRet(Type *retType) { return -1; }
};


class CompoundStmt : public StmtNode
{
private:
    StmtNode *stmt;
public:
    CompoundStmt(StmtNode *stmt) : stmt(stmt) {};
    void output(int level);
    void typeCheck();
    void genCode();
    int checkRet(Type *retType) { return stmt->checkRet(retType); }
};

class SeqNode : public StmtNode
{
private:
    StmtNode *stmt1, *stmt2;
public:
    SeqNode(StmtNode *stmt1, StmtNode *stmt2) : stmt1(stmt1), stmt2(stmt2){};
    void output(int level);
    void typeCheck();
    void genCode();
    int checkRet(Type *retType)
    {
        int result1 = -1;
        int result2 = -1;
        if (stmt1)
        {
            result1 = stmt1->checkRet(retType);
        }
        if (stmt2)
        {
            result2 = stmt2->checkRet(retType);
        }
        if(result1 == -1 && result2 == -1) {
            return -1;
        }
        else if(result1 == -1 && result2 == 0) {
            return 0;
        }
        else if(result1 == -1 && result2 == 1) {
            return 1;
        }
        else if(result1 == 0 && result2 == -1) {
            return 0;
        }
        else if(result1 == 0 && result2 == 0) {
            return 0;
        }
        else if(result1 == 0 && result2 == 1) {
            return 1;
        }
        else {
            return 1;
        }
    }
};

class IfStmt : public StmtNode
{
private:
    ExprNode *cond;
    StmtNode *thenStmt;
public:
    IfStmt(ExprNode *cond, StmtNode *thenStmt) : cond(cond), thenStmt(thenStmt){};
    void output(int level);
    void typeCheck();
    void genCode();
    int checkRet(Type *retType)
    {
        if (thenStmt)
        {
            return thenStmt->checkRet(retType);
        }
        return -1;
    }
};

class IfElseStmt : public StmtNode
{
private:
    ExprNode *cond;
    StmtNode *thenStmt;
    StmtNode *elseStmt;
public:
    IfElseStmt(ExprNode *cond, StmtNode *thenStmt, StmtNode *elseStmt) : cond(cond), thenStmt(thenStmt), elseStmt(elseStmt) {};
    void output(int level);
    void typeCheck();
    void genCode();
    int checkRet(Type *retType)
    {
        int result1 = -1;
        int result2 = -1;
        if (thenStmt)
        {
            result1 = thenStmt->checkRet(retType);
        }
        if (elseStmt)
        {
            result2 = elseStmt->checkRet(retType);
        }
        if(result1 == -1 && result2 == -1) {
            return -1;
        }
        else if(result1 == -1 && result2 == 0) {
            return 0;
        }
        else if(result1 == -1 && result2 == 1) {
            return 1;
        }
        else if(result1 == 0 && result2 == -1) {
            return 0;
        }
        else if(result1 == 0 && result2 == 0) {
            return 0;
        }
        else if(result1 == 0 && result2 == 1) {
            return 1;
        }
        else {
            return 1;
        }
    }
};

class WhileStmt : public StmtNode
{
private:
    ExprNode *cond;
    StmtNode *thenStmt;
    //添加两个成员
    BasicBlock *condb;
    BasicBlock *endb;

public:
    WhileStmt(ExprNode *cond, StmtNode *thenStmt) : cond(cond), thenStmt(thenStmt){};
    void output(int level);
    void typeCheck();
    void genCode();
    BasicBlock *getCondBlock() const { return condb; };
    BasicBlock *getEndBlock() const { return endb; };
    int checkRet(Type *retType)
    {
        if (thenStmt)
        {
            return thenStmt->checkRet(retType);
        }
        return -1;
    }
};

class BreakStmt : public StmtNode
{
private:
    StmtNode *whileStmt;

public:
    BreakStmt() { };
    void output(int level);
    void typeCheck();
    void genCode();
    int checkRet(Type *retType) { return -1; }
};

class ContinueStmt : public StmtNode
{
private:
    StmtNode *whileStmt;

public:
    ContinueStmt() { };
    void output(int level);
    void typeCheck();
    void genCode();
    int checkRet(Type *retType) { return -1; }
};

class ReturnStmt : public StmtNode
{
private:
    ExprNode *retValue;
    Type* retType;
public:
    ReturnStmt(ExprNode*retValue = nullptr) : retValue(retValue) {};
    void output(int level);
    void typeCheck();
    void genCode();
    int checkRet(Type *retType);
};

class AssignStmt : public StmtNode
{
private:
    ExprNode *lval;
    ExprNode *expr;
public:
    AssignStmt(ExprNode *lval, ExprNode *expr) : lval(lval), expr(expr) {};
    void output(int level);
    void typeCheck();
    void genCode();
    int checkRet(Type *retType) { return -1; }
};






/****************************函数声明begin*******************************************/
class FunctionDef : public StmtNode
{
private:
    SymbolEntry *se;
    DeclStmt *FuncDefParams;
    StmtNode *stmt;
public:
    FunctionDef(SymbolEntry *se, StmtNode *stmt, DeclStmt *FuncDefParams = nullptr) : se(se), FuncDefParams(FuncDefParams), stmt(stmt){};
    void output(int level);
    void typeCheck();
    void genCode();
    int checkRet(Type *retType) { return -1; }
};
/****************************函数声明end*******************************************/
/****************************函数调用begin*******************************************/

class FuncCall : public ExprNode
{
private:
    ExprNode* params;

public:
    FuncCall(SymbolEntry *tmp,SymbolEntry *se, ExprNode *params = nullptr);
    void output(int level);
    void typeCheck();
    void genCode();
};
/****************************函数调用end*******************************************/





//语法树类，框架给出
class Ast
{
private:
    Node* root;
public:
    Ast() {root = nullptr;}
    void setRoot(Node*n) {root = n;}
    void output();
    void typeCheck();
    void genCode(Unit *unit);
};

extern WhileStmt* curr_whileStmt;

#endif
