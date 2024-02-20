#include "Instruction.h"
#include "BasicBlock.h"
#include <iostream>
#include <cstring>
#include "Function.h"
#include "Type.h"
#include <sstream>
extern FILE* yyout;

#define genMachineOperand(name) genMOperand(name, builder)

Instruction::Instruction(unsigned instType, BasicBlock *insert_bb)
{
    prev = next = this;
    opcode = -1;
    this->instType = instType;
    if (insert_bb != nullptr)
    {
        insert_bb->insertBack(this);
        parent = insert_bb;
    }
}

Instruction::~Instruction()
{
    parent->remove(this);
}

BasicBlock *Instruction::getParent()
{
    return parent;
}

void Instruction::setParent(BasicBlock *bb)
{
    parent = bb;
}

void Instruction::setNext(Instruction *inst)
{
    next = inst;
}

void Instruction::setPrev(Instruction *inst)
{
    prev = inst;
}

Instruction *Instruction::getNext()
{
    return next;
}

Instruction *Instruction::getPrev()
{
    return prev;
}

bool Instruction::isCritical()
{
    if (isRet())
    {
        if (getUse().empty())
            return true;
        
        //找到当前指令所在函数，然后再找到调用了这个函数的那些指令，如果没人调用则是关键的
        //即孤立函数是关键的，比如main
        auto callPreds = parent->getParent()->getCallPred();
        if (callPreds.empty())
            return true;
        
        //遍历这些调用指令，检查这些调用的结果是否被使用，如果使用了，那么是关键的
        for (auto it : callPreds)
            if (it->getDef()->usersNum())
                return true;
        //如果所有 call 指令的结果都没有被使用，即没有其他指令依赖于这些 call 指令的返回值，那么当前函数的 ret 指令不被视为关键的
        return false;
    }
    //这里的处理是：call指令都不能删（参考资料）
    if (isCall())
    {
        return true;
    }
    //store指令所在函数被调用过，则该指令关键（不严谨，比如main）
    if (isStore())
    {
        auto callPreds = parent->getParent()->getCallPred();
        if (!callPreds.empty())
            return true;
        return true;
    }
    return false;
}

MachineOperand *Instruction::genMOperand(Operand *ope, AsmBuilder *builder = nullptr)
{
    auto se = ope->getSymbolEntry();
    MachineOperand *mope = nullptr;
    if (se->isConstant())
    {
        // 如果一个立即数是浮点数，我们把它当成一个无符号32位数。
        if (se->getType()->isFloat())
        {
            float value = (float)dynamic_cast<ConstantSymbolEntry *>(se)->getValue();
            uint32_t v = reinterpret_cast<uint32_t &>(value);
            mope = new MachineOperand(MachineOperand::IMM, v);
        }
        else
            mope = new MachineOperand(MachineOperand::IMM, dynamic_cast<ConstantSymbolEntry *>(se)->getValue());
    }
    else if (se->isTemporary())
    {
        if (((TemporarySymbolEntry *)se)->isParam() && builder)
        {
            int argNum = dynamic_cast<TemporarySymbolEntry *>(se)->getArgNum(); //获取参数编号
            if (se->getType()->isFloat())
            {
                if (argNum < 16 && argNum >= 0)
                {
                    mope = new MachineOperand(MachineOperand::REG, argNum, true);
                }
                else
                { // 要从栈里加载
                    mope = new MachineOperand(MachineOperand::VREG, SymbolTable::getLabel(), true);
                    auto cur_block = builder->getBlock();
                    auto cur_inst = new LoadMInstruction(cur_block, LoadMInstruction::VLDR, new MachineOperand(*mope), new MachineOperand(MachineOperand::REG, 11), new MachineOperand(MachineOperand::IMM, 4 * -(argNum + 1)));
                    cur_block->InsertInst(cur_inst);
                    cur_block->getParent()->addUInst(cur_inst);
                }
            }
            else
            {
                if (argNum < 4 && argNum >= 0)
                {
                    mope = new MachineOperand(MachineOperand::REG, argNum);
                }
                else
                { // 要从栈里加载
                    mope = new MachineOperand(MachineOperand::VREG, SymbolTable::getLabel());
                    auto cur_block = builder->getBlock();
                    auto cur_inst = new LoadMInstruction(cur_block, LoadMInstruction::LDR, new MachineOperand(*mope), new MachineOperand(MachineOperand::REG, 11), new MachineOperand(MachineOperand::IMM, 4 * -(argNum + 1)));
                    cur_block->InsertInst(cur_inst);
                    cur_block->getParent()->addUInst(cur_inst);
                }
            }
        }
        else
        {
            if (se->getType()->isFloat())
                mope = new MachineOperand(MachineOperand::VREG, dynamic_cast<TemporarySymbolEntry *>(se)->getLabel(), true);
            else
                mope = new MachineOperand(MachineOperand::VREG, dynamic_cast<TemporarySymbolEntry *>(se)->getLabel());
        }
    }
    else if (se->isVariable())
    {
        auto id_se = dynamic_cast<IdentifierSymbolEntry *>(se);
        if (id_se->isGlobal())
            mope = new MachineOperand(id_se->toStr().c_str() + 1);
        else
            exit(0);
    }
    return mope;
}

MachineOperand *Instruction::fimmToVReg(MachineOperand *imm, MachineBlock *cur_block, AsmBuilder *builder)
{
    assert(imm->isImm());
    auto internal_reg = genMachineVReg(true);
    // 可以用vmov32，就用vmov32
    if (builder->couldUseVMOV(imm->getVal()))
    {
        cur_block->InsertInst(new MovMInstruction(cur_block, MovMInstruction::VMOV32, internal_reg, imm));
    }
    else
    {
        imm = new MachineOperand(*immToVReg(imm, cur_block));
        cur_block->InsertInst(new MovMInstruction(cur_block, MovMInstruction::VMOV, internal_reg, imm));
    }
    return internal_reg;
}

MachineOperand *Instruction::genMachineVReg(bool fpu = false)
{
    return new MachineOperand(MachineOperand::VREG, SymbolTable::getLabel(), fpu);
}

MachineOperand *Instruction::immToVReg(MachineOperand *imm, MachineBlock *cur_block)
{
    assert(imm->isImm());
    int value = imm->getVal();
    auto internal_reg = genMachineVReg();
    if (AsmBuilder::isLegalImm(value))
    {
        auto cur_inst = new MovMInstruction(cur_block, MovMInstruction::MOV, internal_reg, imm);
        cur_block->InsertInst(cur_inst);
    }
    else if (AsmBuilder::isLegalImm(~value))
    {
        auto cur_inst = new MovMInstruction(cur_block, MovMInstruction::MVN, internal_reg, genMachineImm(~value));
        cur_block->InsertInst(cur_inst);
    }
    else
    {
        auto cur_inst = new LoadMInstruction(cur_block, LoadMInstruction::LDR, internal_reg, imm);
        cur_block->InsertInst(cur_inst);
    }
    return internal_reg;
}

