//指令
#ifndef __INSTRUCTION_H__
#define __INSTRUCTION_H__

#include "Operand.h"
#include <vector>
#include <map>
#include <unordered_map>
#include "SymbolTable.h"
#include "AsmBuilder.h"

class BasicBlock;

class Instruction
{
public:
    unsigned instType;
    unsigned opcode;
    Instruction *prev;
    Instruction *next;
    BasicBlock *parent;
    std::vector<Operand*> operands;
    enum {BINARY, COND, UNCOND, RET, LOAD, STORE, CMP, ALLOCA, IntFloatCast,CALL,TEMP,GLOBAL,NOT,ZEXT,PHI, GEP,BITCAST};

    bool mark;
    bool floatVersion;

public:
    Instruction(unsigned instType, BasicBlock *insert_bb = nullptr);
    virtual ~Instruction();
    BasicBlock *getParent();
    bool isLoad() const { return instType == LOAD; };
    bool isStore() const { return instType == STORE; };
    bool isUncond() const {return instType == UNCOND;};
    bool isCond() const {return instType == COND;};
    bool isAlloca() const { return instType == ALLOCA; };
    bool isRet() const { return instType == RET; };
    bool isCall() const { return instType == CALL; };
    bool isZext() const { return instType == ZEXT;};
    bool isIntFloatCast() const {return instType ==IntFloatCast; };
    bool isNot() const { return instType == NOT;};
    bool isPhi() const { return instType == PHI; };
    bool isGEP() const { return instType == GEP;};
    bool isBitcast() const { return instType == BITCAST;};
    bool isAdd() const { return instType == BINARY && opcode == 1; }
    bool isBinary() const {return instType == BINARY;};
    void setParent(BasicBlock *);
    void setNext(Instruction *);
    void setPrev(Instruction *);
    Instruction *getNext();
    Instruction *getPrev();
    virtual Operand *getDef() { return nullptr; }
    virtual std::vector<Operand *> getUse() { return {}; }
    virtual void output() const = 0;
    int getType() const { return instType; }
    int getOpCode() const { return opcode; }

    virtual void replaceUse(Operand *, Operand *) {}
    virtual void replaceDef(Operand *) {}
    std::vector<Operand *> &getOperands() { return operands; }
    virtual void setDef(Operand *) {}

    void unsetMark() { mark = false; };
    bool isCritical();
    void setMark() { mark = true; };
    bool getMark() { return mark; };

    virtual void genMachineCode(AsmBuilder*) = 0;
    MachineOperand *genMOperand(Operand *, AsmBuilder *);
    MachineOperand *fimmToVReg(MachineOperand *imm, MachineBlock *cur_block, AsmBuilder *builder);
    MachineOperand *genMachineVReg(bool fpu);
    MachineOperand *immToVReg(MachineOperand *imm, MachineBlock *cur_block);
    MachineOperand *genMachineImm(int val);
    MachineOperand *genMachineLabel(int block_no);
    MachineOperand *genMachineReg(int reg, bool fpu);
};

// meaningless instruction, used as the head node of the instruction list.
class DummyInstruction : public Instruction
{
public:
    DummyInstruction() : Instruction(-1, nullptr) {};
    void output() const {};
    void genMachineCode(AsmBuilder*) {};
};

class AllocaInstruction : public Instruction
{
public:
    AllocaInstruction(Operand *dst, SymbolEntry *se, BasicBlock *insert_bb = nullptr);
    ~AllocaInstruction();
    void output() const;
    Operand *getDef() { return operands[0]; }
    void replaceDef(Operand *rep)
    {
        operands[0]->setDef(nullptr);
        operands[0] = rep;
        operands[0]->setDef(this);
    }
    SymbolEntry *getEntry(){return se;}
    void setDef(Operand *rep)
    {
        operands[0] = rep;
        operands[0]->setDef(this);
    }
    void genMachineCode(AsmBuilder*);
private:
    SymbolEntry *se;
};

