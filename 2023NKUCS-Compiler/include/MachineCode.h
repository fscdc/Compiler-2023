#ifndef __MACHINECODE_H__
#define __MACHINECODE_H__
#include <vector>
#include <set>
#include <string>
#include <algorithm>
#include <fstream>
#include "SymbolTable.h"

/*
 * MachineUnit: Compiler unit
 * MachineFunction: Function in assembly code
 * MachineInstruction: Single assembly instruction
 * MachineOperand: Operand in assembly instruction, such as immediate number, register, address label
 */

class MachineUnit;
class MachineFunction;
class MachineBlock;
class MachineInstruction;

class MachineOperand
{
private:
    MachineInstruction *parent;
    int type;
    int val;           // value of immediate number
    int reg_no;        // register no
    std::string label; // address label
    bool fpu = false;  //如果是ture就用s开头那系列寄存器
public:
    enum
    {
        IMM,
        VREG,
        REG,
        LABEL
    };
    MachineOperand(){};
    MachineOperand(int tp, int val, bool fpu = false);
    MachineOperand(std::string label);
    bool operator==(const MachineOperand &) const;
    bool operator<(const MachineOperand &) const;
    bool isImm() { return this->type == IMM; };
    bool isReg() { return this->type == REG; };
    bool isVReg() { return this->type == VREG; };
    bool isLabel() { return this->type == LABEL; };
    bool isFReg() { return this->fpu; }
    int getVal() { return this->val; };
    int getReg() { return this->reg_no; };
    void setRegNo(int regno) { this->reg_no = regno; };
    void setReg(int regno)
    {
        this->type = REG;
        this->reg_no = regno;
    };
    std::string getLabel() { return this->label; };
    void setParent(MachineInstruction *p) { this->parent = p; };
    MachineInstruction *getParent() { return this->parent; };
    std::string PrintReg();
    std::string toStr();
    void output();
};

class MachineInstruction
{
protected:
    MachineBlock *parent;
    int no;
    int type;                            // Instruction type
    int cond = MachineInstruction::NONE; // Instruction execution condition, optional !!
    int op;                              // Instruction opcode
    // Instruction operand list, sorted by appearance order in assembly instruction
    std::vector<MachineOperand *> def_list;
    std::vector<MachineOperand *> use_list;
    void addDef(MachineOperand *ope) { def_list.push_back(ope); };
    void addUse(MachineOperand *ope) { use_list.push_back(ope); };
    // Print execution code after printing opcode
    void PrintCond();
    enum instType
    {
        BINARY,
        LOAD,
        STORE,
        MOV,
        BRANCH,
        CMP,
        STACK,
        VCVT,
        VMRS,
        MLA,
        MLS,
        VNEG,
        OFFBINARY, 
        SMULL
    };

public:
    enum condType
    {
        EQ,
        NE,
        LT,
        LE,
        GT,
        GE,
        NONE
    };
    virtual void output() = 0;
    void setNo(int no) { this->no = no; };
    int getNo() { return no; };
    std::vector<MachineOperand *> &getDef() { return def_list; };
    std::vector<MachineOperand *> &getUse() { return use_list; };
    MachineBlock *getParent() { return parent; };
    void setParent(MachineBlock *parent) { this->parent = parent; };
    virtual bool replaceUse(MachineOperand *, MachineOperand *) { return false; };
    void setCond(int newCond) { cond = newCond; };
    int getCond() { return cond; };