MachineOperand *Instruction::genMachineImm(int val)
{
    return new MachineOperand(MachineOperand::IMM, val);
}

MachineOperand *Instruction::genMachineLabel(int block_no)
{
    std::ostringstream buf;
    buf << ".L" << block_no;
    std::string label = buf.str();
    return new MachineOperand(label);
}

MachineOperand *Instruction::genMachineReg(int reg, bool fpu = false)
{
    return new MachineOperand(MachineOperand::REG, reg, fpu);
}

BinaryInstruction::BinaryInstruction(unsigned opcode, Operand *dst, Operand *src1, Operand *src2, BasicBlock *insert_bb) : Instruction(BINARY, insert_bb)
{
    this->opcode = opcode;
    operands.push_back(dst);
    operands.push_back(src1);
    operands.push_back(src2);
    dst->setDef(this);
    src1->addUse(this);
    src2->addUse(this);
    floatVersion = (src1->getType()->isFloat() || src2->getType()->isFloat());
}

BinaryInstruction::~BinaryInstruction()
{
    operands[0]->setDef(nullptr);
    operands[1]->removeUse(this);
    operands[2]->removeUse(this);
}

void BinaryInstruction::output() const
{
    std::string s1, s2, s3, op, type;
    s1 = operands[0]->toStr();
    s2 = operands[1]->toStr();
    s3 = operands[2]->toStr();
    type = operands[0]->getType()->toStr();
    if(operands[0]->getType()==TypeSystem::intType || operands[0]->getType()==TypeSystem::constIntType){
        switch (opcode)
        {
        case ADD:
            op = "add";
            break;
        case SUB:
            op = "sub";
            break;
        case MUL:
            op = "mul";
            break;
        case DIV:
            op = "sdiv";
            break;
        case MOD:
            op = "srem";
            break;    
        default:
            break;
        }
    }
    else{
        switch (opcode)
        {
        case ADD:
            op = "fadd";
            break;
        case SUB:
            op = "fsub";
            break;
        case MUL:
            op = "fmul";
            break;
        case DIV:
            op = "fdiv";
            break;
        default:
            break;
        }        
    }
    fprintf(yyout, "  %s = %s %s %s, %s\n", s1.c_str(), op.c_str(), type.c_str(), s2.c_str(), s3.c_str());
}

void BinaryInstruction::genMachineCode(AsmBuilder* builder) {
    auto cur_block = builder->getBlock();
    auto dst = genMachineOperand(operands[0]);
    auto src1 = genMachineOperand(operands[1]);
    auto src2 = genMachineOperand(operands[2]);
    MachineInstruction *cur_inst = nullptr;
    // 加法，这里交换一下
    if (opcode == BinaryInstruction::MUL && bit64)
    {
        // 是一条smull指令
        auto lo = genMachineVReg();
        auto hi = genMachineVReg();
        if (src1->isImm())
            src1 = new MachineOperand(*immToVReg(src1, cur_block));
        if (src2->isImm())
            src2 = new MachineOperand(*immToVReg(src2, cur_block));
        cur_block->InsertInst(new SMULLMInstruction(cur_block, lo, hi, src1, src2));
        builder->smullSig2Doub[operands[0]] = std::make_pair(lo, hi);
        return;
    }
    else if (opcode == BinaryInstruction::MOD && bit64)
    {
        auto lo = builder->smullSig2Doub[operands[1]].first;
        auto hi = builder->smullSig2Doub[operands[1]].second;
        lo = new MachineOperand(*lo);
        hi = new MachineOperand(*hi);
        cur_block->InsertInst(new MovMInstruction(cur_block, MovMInstruction::MOV, genMachineReg(0), lo));
        cur_block->InsertInst(new MovMInstruction(cur_block, MovMInstruction::MOV, genMachineReg(1), hi));
        if (src2->isImm())
            cur_block->InsertInst(new MovMInstruction(cur_block, MovMInstruction::MOV, genMachineReg(2), new MachineOperand(*immToVReg(src2, cur_block))));
        else
            cur_block->InsertInst(new MovMInstruction(cur_block, MovMInstruction::MOV, genMachineReg(2), src2));
        cur_block->InsertInst(new MovMInstruction(cur_block, MovMInstruction::MOV, genMachineReg(3), genMachineImm(0)));
        cur_block->InsertInst(new BranchMInstruction(cur_block, BranchMInstruction::BL, new MachineOperand("@__aeabi_ldivmod")));
        cur_block->InsertInst(new MovMInstruction(cur_block, MovMInstruction::MOV, dst, genMachineReg(2)));
        return;
    }
    if ((opcode == BinaryInstruction::ADD || opcode == BinaryInstruction::MUL) && src1->isImm() && !src2->isImm())
        std::swap(src1, src2);
    if (src1->isImm())
    {
        if (floatVersion)
            src1 = new MachineOperand(*fimmToVReg(src1, cur_block, builder));
        else
            src1 = new MachineOperand(*immToVReg(src1, cur_block));
    }
    if (src2->isImm())
    {
        // 为零可以特殊处理
        if (src2->getVal() == 0) // 注意这里，因为浮点数0的位模式全零，也可以这样判断
        {
            if (opcode == ADD || opcode == SUB)
            {
                cur_block->InsertInst(new MovMInstruction(cur_block, floatVersion ? MovMInstruction::VMOV32 : MovMInstruction::MOV, dst, src1));
                return;
            }
            else if (opcode == MUL)
            {
                if (floatVersion)
                    cur_block->InsertInst(new BinaryMInstruction(cur_block, BinaryMInstruction::VSUB, dst, new MachineOperand(*dst), new MachineOperand(*dst)));
                else
                    cur_block->InsertInst(new MovMInstruction(cur_block, MovMInstruction::MOV, dst, genMachineImm(0)));
                return;
            }
        }
        if (src2->getVal() == 1 && (opcode == MUL || opcode == DIV) && !floatVersion)
        {
            cur_block->InsertInst(new MovMInstruction(cur_block, MovMInstruction::MOV, dst, src1));
            return;
        }
        if (floatVersion) // 如果是浮点数，直接放寄存器里得了
        {
            src2 = new MachineOperand(*fimmToVReg(src2, cur_block, builder));
        }
        else if (opcode == MUL || opcode == DIV || opcode == MOD || !AsmBuilder::isLegalImm(src2->getVal()))
        {
            // int类型，按需放寄存器里
            src2 = new MachineOperand(*immToVReg(src2, cur_block));
        }
    }
    switch (opcode)
    {
    case ADD:
        cur_inst = new BinaryMInstruction(cur_block,
                                          floatVersion ? BinaryMInstruction::VADD : BinaryMInstruction::ADD,
                                          dst,
                                          src1,
                                          src2);
        break;
    case SUB:
        cur_inst = new BinaryMInstruction(cur_block,
                                          floatVersion ? BinaryMInstruction::VSUB : BinaryMInstruction::SUB,
                                          dst,
                                          src1,
                                          src2);
        break;
    case MUL:
        cur_inst = new BinaryMInstruction(cur_block,
                                          floatVersion ? BinaryMInstruction::VMUL : BinaryMInstruction::MUL,
                                          dst,
                                          src1,
                                          src2);
        break;
    case DIV:
        cur_inst = new BinaryMInstruction(cur_block,
                                          floatVersion ? BinaryMInstruction::VDIV : BinaryMInstruction::DIV,
                                          dst,
                                          src1,
                                          src2);
        break;
    case AND: // 下边这两种情况，操作数不会是浮点数，因为已经被隐式转化了
        cur_inst = new BinaryMInstruction(cur_block, BinaryMInstruction::AND, dst, src1, src2);
        break;
    case OR:
        cur_inst = new BinaryMInstruction(cur_block, BinaryMInstruction::OR, dst, src1, src2);
        break;
    case MOD:
        {
            cur_block->InsertInst(new BinaryMInstruction(cur_block, BinaryMInstruction::DIV, dst, src1, src2));
            cur_block->InsertInst(new BinaryMInstruction(cur_block, BinaryMInstruction::MUL, new MachineOperand(*dst), new MachineOperand(*dst), new MachineOperand(*src2)));
            cur_inst = new BinaryMInstruction(cur_block, BinaryMInstruction::SUB, new MachineOperand(*dst), new MachineOperand(*src1), new MachineOperand(*dst));
        }
        break;
    default:
        break;
    }
    cur_block->InsertInst(cur_inst);
}