class LoadInstruction : public Instruction
{
public:
    LoadInstruction(Operand *dst, Operand *src_addr, BasicBlock *insert_bb = nullptr);
    ~LoadInstruction();
    void output() const;
    Operand *getDef() { return operands[0]; }
    std::vector<Operand *> getUse() { return {operands[1]}; }
    void replaceUse(Operand *old, Operand *rep)
    {
        if (operands[1] == old)
        {
            operands[1]->removeUse(this);
            operands[1] = rep;
            rep->addUse(this);
        }
    }    
    void replaceDef(Operand *rep)
    {
        operands[0]->setDef(nullptr);
        operands[0] = rep;
        operands[0]->setDef(this);
    }
    void setDef(Operand *rep)
    {
        operands[0] = rep;
        operands[0]->setDef(this);
    }
    void genMachineCode(AsmBuilder*);
};

class GepInstruction : public Instruction
{
public:
    GepInstruction(Operand *dst, Operand *base, std::vector<Operand *> offs, BasicBlock *insert_bb = nullptr, bool type2 = false);
    void output() const;
    Operand *getDef()
    {
        return operands[0];
    }
    std::vector<Operand *> getUse()
    {
        // 检查 operands 是否有足够的元素（至少两个）以返回非空的 vector
        if (operands.size() > 1)
            return std::vector<Operand *>(operands.begin() + 1, operands.end());
        else
            // 如果没有足够的元素，返回一个空的 vector
            return std::vector<Operand *>();
    }
    void replaceDef(Operand *rep)
    {
        if (operands[0])
        {
            operands[0]->setDef(nullptr);
            operands[0] = rep;
            operands[0]->setDef(this);
        }
    }
    void setDef(Operand *rep)
    {
        if (operands[0])
        {
            operands[0] = rep;
            operands[0]->setDef(this);
        }
    }
    void replaceUse(Operand *old, Operand *rep)
    {
        for (size_t i = 1; i < operands.size(); i++)
        {
            if (operands[i] == old)
            {
                operands[i]->removeUse(this);
                operands[i] = rep;
                rep->addUse(this);
            }
        }
    }
    void genMachineCode(AsmBuilder *);
private:
    bool type2 = false;
};

class StoreInstruction : public Instruction
{
public:
    StoreInstruction(Operand *dst_addr, Operand *src, BasicBlock *insert_bb = nullptr);
    ~StoreInstruction();
    void output() const;
    std::vector<Operand *> getUse() { return {operands[0], operands[1]}; }
    void replaceUse(Operand *old, Operand *rep)
    {
        if (operands[0] == old)
        {
            operands[0]->removeUse(this);
            operands[0] = rep;
            rep->addUse(this);
        }
        else if (operands[1] == old)
        {
            operands[1]->removeUse(this);
            operands[1] = rep;
            rep->addUse(this);
        }
    }    
    void genMachineCode(AsmBuilder *);
};

class BinaryInstruction : public Instruction
{
private:
    bool forbidSremSplit = false;
public:
    BinaryInstruction(unsigned opcode, Operand *dst, Operand *src1, Operand *src2, BasicBlock *insert_bb = nullptr);
    ~BinaryInstruction();
    void output() const;
    enum {SUB, ADD, MUL,DIV,MOD,AND, OR};
    Operand *getDef() { return operands[0]; }
    std::vector<Operand *> getUse() { return {operands[1], operands[2]}; }
    void replaceUse(Operand *old, Operand *rep)
    {
        if (operands[1] == old)
        {
            operands[1]->removeUse(this);
            operands[1] = rep;
            rep->addUse(this);
        }
        else if (operands[2] == old)
        {
            operands[2]->removeUse(this);
            operands[2] = rep;
            rep->addUse(this);
        }
    }    
    void replaceDef(Operand *rep)
    {
        operands[0]->setDef(nullptr);
        operands[0] = rep;
        operands[0]->setDef(this);
    }
    void setDef(Operand *rep)
    {
        operands[0] = rep;
        operands[0]->setDef(this);
    }
    void genMachineCode(AsmBuilder *);
    bool is64Bit() { return bit64; };
    void set64Bit(bool bit64) { this->bit64 = bit64; };