    bool isStack() { return type == STACK; };
    bool isPOP() { return type == STACK && this->op == 1; }
    bool isVPOP() { return type == STACK && this->op == 3; }
    bool isSub() { return type == BINARY && this->op == 1; };
    bool isRet() { return type == BRANCH && op == 2; };
    bool isUncondBranch() { return type == BRANCH && (op == 2 || (op == 0 && cond == MachineInstruction::NONE)); };
    bool isUBranch() { return type == BRANCH && op == 0 && cond == MachineInstruction::NONE; }
    bool isBranch() { return type == BRANCH && op == 0; };
    bool isMovClass() { return type == MOV; };
    bool isMov() { return type == MOV && op == 0; };
    bool isVMov() { return type == MOV && op == 2; };
    bool isVMov32() { return type == MOV && op == 3; };
    bool isCondMov() { return type == MOV && cond != MachineInstruction::NONE; };
    bool isCall() { return type == BRANCH && op == 1; };
    bool isLoad() { return type == LOAD; };
    bool isCLoad() { return type == LOAD && this->op == 0; };
    bool isVLoad() { return type == LOAD && this->op == 1; };
    bool isStore() { return type == STORE; };
    bool isCStore() { return type == STORE && this->op == 0; };
    bool isVStore() { return type == STORE && this->op == 1; };
    bool isMul() { return type == BINARY && this->op == 2; };
    bool isAdd() { return type == BINARY && this->op == 0; };
    bool isVSub() { return type == BINARY && op == 7; };
    bool isBinary() { return type == BINARY; };
    bool isVBinary() { return type == BINARY && (op == 6 || op == 7 || op == 8 || op == 9); };

};

class BinaryMInstruction : public MachineInstruction
{
public:
    enum opType
    {
        ADD = 0,
        SUB,
        MUL,
        DIV,
        AND,
        OR,
        VADD, //向量加减乘除
        VSUB,
        VMUL,
        VDIV,
        LSL, //逻辑左移
        ASR, //算数右移
        LSR, //逻辑右移
    };
    BinaryMInstruction(MachineBlock *p, int op,
                       MachineOperand *dst, MachineOperand *src1, MachineOperand *src2,
                       int cond = MachineInstruction::NONE);
    void setStackSize(int stack_size)
    {
        use_list[1] = new MachineOperand(MachineOperand::IMM, stack_size);
    }
    bool replaceUse(MachineOperand *old, MachineOperand *rep)
    {
        for (size_t i = 0; i < use_list.size(); i++)
        {
            if (use_list[i] == old)
            {
                if (rep->isImm())
                {
                    if (i == 0)
                        break;
                    if (!(op == ADD || op == SUB || op == AND || op == OR || op == LSL || op == LSR || op == ASR))
                        break;
                }
                delete use_list[i];
                use_list[i] = rep;
                rep->setParent(this);
                return true;
            }
        }
        return false;
    }
    void output();
    std::string opStr();
};

class LoadMInstruction : public MachineInstruction
{
public:
    enum opType
    {
        LDR,
        VLDR
    };
    LoadMInstruction(MachineBlock *p, int op,
                     MachineOperand *dst, MachineOperand *src1, MachineOperand *src2 = nullptr,
                     int cond = MachineInstruction::NONE);
    void setOff(int offset)
    {
        use_list[1] = new MachineOperand(MachineOperand::IMM, use_list[1]->getVal() + offset);
    }
    bool replaceUse(MachineOperand *old, MachineOperand *rep)
    {
        if (rep->isImm())
            return false;
        for (size_t i = 0; i < use_list.size(); i++)
        {
            if (use_list[i] == old)
            {
                delete use_list[i];
                use_list[i] = rep;
                rep->setParent(this);
                return true;
            }
        }
        return false;
    }
    void output();
};

class StoreMInstruction : public MachineInstruction
{
public:
    enum opType
    {
        STR,
        VSTR
    };
    StoreMInstruction(MachineBlock *p, int op,
                      MachineOperand *src1, MachineOperand *src2, MachineOperand *src3 = nullptr,
                      int cond = MachineInstruction::NONE);
    bool replaceUse(MachineOperand *old, MachineOperand *rep)
    {
        if (rep->isImm())
            return false;
        for (size_t i = 0; i < use_list.size(); i++)
        {
            if (use_list[i] == old)
            {
                delete use_list[i];
                use_list[i] = rep;
                rep->setParent(this);
                return true;
            }
        }
        return false;
    }
    void output();
};