CmpInstruction::CmpInstruction(unsigned opcode, Operand *dst, Operand *src1, Operand *src2, BasicBlock *insert_bb): Instruction(CMP, insert_bb){
    this->opcode = opcode;
    operands.push_back(dst);
    operands.push_back(src1);
    operands.push_back(src2);
    dst->setDef(this);
    src1->addUse(this);
    src2->addUse(this);
    dst->getSymbolEntry()->setType(TypeSystem::boolType);
    floatVersion = (src1->getType()->isFloat() || src2->getType()->isFloat());
}

CmpInstruction::~CmpInstruction()
{
    operands[0]->setDef(nullptr);
    if (operands[0]->usersNum() == 0)
    {
        delete operands[0];
        operands[0] = nullptr;
    }
    operands[1]->removeUse(this);
    operands[2]->removeUse(this);
}

void CmpInstruction::output() const
{
    std::string s1, s2, s3, op, type;
    s1 = operands[0]->toStr();
    s2 = operands[1]->toStr();
    s3 = operands[2]->toStr();
    type = operands[1]->getType()->toStr();
    if (operands[1]->getType() == TypeSystem::intType || operands[1]->getType() == TypeSystem::constIntType) {
        switch (opcode)
        {
        case E:
            op = "eq";
            break;
        case NE:
            op = "ne";
            break;
        case L:
            op = "slt";
            break;
        case LE:
            op = "sle";
            break;
        case G:
            op = "sgt";
            break;
        case GE:
            op = "sge";
            break;
        default:
            op = "";
            break;
        }
        fprintf(yyout, "  %s = icmp %s %s %s, %s\n", s1.c_str(), op.c_str(), type.c_str(), s2.c_str(), s3.c_str());
    }
    else{
        assert(operands[1]->getType() == TypeSystem::floatType || operands[1]->getType() == TypeSystem::constFloatType);
        switch (opcode)
        {
        case E:
            op = "oeq";
            break;
        case NE:
            op = "one";
            break;
        case L:
            op = "olt";
            break;
        case LE:
            op = "ole";
            break;
        case G:
            op = "ogt";
            break;
        case GE:
            op = "oge";
            break;
        default:
            op = "";
            break;
        }        
        fprintf(yyout, "  %s = fcmp %s %s %s, %s\n", s1.c_str(), op.c_str(), type.c_str(), s2.c_str(), s3.c_str());
    }
    
}
void CmpInstruction::genMachineCode(AsmBuilder* builder) {
    auto cur_block = builder->getBlock();
    bool reverse = false;
    auto src1 = genMachineOperand(operands[1]);
    auto src2 = genMachineOperand(operands[2]);
    if (src1->isImm())
    {
        // 如果src1是立即数，src2不是，把他俩交换一下，然后判断码取反
        if (!floatVersion && AsmBuilder::isLegalImm(src1->getVal()) && !src2->isImm())
        {
            std::swap(src1, src2);
            reverse = true;
        }
        else
        {
            if (floatVersion)
                src1 = new MachineOperand(*fimmToVReg(src1, cur_block, builder));
            else
                src1 = new MachineOperand(*immToVReg(src1, cur_block));
        }
    }
    if (src2->isImm())
    {
        if (!floatVersion && AsmBuilder::isLegalImm(src2->getVal()))
            goto SKIP;
        if (floatVersion)
            src2 = new MachineOperand(*fimmToVReg(src2, cur_block, builder));
        else
            src2 = new MachineOperand(*immToVReg(src2, cur_block));
    }
SKIP:
    if (floatVersion)
    {
        cur_block->InsertInst(new CmpMInstruction(cur_block, CmpMInstruction::VCMP, src1, src2));
        cur_block->InsertInst(new VmrsMInstruction(cur_block));
    }
    else
    {
        cur_block->InsertInst(new CmpMInstruction(cur_block, CmpMInstruction::CMP, src1, src2));
    }
    int cmpOpCode = 0, minusOpCode = 0;
    switch (opcode)
    {
    case E:
    {
        cmpOpCode = CmpMInstruction::EQ;
        minusOpCode = CmpMInstruction::NE;
    }
    break;
    case NE:
    {
        cmpOpCode = CmpMInstruction::NE;
        minusOpCode = CmpMInstruction::EQ;
    }
    break;
    case L:
    {
        cmpOpCode = reverse ? CmpMInstruction::GT : CmpMInstruction::LT;
        minusOpCode = reverse ? CmpMInstruction::LE : CmpMInstruction::GE;
    }
    break;
    case LE:
    {
        cmpOpCode = reverse ? CmpMInstruction::GE : CmpMInstruction::LE;
        minusOpCode = reverse ? CmpMInstruction::LT : CmpMInstruction::GT;
    }
    break;
    case G:
    {
        cmpOpCode = reverse ? CmpMInstruction::LT : CmpMInstruction::GT;
        minusOpCode = reverse ? CmpMInstruction::GE : CmpMInstruction::LE;
    }
    break;
    case GE:
    {
        cmpOpCode = reverse ? CmpMInstruction::LE : CmpMInstruction::GE;
        minusOpCode = reverse ? CmpMInstruction::GT : CmpMInstruction::LT;
    }
    break;
    default:
        break;
    }
    auto dst = genMachineOperand(operands[0]);
    cur_block->InsertInst(new MovMInstruction(cur_block, MovMInstruction::MOV, dst, genMachineImm(1), cmpOpCode));
    cur_block->InsertInst(new MovMInstruction(cur_block, MovMInstruction::MOV, dst, genMachineImm(0), minusOpCode));
    builder->setCmpOpcode(cmpOpCode);
}