    bool bit64 = false;
    bool forbidSremSpl() {return this->forbidSremSplit;};
    void setForbidSremSplit(bool fss) {this->forbidSremSplit = fss;};

};

class CmpInstruction : public Instruction
{
public:
    CmpInstruction(unsigned opcode, Operand *dst, Operand *src1, Operand *src2, BasicBlock *insert_bb = nullptr);
    ~CmpInstruction();
    void output() const;
    enum {E, NE, L, GE, G, LE};
    Operand *getDef() { return operands[0]; }
    std::vector<Operand *> getUse() { return {operands[1], operands[2]}; }
    void replaceUse(Operand *old, Operand *rep)
    {
        if (operands[1] == old)
        {
            operands[1]->removeUse(this);
            operands[1] = rep;
            rep->addUse(this);
        }
        else if (operands[2] == old)
        {
            operands[2]->removeUse(this);
            operands[2] = rep;
            rep->addUse(this);
        }
    }
    void replaceDef(Operand *rep)
    {
        operands[0]->setDef(nullptr);
        operands[0] = rep;
        operands[0]->setDef(this);
    }
    void setDef(Operand *rep)
    {
        operands[0] = rep;
        operands[0]->setDef(this);
    }
    void genMachineCode(AsmBuilder *);
};

// unconditional branch
class UncondBrInstruction : public Instruction
{
public:
    UncondBrInstruction(BasicBlock*, BasicBlock *insert_bb = nullptr);
    void output() const;
    void setBranch(BasicBlock *);
    BasicBlock *getBranch();
    BasicBlock **patchBranch() {return &branch;};
    void genMachineCode(AsmBuilder *);
protected:
    BasicBlock *branch;
};

// conditional branch
class CondBrInstruction : public Instruction
{
public:
    CondBrInstruction(BasicBlock*, BasicBlock*, Operand *, BasicBlock *insert_bb = nullptr);
    ~CondBrInstruction();
    void output() const;
    void setTrueBranch(BasicBlock*);
    BasicBlock* getTrueBranch();
    void setFalseBranch(BasicBlock*);
    BasicBlock* getFalseBranch();
    BasicBlock **patchBranchTrue() {return &true_branch;};
    BasicBlock **patchBranchFalse() {return &false_branch;};
    std::vector<Operand *> getUse() { return {operands[0]}; }
    void replaceUse(Operand *old, Operand *rep)
    {
        if (operands[0] == old)
        {
            operands[0]->removeUse(this);
            operands[0] = rep;
            rep->addUse(this);
        }
    }
    void genMachineCode(AsmBuilder *);
protected:
    BasicBlock* true_branch;
    BasicBlock* false_branch;
};

class RetInstruction : public Instruction
{
public:
    RetInstruction(Operand *src, BasicBlock *insert_bb = nullptr);
    ~RetInstruction();
    std::vector<Operand *> getUse()
    {
        if (operands.size())
            return {operands[0]};
        else
            return {};
    }
    void output() const;
    void replaceUse(Operand *old, Operand *rep)
    {
        if (operands.size() && operands[0] == old)
        {
            operands[0]->removeUse(this);
            operands[0] = rep;
            rep->addUse(this);
        }
    }   
    void replaceDef(Operand *rep)
    {
        if (operands.size())
        {
            operands[0]->setDef(nullptr);
            operands[0] = rep;
            operands[0]->setDef(this);
        }
    } 
    void setDef(Operand *rep)
    {
        if (operands.size())
        {
            operands[0] = rep;
            operands[0]->setDef(this);
        }
    }
    void genMachineCode(AsmBuilder *);
};