class MovMInstruction : public MachineInstruction
{
public:
    enum opType
    {
        MOV,
        MVN,
        VMOV,
        VMOV32
    };
    MovMInstruction(MachineBlock *p, int op,
                    MachineOperand *dst, MachineOperand *src,
                    int cond = MachineInstruction::NONE);
    bool replaceUse(MachineOperand *old, MachineOperand *rep)
    {
        if (rep->isImm() && (op == VMOV || op == VMOV32))
            return false;
        for (size_t i = 0; i < use_list.size(); i++)
        {
            if (use_list[i] == old)
            {
                delete use_list[i];
                use_list[i] = rep;
                rep->setParent(this);
                return true;
            }
        }
        return false;
    }
    void output();
};

class BranchMInstruction : public MachineInstruction
{
public:
    enum opType
    {
        B = 0,
        BL,
        BX
    };
    BranchMInstruction(MachineBlock *p, int op,
                       MachineOperand *dst,
                       int cond = MachineInstruction::NONE);
    void setTarget(MachineOperand *dst)
    {
        assert(this->op == B || this->op == BL);
        use_list.clear();
        use_list.push_back(dst);
        dst->setParent(this);
    };
    void output();
};

class CmpMInstruction : public MachineInstruction
{
public:
    enum opType
    {
        CMP,
        VCMP
    };
    CmpMInstruction(MachineBlock *p, int op,
                    MachineOperand *src1, MachineOperand *src2,
                    int cond = MachineInstruction::NONE);
    bool replaceUse(MachineOperand *old, MachineOperand *rep)
    {
        if (rep->isImm())
            return false;
        for (size_t i = 0; i < use_list.size(); i++)
        {
            if (use_list[i] == old)
            {
                delete use_list[i];
                use_list[i] = rep;
                rep->setParent(this);
                return true;
            }
        }
        return false;
    }
    void output();
};

class StackMInstruction : public MachineInstruction
{
public:
    enum opType
    {
        PUSH = 0,
        POP,
        VPUSH,
        VPOP
    };
    StackMInstruction(MachineBlock *p, int op,
                     std::vector<MachineOperand *> srcs,
                     int cond = MachineInstruction::NONE);
    void setRegs(std::vector<MachineOperand *> regs)
    {
        use_list.assign(regs.begin(), regs.end());
    }
    void output();
};

class VcvtMInstruction : public MachineInstruction
{
public:
    enum opType
    {
        FTS,
        STF
    };
    VcvtMInstruction(MachineBlock *p, int op,
                     MachineOperand *dst, MachineOperand *src,
                     int cond = MachineInstruction::NONE);
    bool replaceUse(MachineOperand *old, MachineOperand *rep)
    {
        if (rep->isImm())
            return false;
        for (size_t i = 0; i < use_list.size(); i++)
        {
            if (use_list[i] == old)
            {
                delete use_list[i];
                use_list[i] = rep;
                rep->setParent(this);
                return true;
            }
        }
        return false;
    }
    void output();
};

class VmrsMInstruction : public MachineInstruction
{
public:
    VmrsMInstruction(MachineBlock *p,
                     int cond = MachineInstruction::NONE);
    void output();
};

class MlaMInstruction : public MachineInstruction
{
public:
    MlaMInstruction(MachineBlock *p, MachineOperand *dst,
                    MachineOperand *src1, MachineOperand *src2, MachineOperand *src3,
                    int cond = MachineInstruction::NONE);
    bool replaceUse(MachineOperand *old, MachineOperand *rep)
    {
        if (rep->isImm())
            return false;
        for (size_t i = 0; i < use_list.size(); i++)
        {
            if (use_list[i] == old)
            {
                delete use_list[i];
                use_list[i] = rep;
                rep->setParent(this);
                return true;
            }
        }
        return false;
    }
    void output();
};

class MlsMInstruction : public MachineInstruction
{
public:
    MlsMInstruction(MachineBlock *p, MachineOperand *dst,
                    MachineOperand *src1, MachineOperand *src2, MachineOperand *src3,
                    int cond = MachineInstruction::NONE);
    bool replaceUse(MachineOperand *old, MachineOperand *rep)
    {
        if (rep->isImm())
            return false;
        for (size_t i = 0; i < use_list.size(); i++)
        {
            if (use_list[i] == old)
            {
                delete use_list[i];
                use_list[i] = rep;
                rep->setParent(this);
                return true;
            }
        }
        return false;
    }
    void output();
};