UncondBrInstruction::UncondBrInstruction(BasicBlock *to, BasicBlock *insert_bb) : Instruction(UNCOND, insert_bb)
{
    branch = to;
}

void UncondBrInstruction::output() const
{
    fprintf(yyout, "  br label %%B%d\n", branch->getNo());
}

void UncondBrInstruction::setBranch(BasicBlock *bb)
{
    branch = bb;
}

BasicBlock *UncondBrInstruction::getBranch()
{
    return branch;
}
void UncondBrInstruction::genMachineCode(AsmBuilder* builder) {
    auto cur_block = builder->getBlock();
    cur_block->InsertInst(new BranchMInstruction(cur_block, BranchMInstruction::B, genMachineLabel(branch->getNo())));
}

CondBrInstruction::CondBrInstruction(BasicBlock*true_branch, BasicBlock*false_branch, Operand *cond, BasicBlock *insert_bb) : Instruction(COND, insert_bb){
    this->true_branch = true_branch;
    this->false_branch = false_branch;
    cond->addUse(this);
    operands.push_back(cond);
}

CondBrInstruction::~CondBrInstruction()
{
    operands[0]->removeUse(this);
}

void CondBrInstruction::output() const
{
    std::string cond, type;
    cond = operands[0]->toStr();
    type = operands[0]->getType()->toStr();
    int true_label = true_branch->getNo();
    int false_label = false_branch->getNo();
    fprintf(yyout, "  br %s %s, label %%B%d, label %%B%d\n", type.c_str(), cond.c_str(), true_label, false_label);
}

void CondBrInstruction::setFalseBranch(BasicBlock *bb)
{
    false_branch = bb;
}

BasicBlock *CondBrInstruction::getFalseBranch()
{
    return false_branch;
}

void CondBrInstruction::setTrueBranch(BasicBlock *bb)
{
    true_branch = bb;
}

BasicBlock *CondBrInstruction::getTrueBranch()
{
    return true_branch;
}
void CondBrInstruction::genMachineCode(AsmBuilder* builder) {
    auto cur_block = builder->getBlock();
    cur_block->InsertInst(new BranchMInstruction(cur_block, BranchMInstruction::B, genMachineLabel(true_branch->getNo()), builder->getCmpOpcode()));
    cur_block->InsertInst(new BranchMInstruction(cur_block, BranchMInstruction::B, genMachineLabel(false_branch->getNo())));
}

RetInstruction::RetInstruction(Operand *src, BasicBlock *insert_bb) : Instruction(RET, insert_bb)
{
    if(src != nullptr)
    {
        operands.push_back(src);
        src->addUse(this);
    }
}

RetInstruction::~RetInstruction()
{
    if(!operands.empty())
        operands[0]->removeUse(this);
}

void RetInstruction::output() const
{
    if(operands.empty())
    {
        fprintf(yyout, "  ret void\n");
    }
    else
    {
        std::string ret, type;
        ret = operands[0]->toStr();
        type = operands[0]->getType()->toStr();
        fprintf(yyout, "  ret %s %s\n", type.c_str(), ret.c_str());
    }
}
void RetInstruction::genMachineCode(AsmBuilder* builder) {
    auto cur_bb = builder->getBlock();
    // 如果有返回值
    if (operands.size() > 0)
    {
        //printf("here is param~~~~~\n");
        auto ret_value = genMachineOperand(operands[0]);
        if (ret_value->isImm())
            ret_value = new MachineOperand(*immToVReg(ret_value, cur_bb));
        if (operands[0]->getType()->isFloat())
        {
            if (ret_value->isFReg())
                cur_bb->InsertInst(new MovMInstruction(cur_bb, MovMInstruction::VMOV32, genMachineReg(0, true), ret_value));
            else // 同样的，这种情况是返回立即数，把立即数放到r寄存器里了
                cur_bb->InsertInst(new MovMInstruction(cur_bb, MovMInstruction::VMOV, genMachineReg(0, true), ret_value));
        }
        else{
            cur_bb->InsertInst(new MovMInstruction(cur_bb, MovMInstruction::MOV, genMachineReg(0), ret_value));
        }
    }
    auto sp = genMachineReg(13);
    // 释放栈空间，这里直接来一条mov就行了
    auto cur_inst = new MovMInstruction(cur_bb, MovMInstruction::MOV, sp, genMachineReg(11));
    cur_bb->InsertInst(cur_inst);
    // 恢复保存的寄存器
    auto curr_inst = new StackMInstruction(cur_bb, StackMInstruction::VPOP, {});
    cur_bb->InsertInst(curr_inst);
    cur_bb->getParent()->addUInst(curr_inst);
    curr_inst = new StackMInstruction(cur_bb, StackMInstruction::POP, {});
    cur_bb->InsertInst(curr_inst);
    cur_bb->getParent()->addUInst(curr_inst);
    // bx指令
    cur_bb->InsertInst(new BranchMInstruction(cur_bb, BranchMInstruction::BX, genMachineReg(14)));
}

AllocaInstruction::AllocaInstruction(Operand *dst, SymbolEntry *se, BasicBlock *insert_bb) : Instruction(ALLOCA, insert_bb)
{
    operands.push_back(dst);
    dst->setDef(this);
    this->se = se;
}

AllocaInstruction::~AllocaInstruction()
{
    operands[0]->setDef(nullptr);
    if (operands[0]->usersNum() == 0)
    {
        delete operands[0];
        operands[0] = nullptr;
    }
}

void AllocaInstruction::output() const
{
    std::string dst, type;
    dst = operands[0]->toStr();
    type = se->getType()->toStr();
    fprintf(yyout, "  %s = alloca %s, align 4\n", dst.c_str(), type.c_str());
}
void AllocaInstruction::genMachineCode(AsmBuilder* builder) {
    auto cur_func = builder->getFunction();
    int offset = cur_func->AllocSpace(se->getType()->getSize() / TypeSystem::intType->getSize() * 4);
    dynamic_cast<TemporarySymbolEntry *>(operands[0]->getSymbolEntry())->setOffset(-offset);
}

LoadInstruction::LoadInstruction(Operand *dst, Operand *src_addr, BasicBlock *insert_bb) : Instruction(LOAD, insert_bb)
{
    operands.push_back(dst);
    operands.push_back(src_addr);
    dst->setDef(this);
    src_addr->addUse(this);
}

LoadInstruction::~LoadInstruction()
{
    operands[0]->setDef(nullptr);
    if (operands[0]->usersNum() == 0)
    {
        delete operands[0];
        operands[0] = nullptr;
    }
    operands[1]->removeUse(this);
}