//添加起始

class ZextInstruction : public Instruction
{
public:
    ZextInstruction(Operand *dst, Operand *src, BasicBlock *insertBB = nullptr);
    ~ZextInstruction() {
        operands[0]->setDef(nullptr);
        if (operands[0]->usersNum() == 0)
            delete operands[0];
        operands[1]->removeUse(this);
    }
    void output() const;
    void replaceUse(Operand *old, Operand *rep)
    {
        if (operands[1] == old)
        {
            operands[1]->removeUse(this);
            operands[1] = rep;
            rep->addUse(this);
        }
    }
    void replaceDef(Operand *rep)
    {
        operands[0]->setDef(nullptr);
        operands[0] = rep;
        operands[0]->setDef(this);
    }
    Operand *getDef()
    {
        return operands[0];
    }
    void setDef(Operand *rep)
    {
        operands[0] = rep;
        operands[0]->setDef(this);
    }
    std::vector<Operand *> getUse() { return {operands[1]}; }
    void genMachineCode(AsmBuilder *);
};

class IntFloatCastInstruction : public Instruction
{
public:
    IntFloatCastInstruction(unsigned opcode, Operand *dst, Operand *src, BasicBlock *insert_bb = nullptr);
    void output() const;
    enum
    {
        S2F,
        F2S
    };
    void replaceUse(Operand *old, Operand *rep)
    {
        if (operands[1] == old)
        {
            operands[1]->removeUse(this);
            operands[1] = rep;
            rep->addUse(this);
        }
    }
    void replaceDef(Operand *rep)
    {
        operands[0]->setDef(nullptr);
        operands[0] = rep;
        operands[0]->setDef(this);
    }
    Operand *getDef()
    {
        return operands[0];
    }
    void setDef(Operand *rep)
    {
        operands[0] = rep;
        operands[0]->setDef(this);
    }
    std::vector<Operand *> getUse() { return {operands[1]}; }
    void genMachineCode(AsmBuilder *);
};

class NotInstruction : public Instruction
{
public:
    NotInstruction(Operand *dst, Operand *src, BasicBlock *insert_bb = nullptr);
    ~NotInstruction();
    void output() const;
    void replaceUse(Operand *old, Operand *rep)
    {
        if (operands[1] == old)
        {
            operands[1]->removeUse(this);
            operands[1] = rep;
            rep->addUse(this);
        }
    }
    void replaceDef(Operand *rep)
    {
        operands[0]->setDef(nullptr);
        operands[0] = rep;
        operands[0]->setDef(this);
    }
    Operand *getDef()
    {
        return operands[0];
    }
    void setDef(Operand *rep)
    {
        operands[0] = rep;
        operands[0]->setDef(this);
    }
    std::vector<Operand *> getUse() { return {operands[1]}; }
    void genMachineCode(AsmBuilder *);
};

class CallInstruction : public Instruction
{
private:
    SymbolEntry *func;
    bool isTailCall;
public:

    CallInstruction(Operand *dst,
                    SymbolEntry *func,
                    std::vector<Operand *> params,
                    BasicBlock *insert_bb = nullptr);
    void output() const;
    SymbolEntry *getFunc() { return func; };
    Operand *getDef()
    {
        return operands[0];
    }
    void replaceUse(Operand *old, Operand *rep)
    {
        for (size_t i = 1; i < operands.size(); i++)
        {
            if (operands[i] == old)
            {
                operands[i]->removeUse(this);
                operands[i] = rep;
                rep->addUse(this);
            }
        }
    }    
    void replaceDef(Operand *rep)
    {
        if (operands[0])
        {
            operands[0]->setDef(nullptr);
            operands[0] = rep;
            operands[0]->setDef(this);
        }
    }
    void setDef(Operand *rep)
    {
        if (operands[0])
        {
            operands[0] = rep;
            operands[0]->setDef(this);
        }
    }
    std::vector<Operand *> getUse()
    {
        // 检查 operands 是否有足够的元素（至少两个）以返回非空的 vector
        if (operands.size() > 1)
            return std::vector<Operand *>(operands.begin() + 1, operands.end());
        else
            // 如果没有足够的元素，返回一个空的 vector
            return std::vector<Operand *>();
    }