class OffsetBinaryMInstruction : public MachineInstruction
{
public:
    int offset;
    OffsetBinaryMInstruction(MachineBlock *p, int op, MachineOperand *dst,
                             MachineOperand *src1, MachineOperand *src2, MachineOperand *src3, int offset,
                             int cond = MachineInstruction::NONE);
    bool replaceUse(MachineOperand *old, MachineOperand *rep)
    {
        if (rep->isImm())
            return false;
        for (size_t i = 0; i < use_list.size(); i++)
        {
            if (use_list[i] == old)
            {
                delete use_list[i];
                use_list[i] = rep;
                rep->setParent(this);
                return true;
            }
        }
        return false;
    }
    void output();
    enum
    {
        ADD,
        SUB
    };
    enum
    {
        LSL,
        ASR
    };
};

class SMULLMInstruction : public MachineInstruction
{
public:
    SMULLMInstruction(MachineBlock *p, MachineOperand *lo,
                    MachineOperand *hi, MachineOperand *src1, MachineOperand *src2,
                    int cond = MachineInstruction::NONE);
    bool replaceUse(MachineOperand *old, MachineOperand *rep)
    {
        if (rep->isImm())
            return false;
        for (size_t i = 0; i < use_list.size(); i++)
        {
            if (use_list[i] == old)
            {
                delete use_list[i];
                use_list[i] = rep;
                rep->setParent(this);
                return true;
            }
        }
        return false;
    }
    void output();
};

class VNegMInstruction : public MachineInstruction
{
public:
    VNegMInstruction(MachineBlock *p, MachineOperand *dst,
                     MachineOperand *src, int cond = MachineInstruction::NONE);
    bool replaceUse(MachineOperand *old, MachineOperand *rep)
    {
        if (rep->isImm())
            return false;
        for (size_t i = 0; i < use_list.size(); i++)
        {
            if (use_list[i] == old)
            {
                delete use_list[i];
                use_list[i] = rep;
                rep->setParent(this);
                return true;
            }
        }
        return false;
    }
    void output();
};

class MachineBlock
{
private:
    static unsigned long long inst_num; //用来定指令数就打印文字池
    MachineFunction *parent;
    int no;
    std::vector<MachineBlock *> pred, succ;
    std::vector<MachineInstruction *> inst_list;

public:
    std::set<MachineOperand *> live_in;
    std::set<MachineOperand *> live_out;
    std::vector<MachineInstruction *> &getInsts() { return inst_list; };
    std::vector<MachineInstruction *>::iterator begin() { return inst_list.begin(); };
    std::vector<MachineInstruction *>::iterator end() { return inst_list.end(); };
    std::vector<MachineInstruction *>::reverse_iterator rbegin() { return inst_list.rbegin(); };
    std::vector<MachineInstruction *>::reverse_iterator rend() { return inst_list.rend(); };
    MachineBlock(MachineFunction *p, int no)
    {
        this->parent = p;
        this->no = no;
    };
    void InsertInst(MachineInstruction *inst) { this->inst_list.push_back(inst); };
    void addPred(MachineBlock *p)
    {
        // 自动去重
        if (std::find(this->pred.begin(), this->pred.end(), p) == this->pred.end())
            this->pred.push_back(p);
    };
    void removePred(MachineBlock *p) { this->pred.erase(std::find(this->pred.begin(), this->pred.end(), p)); };
    void addSucc(MachineBlock *s)
    {
        // 自动去重
        if (std::find(this->succ.begin(), this->succ.end(), s) == this->succ.end())
            this->succ.push_back(s);
    };
    void removeSucc(MachineBlock *s) { this->succ.erase(std::find(this->succ.begin(), this->succ.end(), s)); };
    void insertBefore(MachineInstruction *, MachineInstruction *);
    void insertAfter(MachineInstruction *, MachineInstruction *);
    void insertFront(MachineInstruction *inst) { this->inst_list.insert(inst_list.begin(), inst); };
    void eraseInst(MachineInstruction *inst)
    {
        auto it = find(inst_list.begin(), inst_list.end(), inst);
        if (it != inst_list.end())
            this->inst_list.erase(it);
    };
    inline std::set<MachineOperand *> &getLiveIn() { return live_in; };
    inline std::set<MachineOperand *> &getLiveOut() { return live_out; };
    std::vector<MachineBlock *> &getPreds() { return pred; };
    std::vector<MachineBlock *> &getSuccs() { return succ; };
    MachineFunction *getParent() { return parent; };
    int getNo() { return no; };
    void output();
};