void LoadInstruction::output() const
{
    std::string dst = operands[0]->toStr();
    std::string src = operands[1]->toStr();
    std::string src_type;
    std::string dst_type;
    dst_type = operands[0]->getType()->toStr();
    src_type = operands[1]->getType()->toStr();
    fprintf(yyout, "  %s = load %s, %s %s, align 4\n", dst.c_str(), dst_type.c_str(), src_type.c_str(), src.c_str());
}
void LoadInstruction::genMachineCode(AsmBuilder* builder) {
    auto cur_block = builder->getBlock(); //获取当前基本块
    MachineInstruction *cur_inst = nullptr;
    bool floatVersion = operands[0]->getType()->isFloat();
    int ldrOp = floatVersion ? LoadMInstruction::VLDR : LoadMInstruction::LDR; //区分浮点数与整数
    // 如果操作数是全局变量
    if (operands[1]->getSymbolEntry()->isVariable() && dynamic_cast<IdentifierSymbolEntry *>(operands[1]->getSymbolEntry())->isGlobal())
    {
        auto dst = genMachineOperand(operands[0]); //生成目标寄存器的操作数
        auto internal_reg1 = genMachineVReg();
        auto internal_reg2 = new MachineOperand(*internal_reg1);
        auto src = genMachineOperand(operands[1]);
        // 例如: load r0, addr_a
        cur_inst = new LoadMInstruction(cur_block, LoadMInstruction::LDR, internal_reg1, src);
        cur_block->InsertInst(cur_inst);
        // 例如: load r1, [r0]
        cur_inst = new LoadMInstruction(cur_block, ldrOp, dst, internal_reg2);
        cur_block->InsertInst(cur_inst);
    }
    // 局部变量
    else if (operands[1]->getSymbolEntry()->isTemporary() && operands[1]->getDef() && operands[1]->getDef()->isAlloca())
    {
        // example: load r1, [r0, #4]
        auto dst = genMachineOperand(operands[0]);
        auto src1 = genMachineReg(11);
        int offset = dynamic_cast<TemporarySymbolEntry *>(operands[1]->getSymbolEntry())->getOffset();
        cur_block->InsertInst(new LoadMInstruction(cur_block, ldrOp, dst, src1, genMachineImm(offset)));
    }
    // 临时变量
    else
    {
        // example: load r1, [r0]
        auto dst = genMachineOperand(operands[0]);
        auto src = genMachineOperand(operands[1]);
        cur_inst = new LoadMInstruction(cur_block, ldrOp, dst, src);
        cur_block->InsertInst(cur_inst);
    }
}

GepInstruction::GepInstruction(Operand *dst, Operand *base, std::vector<Operand *> offs, BasicBlock *insert_bb, bool type2) : Instruction(GEP, insert_bb), type2(type2)
{
    operands.push_back(dst);
    operands.push_back(base);
    dst->setDef(this);
    base->addUse(this);
    for (auto off : offs)
    {
        operands.push_back(off);
        off->addUse(this);
    }
}

void GepInstruction::output() const
{
    Operand *dst = operands[0];
    Operand *base = operands[1];
    std::string arrType = base->getType()->toStr();
    if (!type2)
    {
        fprintf(yyout, "  %s = getelementptr inbounds %s, %s %s, i32 0",
                dst->toStr().c_str(), arrType.substr(0, arrType.size() - 1).c_str(),
                arrType.c_str(), base->toStr().c_str());
    }
    else
    {
        fprintf(yyout, "  %s = getelementptr inbounds %s, %s %s",
                dst->toStr().c_str(), arrType.substr(0, arrType.size() - 1).c_str(),
                arrType.c_str(), base->toStr().c_str());
    }
    for (unsigned long int i = 2; i < operands.size(); i++)
    {
        fprintf(yyout, ", i32 %s", operands[i]->toStr().c_str());
    }
    fprintf(yyout, "\n");
}
void GepInstruction::genMachineCode(AsmBuilder* builder) {
    // type2表示是不是通过传参传过来的数组指针，为true表示是，否则表示局部变量或者全局变量
    auto cur_block = builder->getBlock();
    auto dst = genMachineOperand(operands[0]);
    // 这里就是对于局部变量或者全局变量，要先把它们地址放到一个临时寄存器里，
    // 而函数参数，其实operand[1]就存的有地址
    auto base = type2 ? genMachineOperand(operands[1]) : genMachineVReg();
    // 全局变量，先load
    if (operands[1]->getSymbolEntry()->isVariable() && dynamic_cast<IdentifierSymbolEntry *>(operands[1]->getSymbolEntry())->isGlobal())
    {
        auto src = genMachineOperand(operands[1]);
        cur_block->InsertInst(new LoadMInstruction(cur_block, LoadMInstruction::LDR, base, src));
        base = new MachineOperand(*base);
    }
    else if (!type2) // 局部变量
    {
        // 偏移都是负数
        int offset = ((TemporarySymbolEntry *)operands[1]->getSymbolEntry())->getOffset();
        auto off = genMachineImm(offset);
        if (AsmBuilder::isLegalImm(offset))
            cur_block->InsertInst(new BinaryMInstruction(cur_block, BinaryMInstruction::ADD, base, genMachineReg(11), off));
        else
            cur_block->InsertInst(new BinaryMInstruction(cur_block, BinaryMInstruction::ADD, base, genMachineReg(11), new MachineOperand(*immToVReg(off, cur_block))));
        base = new MachineOperand(*base);
    }
    Type *arrType = ((PointerType *)operands[1]->getType())->getValType();
    std::vector<int> indexs = {};
    if (arrType->isArray())
        indexs = ((ArrayType *)arrType)->getIndexs();
    std::vector<int> imms; // 这个专门用来记录索引中的立即数比如说a[10][i][3] 就存一个{0, 2}这样子
    for (unsigned long int i = 2; i < operands.size(); i++)
    {
        if (operands[i]->getSymbolEntry()->isConstant())
        {
            imms.push_back(i);
            continue;
        }
        unsigned int step = 4;
        for (unsigned long int j = i - (type2 ? 2 : 1); j < indexs.size(); j++)
        {
            step *= indexs[j];
        }
        if (AsmBuilder::isPowNumber(step) != -1)
        {
            auto internal_reg1 = genMachineVReg();
            auto src1 = genMachineOperand(operands[i]);
            cur_block->InsertInst(new BinaryMInstruction(cur_block, BinaryMInstruction::LSL, internal_reg1, src1, genMachineImm(AsmBuilder::isPowNumber(step))));
            auto internal_reg2 = genMachineVReg();
            cur_block->InsertInst(new BinaryMInstruction(cur_block, BinaryMInstruction::ADD, internal_reg2, new MachineOperand(*base), new MachineOperand(*internal_reg1)));
            base = new MachineOperand(*internal_reg2);
        }
        else
        {
            auto off = genMachineVReg();
            if (AsmBuilder::isLegalImm(step))
                cur_block->InsertInst(new MovMInstruction(cur_block, MovMInstruction::MOV, off, genMachineImm(step)));
            else
                cur_block->InsertInst(new LoadMInstruction(cur_block, LoadMInstruction::LDR, off, genMachineImm(step)));
            auto internal_reg1 = genMachineVReg();
            auto src1 = genMachineOperand(operands[i]);
            cur_block->InsertInst(new BinaryMInstruction(cur_block, BinaryMInstruction::MUL, internal_reg1, src1, new MachineOperand(*off)));
            auto internal_reg2 = genMachineVReg();
            cur_block->InsertInst(new BinaryMInstruction(cur_block, BinaryMInstruction::ADD, internal_reg2, new MachineOperand(*base), new MachineOperand(*internal_reg1)));
            base = new MachineOperand(*internal_reg2);
        }
    }
    int off = 0;
    for (auto index : imms)
    {
        int imm = ((ConstantSymbolEntry *)operands[index]->getSymbolEntry())->getValue();
        unsigned int step = 4;
        for (unsigned long int j = index - (type2 ? 2 : 1); j < indexs.size(); j++)
        {
            step *= indexs[j];
        }
        off += (imm * step);
    }
    if (off > 0)
    {
        auto internal_reg1 = genMachineImm(off);
        if (!AsmBuilder::isLegalImm(off))
        {
            internal_reg1 = new MachineOperand(*immToVReg(internal_reg1, cur_block));
        }
        auto new_base = genMachineVReg();
        cur_block->InsertInst(new BinaryMInstruction(cur_block, BinaryMInstruction::ADD, new_base, new MachineOperand(*base), new MachineOperand(*internal_reg1)));
        base = new MachineOperand(*new_base);
    }
    cur_block->InsertInst(new MovMInstruction(cur_block, MovMInstruction::MOV, dst, base));

}