    void funcAddPred() ;
    void genMachineCode(AsmBuilder *);
    void setTailCall(bool tailCall) { this->isTailCall = tailCall; }
};

class GlobalInstruction : public Instruction
{
public:
    GlobalInstruction(Operand *dst, Operand *expr, SymbolEntry *se, BasicBlock *insertBB = nullptr);
    ~GlobalInstruction(){};
    void output() const;
    void genMachineCode(AsmBuilder *);
private:
    SymbolEntry *se;
};

class PhiInstruction : public Instruction
{
private:
    std::unordered_map<BasicBlock *, Operand *> srcs;
    Operand *addr; // old PTR

public:
    PhiInstruction(Operand *dst, BasicBlock *insert_bb = nullptr);
    ~PhiInstruction();
    void output() const;
    void addEdge(BasicBlock *block, Operand *src);
    Operand *getAddr() { return addr; }
    Operand *getEdge(BasicBlock *block) { return (srcs.find(block) != srcs.end()) ? srcs[block] : nullptr; }
    std::unordered_map<BasicBlock *, Operand *> &getSrcs() { return srcs; }
    bool findSrc(BasicBlock *block);
    Operand *getBlockSrc(BasicBlock *block);
    void removeBlockSrc(BasicBlock *block);
    void addSrc(BasicBlock *block, Operand *src);
    void removeUse(Operand *use);

    // void genMachineCode(AsmBuilder *)
    // {
    // }
    Operand *getDef() { return operands[0]; }
    std::vector<Operand *> getUse()
    {
        std::vector<Operand *> vec;
        for (auto &ope : operands)
            if (ope != operands[0])
                vec.push_back(ope);
        return vec;
    }
    void replaceUse(Operand *old, Operand *rep)
    {
        for (auto &it : srcs)
        {
            if (it.second == old)
            {
                it.second->removeUse(this);
                it.second = rep;
                rep->addUse(this);
            }
        }
        for (auto it = operands.begin() + 1; it != operands.end(); it++)
            if (*it == old)
                *it = rep;
    }
    void replaceDef(Operand *rep)
    {
        operands[0]->setDef(nullptr);
        operands[0] = rep;
        operands[0]->setDef(this);
    }
    void setDef(Operand *rep)
    {
        operands[0] = rep;
        operands[0]->setDef(this);
    }
    void genMachineCode(AsmBuilder *);
};


class BitcastInstruction : public Instruction
{
    Operand *dst;
    Operand *src;

public:
    BitcastInstruction(Operand *dst, Operand *src, BasicBlock *insert_bb = nullptr);
    ~BitcastInstruction();
    Operand *getSrc() { return src; }
    void output() const;
    //void genMachineCode(AsmBuilder *);
    Operand *getDef() { return operands[0]; }
    std::vector<Operand *> getUse() { return {operands[1]}; }
    void replaceUse(Operand *old, Operand *rep)
    {
        if (operands[1] == old)
        {
            operands[1]->removeUse(this);
            operands[1] = rep;
            rep->addUse(this);
            src = rep;
        }
    }
    void replaceDef(Operand *rep)
    {
        operands[0]->setDef(nullptr);
        operands[0] = rep;
        operands[0]->setDef(this);
        dst = rep;
    }
    void setDef(Operand *rep)
    {
        operands[0] = rep;
        operands[0]->setDef(this);
        dst = rep;
    }
    void genMachineCode(AsmBuilder *);
};


#endif