class MachineFunction
{
private:
    MachineUnit *parent;
    std::vector<MachineBlock *> block_list;
    int stack_size;
    std::set<int> saved_regs;
    SymbolEntry *sym_ptr;
    MachineBlock *entry;
    std::vector<MachineInstruction *> unsure_insts;

public:
    void setEntry(MachineBlock *block) { entry = block; };
    MachineBlock *getEntry() { return entry; };
    SymbolEntry *getSymbolEntry() { return sym_ptr; };
    std::vector<MachineBlock *> &getBlocks() { return block_list; };
    std::vector<MachineBlock *>::iterator begin() { return block_list.begin(); };
    std::vector<MachineBlock *>::iterator end() { return block_list.end(); };
    MachineFunction(MachineUnit *p, SymbolEntry *sym_ptr);
    /* HINT:
     * Alloc stack space for local variable;
     * return current frame offset ;
     * we store offset in symbol entry of this variable in function AllocInstruction::genMachineCode()
     * you can use this function in LinearScan::genSpillCode() */
    int AllocSpace(int size)
    {
        this->stack_size += size;
        return this->stack_size;
    };
    void InsertBlock(MachineBlock *block) { this->block_list.push_back(block); };
    void InsertFront(MachineBlock *block) { this->block_list.insert(this->block_list.begin(), block); };
    void RemoveBlock(MachineBlock *block) { this->block_list.erase(std::find(this->block_list.begin(), this->block_list.end(), block)); };
    void backPatch(std::vector<MachineOperand *>);
    void addUInst(MachineInstruction *inst) { unsure_insts.push_back(inst); };
    void addSavedRegs(int regno) { saved_regs.insert(regno); };
    std::vector<MachineOperand *> getAllSavedRegs();
    std::vector<MachineOperand *> getSavedRegs();
    std::vector<MachineOperand *> getSavedFRegs();
    MachineUnit *getParent() { return parent; };
    void output();
};

class MachineUnit
{
private:
    std::vector<MachineFunction *> func_list;
    std::vector<SymbolEntry *> global_vars;
    int ltorg_num = 0;
    void PrintGlobalDecl();

public:
    std::vector<MachineFunction *> &getFuncs() { return func_list; };
    std::vector<MachineFunction *>::iterator begin() { return func_list.begin(); };
    std::vector<MachineFunction *>::iterator end() { return func_list.end(); };
    void InsertFunc(MachineFunction *func) { func_list.push_back(func); };
    void setGlobalVars(std::vector<SymbolEntry *> gv) { global_vars = gv; };
    void printLTORG(); // 打印文字池
    int getLtorgNum() { return ltorg_num; };
    void output();
    /* 这里两个优化都用到这个函数，放到MachineUnit里面吧 */
    static int getHash(MachineOperand *op)
    {
        /* 普通的虚拟寄存器直接返回编号，r系列的rx哈希为-x-33，s系列的sx哈希为-x-1 */
        if (op->isImm() || op->isLabel())
            return 0;
        int res = 0;
        res = op->getReg();
        if (op->isReg())
        {
            if (!op->isFReg())
                res = -res - 33;
            else
                res = -res - 1;
        }
        return res;
    }
};

#endif