StoreInstruction::StoreInstruction(Operand *dst_addr, Operand *src, BasicBlock *insert_bb) : Instruction(STORE, insert_bb)
{
    operands.push_back(dst_addr);
    operands.push_back(src);
    dst_addr->addUse(this);
    src->addUse(this);
}

StoreInstruction::~StoreInstruction()
{
    operands[0]->removeUse(this);
    operands[1]->removeUse(this);
}

void StoreInstruction::output() const
{
    std::string dst = operands[0]->toStr();
    std::string src = operands[1]->toStr();
    std::string dst_type = operands[0]->getType()->toStr();
    std::string src_type = operands[1]->getType()->toStr();

    fprintf(yyout, "  store %s %s, %s %s, align 4\n", src_type.c_str(), src.c_str(), dst_type.c_str(), dst.c_str());
}

void StoreInstruction::genMachineCode(AsmBuilder* builder) {
    // TODO
    auto cur_block = builder->getBlock();
    MachineInstruction *cur_inst = nullptr;
    auto src = genMachineOperand(operands[1]);
    bool floatVersion = operands[1]->getType()->isFloat();
    int strOp = (floatVersion && !src->isImm()) ? StoreMInstruction::VSTR : StoreMInstruction::STR;
    if (src->isImm()) // 这里立即数可能为浮点数，这样做也没问题
    {
        src = new MachineOperand(*immToVReg(src, cur_block));
    }
    if (operands[0]->getSymbolEntry()->isVariable() && dynamic_cast<IdentifierSymbolEntry *>(operands[0]->getSymbolEntry())->isGlobal())
    {
        auto dst = genMachineOperand(operands[0]);
        auto internal_reg1 = genMachineVReg();
        auto internal_reg2 = new MachineOperand(*internal_reg1);
        cur_inst = new LoadMInstruction(cur_block, LoadMInstruction::LDR, internal_reg1, dst);
        cur_block->InsertInst(cur_inst);
        cur_inst = new StoreMInstruction(cur_block, strOp, src, internal_reg2);
        cur_block->InsertInst(cur_inst);
    }
    else if (operands[0]->getSymbolEntry()->isTemporary() && operands[0]->getDef() && operands[0]->getDef()->isAlloca())
    {
        auto dst = genMachineReg(11);
        int offset = dynamic_cast<TemporarySymbolEntry *>(operands[0]->getSymbolEntry())->getOffset();
        cur_inst = new StoreMInstruction(cur_block, strOp, src, dst, genMachineImm(offset));
        cur_block->InsertInst(cur_inst);
    }
    else
    {
        auto dst = genMachineOperand(operands[0]);
        cur_inst = new StoreMInstruction(cur_block, strOp, src, dst);
        cur_block->InsertInst(cur_inst);
    }
}


IntFloatCastInstruction::IntFloatCastInstruction(unsigned opcode, Operand *dst, Operand *src, BasicBlock *insert_bb) : Instruction(IntFloatCast, insert_bb)
{
    this->opcode = opcode;
    operands.push_back(dst);
    operands.push_back(src);
    dst->setDef(this);
    src->addUse(this);
}

void IntFloatCastInstruction::output() const
{
    std::string dst = operands[0]->toStr();
    std::string src = operands[1]->toStr();
    std::string dst_type = operands[0]->getType()->toStr();
    std::string src_type = operands[1]->getType()->toStr();
    std::string op;
    switch (opcode)
    {
    case S2F:
        op = "sitofp";
        break;
    case F2S:
        op = "fptosi";
        break;
    default:
        op = "";
        break;
    }
    fprintf(yyout, "  %s = %s %s %s to %s\n", dst.c_str(), op.c_str(), src_type.c_str(), src.c_str(), dst_type.c_str());
}

void IntFloatCastInstruction::genMachineCode(AsmBuilder* builder) {
    if(opcode == S2F) {
        auto cur_block = builder->getBlock();
        auto dst = genMachineOperand(operands[0]);
        auto src = genMachineOperand(operands[1]);
        if (src->isImm())
        {
            src = new MachineOperand(*immToVReg(src, cur_block));
        }
        assert(dst->isFReg());
        cur_block->InsertInst(new MovMInstruction(cur_block, MovMInstruction::VMOV, dst, src));
        cur_block->InsertInst(new VcvtMInstruction(cur_block, VcvtMInstruction::STF, new MachineOperand(*dst), new MachineOperand(*dst)));
    }
    else if (opcode == F2S) {
        auto cur_block = builder->getBlock();
        auto dst = genMachineOperand(operands[0]);
        auto src = genMachineOperand(operands[1]);
        if (src->isImm())
        {
            src = new MachineOperand(*immToVReg(src, cur_block));
        }
        if (src->isFReg())
        { 
            auto internal_reg = genMachineVReg(true);
            cur_block->InsertInst(new VcvtMInstruction(cur_block, VcvtMInstruction::FTS, internal_reg, src));
            cur_block->InsertInst(new MovMInstruction(cur_block, MovMInstruction::VMOV, dst, new MachineOperand(*internal_reg)));
        }
        else
        {
            auto internal_reg = genMachineVReg(true);
            cur_block->InsertInst(new MovMInstruction(cur_block, MovMInstruction::VMOV, internal_reg, src));
            cur_block->InsertInst(new VcvtMInstruction(cur_block, VcvtMInstruction::FTS, new MachineOperand(*internal_reg), new MachineOperand(*internal_reg)));
            cur_block->InsertInst(new MovMInstruction(cur_block, MovMInstruction::VMOV, dst, new MachineOperand(*internal_reg)));
        }
    }
}


// 零拓展
ZextInstruction::ZextInstruction(Operand *dst, Operand *src, BasicBlock *insertBB) : Instruction(TEMP, insertBB)
{
    dst->setDef(this);
    src->addUse(this);
    operands.push_back(dst);
    operands.push_back(src);
}

void ZextInstruction::output() const
{
    fprintf(yyout, "  %s = zext i1 %s to i32\n", operands[0]->toStr().c_str(), operands[1]->toStr().c_str());
}
void ZextInstruction::genMachineCode(AsmBuilder* builder) {
    auto cur_block = builder->getBlock();
    auto dst = genMachineOperand(operands[0]);
    auto src = genMachineOperand(operands[1]);
    cur_block->InsertInst(new MovMInstruction(cur_block, MovMInstruction::MOV, dst, src));

}

NotInstruction::NotInstruction(Operand *dst, Operand *src, BasicBlock *insert_bb) : Instruction(NOT, insert_bb)
{
    operands.push_back(dst);
    operands.push_back(src);
    dst->setDef(this);
    src->addUse(this);

    dst->getSymbolEntry()->setType(TypeSystem::boolType);
}

NotInstruction::~NotInstruction()
{
    operands[0]->setDef(nullptr);
    if (operands[0]->usersNum() == 0)
        delete operands[0];
    operands[1]->removeUse(this);
}

void NotInstruction::output() const

{
    std::string s1, s2, type;
    s1 = operands[0]->toStr();
    s2 = operands[1]->toStr();
    type = operands[0]->getType()->toStr();

    fprintf(yyout, "  %s = xor %s %s, true\n", s1.c_str(), type.c_str(), s2.c_str());
}

void NotInstruction::genMachineCode(AsmBuilder* builder) {
    auto cur_block = builder->getBlock();
    auto dst = genMachineOperand(operands[0]);
    auto src = genMachineOperand(operands[1]);
    cur_block->InsertInst(new CmpMInstruction(cur_block, CmpMInstruction::CMP, src, genMachineImm(0)));
    cur_block->InsertInst(new MovMInstruction(cur_block, MovMInstruction::MOV, dst, genMachineImm(1), CmpMInstruction::EQ));
    cur_block->InsertInst(new MovMInstruction(cur_block, MovMInstruction::MOV, dst, genMachineImm(0), CmpMInstruction::NE));
    builder->setCmpOpcode(CmpMInstruction::EQ);
}

GlobalInstruction::GlobalInstruction(Operand *dst, Operand *expr, SymbolEntry *se, BasicBlock *insertBB) : Instruction(GLOBAL, insertBB)
{
    operands.push_back(dst);
    operands.push_back(expr);
    this->se = se;
    dst->setDef(this);
}

void GlobalInstruction::output() const
{
    SymbolEntry* op0_se = operands[0]->getSymbolEntry();
    bool is_arr = op0_se->getType()->isArray();
    if (operands[1] == nullptr)
    {
        if(is_arr){
            fprintf(yyout, "%s = global %s zeroinitializer, align 4 \n", operands[0]->toStr().c_str(), operands[0]->getType()->toStr().c_str());
        }
        else if(strcmp(operands[0]->getType()->toStr().c_str(), "float") == 0){
            fprintf(yyout, "%s = global %s 0.0, align 4 \n", operands[0]->toStr().c_str(), operands[0]->getType()->toStr().c_str());
        }
        else{
            fprintf(yyout, "%s = global %s 0, align 4 \n", operands[0]->toStr().c_str(), operands[0]->getType()->toStr().c_str());
        }
    }
    else
    {
        if(strcmp(operands[0]->getType()->toStr().c_str(), "float") == 0){
            if(strstr(operands[1]->toStr().c_str(), "0x") == nullptr){
                fprintf(yyout, "%s = global %s %s.0, align 4 \n", operands[0]->toStr().c_str(), operands[0]->getType()->toStr().c_str(), operands[1]->toStr().c_str());
            }
            else{
                fprintf(yyout, "%s = global %s %s, align 4 \n", operands[0]->toStr().c_str(), operands[0]->getType()->toStr().c_str(), operands[1]->toStr().c_str());
            }
        }
        else{
            fprintf(yyout, "%s = global %s %s, align 4 \n", operands[0]->toStr().c_str(), operands[0]->getType()->toStr().c_str(), operands[1]->toStr().c_str());
        }
    }
}

void GlobalInstruction::genMachineCode(AsmBuilder* builder) {
}

CallInstruction::CallInstruction(Operand *dst,
                                 SymbolEntry *func,
                                 std::vector<Operand *> params,
                                 BasicBlock *insert_bb)
    : Instruction(CALL, insert_bb), func(func)
{
    operands.push_back(dst);
    if (dst)
        dst->setDef(this);
    for (auto param : params)
    {
        operands.push_back(param);
        param->addUse(this);
    }
    this->funcAddPred();
}

void CallInstruction::output() const
{
    fprintf(yyout, "  ");
    if (operands[0])
        fprintf(yyout, "%s = ", operands[0]->toStr().c_str());
    std::string fullTypeStr = ((FunctionType *)(func->getType()))->toStr();
    size_t pos = fullTypeStr.find('(');
    std::string returnTypeStr;
    if (pos != std::string::npos) {
        returnTypeStr = fullTypeStr.substr(0, pos);
    }
    fprintf(yyout, "call %s %s(", returnTypeStr.c_str(),
            func->toStr().c_str());
    for (long unsigned int i = 1; i < operands.size(); i++)
    {
        if (i != 1)
            fprintf(yyout, ", ");
        fprintf(yyout, "%s %s", operands[i]->getType()->toStr().c_str(),operands[i]->toStr().c_str());
    }
    fprintf(yyout, ")\n");
}

void CallInstruction::funcAddPred() {
        IdentifierSymbolEntry *funcSE = (IdentifierSymbolEntry *)func;
        if (!funcSE->isSysy() && funcSE->getName() != "llvm.memset.p0i8.i32")
        {
            funcSE->getFunction()->addCallPred(this);
        }
    }

void CallInstruction::genMachineCode(AsmBuilder* builder) {
    auto cur_block = builder->getBlock();

    // 先把不是浮点数的放到r0-r3里
    size_t i;
    int sum = 0;
    for (i = 1; i <= operands.size() - 1 && sum < 4; i++)
    {
        if (operands[i]->getType()->isFloat())
            continue;
        auto param = genMachineOperand(operands[i]);
        if (param->isImm())
        {
            if (!AsmBuilder::isLegalImm(param->getVal()))
                cur_block->InsertInst(new LoadMInstruction(cur_block, LoadMInstruction::LDR, genMachineReg(sum), param));
            else
                cur_block->InsertInst(new MovMInstruction(cur_block, MovMInstruction::MOV, genMachineReg(sum), param));
        }
        else
            cur_block->InsertInst(new MovMInstruction(cur_block, MovMInstruction::MOV, genMachineReg(sum), param));
        sum++;
    }
    auto intLastPos = i;
    // 把浮点数放到寄存器里
    sum = 0;
    for (i = 1; i <= operands.size() - 1 && sum < 16; i++)
    {
        if (!operands[i]->getType()->isFloat())
            continue;
        auto param = genMachineOperand(operands[i]);
        if (param->isImm())
        {
            param = new MachineOperand(*immToVReg(param, cur_block));
            cur_block->InsertInst(new MovMInstruction(cur_block, MovMInstruction::VMOV, genMachineReg(sum, true), param));
        }
        else
        {
            // 用mov指令把参数放到对应寄存器里
            cur_block->InsertInst(new MovMInstruction(cur_block, MovMInstruction::VMOV32, genMachineReg(sum, true), param));
        }
        sum++;
    }
    auto floatLastPos = i;
    int param_size_in_stack = 0;
    for (long unsigned int i = operands.size() - 1; i >= 1; i--)
    {
        if (operands[i]->getType()->isFloat() && i < floatLastPos)
            continue;
        if (!operands[i]->getType()->isFloat() && i < intLastPos)
            continue;
        auto param = genMachineOperand(operands[i]);
        if (param->isFReg())
        {
            cur_block->InsertInst(new StackMInstruction(cur_block, StackMInstruction::VPUSH, {param}));
        }
        else
        {
            if (param->isImm())
                param = new MachineOperand(*immToVReg(param, cur_block));
            cur_block->InsertInst(new StackMInstruction(cur_block, StackMInstruction::PUSH, {param}));
        }
        param_size_in_stack += 4;
    }
    // 生成bl指令，调用函数
    auto blInst = new BranchMInstruction(cur_block, BranchMInstruction::BL, new MachineOperand(func->toStr().c_str()));
    cur_block->InsertInst(blInst);
    // 生成add指令释放栈空间
    if (param_size_in_stack > 0)
    {
        auto sp = genMachineReg(13);
        auto stack_size = genMachineImm(param_size_in_stack);
        if (AsmBuilder::isLegalImm(param_size_in_stack))
            cur_block->InsertInst(new BinaryMInstruction(cur_block, BinaryMInstruction::ADD, sp, new MachineOperand(*sp), stack_size));
        else
            cur_block->InsertInst(new BinaryMInstruction(cur_block, BinaryMInstruction::ADD, sp, new MachineOperand(*sp), new MachineOperand(*immToVReg(stack_size, cur_block))));
    }
    if (operands[0])
    {
        if (operands[0]->getType()->isFloat())
            cur_block->InsertInst(new MovMInstruction(cur_block, MovMInstruction::VMOV32, genMachineOperand(operands[0]), genMachineReg(0, true)));
        else
            cur_block->InsertInst(new MovMInstruction(cur_block, MovMInstruction::MOV, genMachineOperand(operands[0]), genMachineReg(0)));
    }
}

PhiInstruction::PhiInstruction(Operand *dst, BasicBlock *insert_bb) : Instruction(PHI, insert_bb)
{
    operands.push_back(dst);
    addr = dst;
}

PhiInstruction::~PhiInstruction()
{
}

void PhiInstruction::output() const
{
    fprintf(yyout, "  %s = phi %s ", operands[0]->toStr().c_str(), operands[0]->getType()->toStr().c_str());
    if (srcs.empty())
    {
        fprintf(stderr, "\n");
        return;
    }
    auto it = srcs.begin();
    fprintf(yyout, "[ %s , %%B%d ]", it->second->toStr().c_str(), it->first->getNo());
    it++;
    for (; it != srcs.end(); it++)
    {
        fprintf(yyout, ", ");
        fprintf(yyout, "[ %s , %%B%d ]", it->second->toStr().c_str(), it->first->getNo());
    }
    fprintf(yyout, "\n");
}
void PhiInstruction::genMachineCode(AsmBuilder* builder) {
    //do nothing
    printf("phi not here~~~~~~~\n");
}

void PhiInstruction::addEdge(BasicBlock *block, Operand *src)
{
    operands.push_back(src);
    srcs[block] = src;
    src->addUse(this);
}

bool PhiInstruction::findSrc(BasicBlock *block)
{
    for (auto it = srcs.begin(); it != srcs.end(); it++)
    {
        if (it->first == block)
        {
            return true;
        }
    }
    return false;
}

Operand *PhiInstruction::getBlockSrc(BasicBlock *block)
{
    if (srcs.find(block) != srcs.end())
        return srcs[block];
    return nullptr;
}

void PhiInstruction::removeBlockSrc(BasicBlock *block)
{
    for (auto it = srcs.begin(); it != srcs.end(); it++)
    {
        if (it->first == block)
        {
            // 使用erase时容器失效
            srcs.erase(block);
            removeUse(it->second);
            it->second->removeUse(this);
            return;
        }
    }
    return;
}

void PhiInstruction::addSrc(BasicBlock *block, Operand *src)
{
    operands.push_back(src);
    srcs.insert(std::make_pair(block, src));
    src->addUse(this);
}

void PhiInstruction::removeUse(Operand *use)
{
    auto it = find(operands.begin() + 1, operands.end(), use);
    if (it != operands.end())
        operands.erase(it);
}

BitcastInstruction::BitcastInstruction(Operand *dst, Operand *src, BasicBlock *insert_bb) : Instruction(BITCAST, insert_bb), dst(dst), src(src)
{
    operands.push_back(dst);
    operands.push_back(src);
    dst->setDef(this);
    src->addUse(this);
}

BitcastInstruction::~BitcastInstruction()
{
    operands[0]->setDef(nullptr);
    if (operands[0]->usersNum() == 0)
    {
        delete operands[0];
        operands[0] = nullptr;
    }
    operands[1]->removeUse(this);
}

void BitcastInstruction::output() const
{
    std::string dst = operands[0]->toStr();
    std::string src = operands[1]->toStr();
    std::string dst_type = operands[0]->getType()->toStr();
    std::string src_type = operands[1]->getType()->toStr();
    fprintf(yyout, "  %s = bitcast %s %s to %s\n", dst.c_str(), src_type.c_str(), src.c_str(), dst_type.c_str());
}

void BitcastInstruction::genMachineCode(AsmBuilder* builder) {
    auto cur_block = builder->getBlock();
    assert(operands[1]->getSymbolEntry()->isTemporary());
    auto dst = genMachineOperand(operands[0]);
    MachineOperand *src = nullptr;
    int offset = static_cast<TemporarySymbolEntry *>(operands[1]->getSymbolEntry())->getOffset();
    if (offset < 0)
    {
        auto off = genMachineImm(offset);
        auto tmp = new MachineOperand(*immToVReg(off, cur_block));
        src = genMachineVReg();
        cur_block->InsertInst(new BinaryMInstruction(cur_block, BinaryMInstruction::ADD, src, genMachineReg(11), tmp));
        src = new MachineOperand(*src);
    }
    else
        src = genMachineOperand(operands[1]);
    cur_block->InsertInst(new MovMInstruction(cur_block, MovMInstruction::MOV, dst, src));

}