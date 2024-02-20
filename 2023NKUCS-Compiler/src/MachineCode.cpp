#include "MachineCode.h"
#include "assert.h"
#include "AsmBuilder.h"
#include "Type.h"
#include <iostream>
#include <sstream>
#include <cstring>
extern FILE *yyout;

MachineOperand::MachineOperand(int tp, int val, bool fpu)
{
    this->type = tp;
    this->fpu = fpu;
    if (tp == MachineOperand::IMM)
        this->val = val;
    else
        this->reg_no = val;
}

MachineOperand::MachineOperand(std::string label)
{
    this->type = MachineOperand::LABEL;
    this->label = label;
}

bool MachineOperand::operator==(const MachineOperand &a) const
{
    if (this->type != a.type)
        return false;
    if (this->fpu != a.fpu)
        return false;
    if (this->type == IMM)
        return this->val == a.val;
    return this->reg_no == a.reg_no;
}

bool MachineOperand::operator<(const MachineOperand &a) const
{
    if (this->type == a.type)
    {
        if (this->type == IMM)
            return this->val < a.val;
        return this->reg_no < a.reg_no;
    }
    return this->type < a.type;

    if (this->type != a.type)
        return false;
    if (this->type == IMM)
        return this->val == a.val;
    return this->reg_no == a.reg_no;
}

std::string MachineOperand::PrintReg()
{
    std::stringstream ss;
    ss.clear();
    std::string res;
    if (fpu)
    {
        // 浮点寄存器用s
        ss << "s" << reg_no;
        ss >> res;
        return res;
    }
    switch (reg_no)
    {
    case 11:
        res = "fp";
        break;
    case 12:
        res = "ip";
        break;
    case 13:
        res = "sp";
        break;
    case 14:
        res = "lr";
        break;
    case 15:
        res = "pc";
        break;
    default:
        ss << "r" << reg_no;
        ss >> res;
        break;
    }
    return res;
}

std::string MachineOperand::toStr()
{
    std::stringstream ss;
    ss.clear();
    std::string res;
    switch (this->type)
    {
    case IMM:
        ss << "#" << this->val;
        ss >> res;
        break;
    case VREG:
    {
        if (fpu) // 这里如果是浮点虚拟，打印f，好调试
            ss << "f";
        else
            ss << "v";
        ss << this->reg_no;
        ss >> res;
    }
    break;
    case REG:
        res = PrintReg();
        break;
    case LABEL:
        if (this->label.substr(0, 2) == ".L") // 标签
            res = this->label;
        else if (this->label.substr(0, 1) == "@") // 函数
            res = this->label.substr(1, this->label.size() - 1);
        else // 变量
        {
            ss << this->getParent()->getParent()->getParent()->getParent()->getLtorgNum();
            ss >> res;
            res = "addr_" + this->label + "_" + res;
        }
        break;
    default:
        assert(0);
        break;
    }
    return res;
}

void MachineOperand::output()
{
    fprintf(yyout, "%s", this->toStr().c_str());
}

void MachineInstruction::PrintCond()
{
    // TODO
    switch (cond)
    {
    case EQ:
        fprintf(yyout, "eq");
        break;
    case NE:
        fprintf(yyout, "ne");
        break;
    case LT:
        fprintf(yyout, "lt");
        break;
    case LE:
        fprintf(yyout, "le");
        break;
    case GT:
        fprintf(yyout, "gt");
        break;
    case GE:
        fprintf(yyout, "ge");
        break;
    default:
        break;
    }
}

BinaryMInstruction::BinaryMInstruction(
    MachineBlock *p, int op,
    MachineOperand *dst, MachineOperand *src1, MachineOperand *src2,
    int cond)
{
    this->parent = p;
    this->type = MachineInstruction::BINARY;
    this->op = op;
    this->cond = cond;
    this->def_list.push_back(dst);
    this->use_list.push_back(src1);
    this->use_list.push_back(src2);
    dst->setParent(this);
    src1->setParent(this);
    src2->setParent(this);
}

void BinaryMInstruction::output()
{
    // TODO:
    // Complete other instructions
    switch (this->op)
    {
    case BinaryMInstruction::ADD:
        fprintf(yyout, "\tadd");
        break;
    case BinaryMInstruction::SUB:
        fprintf(yyout, "\tsub");
        break;
    case BinaryMInstruction::MUL:
        fprintf(yyout, "\tmul");
        break;
    case BinaryMInstruction::DIV:
        fprintf(yyout, "\tsdiv");
        break;
    case BinaryMInstruction::AND:
        fprintf(yyout, "\tand");
        break;
    case BinaryMInstruction::OR:
        fprintf(yyout, "\torr");
        break;
    case BinaryMInstruction::VADD:
        fprintf(yyout, "\tvadd.f32");
        break;
    case BinaryMInstruction::VSUB:
        fprintf(yyout, "\tvsub.f32");
        break;
    case BinaryMInstruction::VMUL:
        fprintf(yyout, "\tvmul.f32");
        break;
    case BinaryMInstruction::VDIV:
        fprintf(yyout, "\tvdiv.f32");
        break;
    case BinaryMInstruction::LSL:
        fprintf(yyout, "\tlsl");
        break;
    case BinaryMInstruction::LSR:
        fprintf(yyout, "\tlsr");
        break;
    case BinaryMInstruction::ASR:
        fprintf(yyout, "\tasr");
        break;
    default:
        break;
    }
    this->PrintCond();
    fprintf(yyout, " ");
    this->def_list[0]->output();
    fprintf(yyout, ", ");
    this->use_list[0]->output();
    fprintf(yyout, ", ");
    this->use_list[1]->output();
    fprintf(yyout, "\n");
}

std::string BinaryMInstruction::opStr()
{
    switch (this->op)
    {
    case BinaryMInstruction::ADD:
        return "add";
    case BinaryMInstruction::SUB:
        return "sub";
    case BinaryMInstruction::MUL:
        return "mul";
    case BinaryMInstruction::DIV:
        return "sdiv";
    case BinaryMInstruction::AND:
        return "and";
    case BinaryMInstruction::OR:
        return "orr";
    case BinaryMInstruction::VADD:
        return "vadd";
    case BinaryMInstruction::VSUB:
        return "vsub";
    case BinaryMInstruction::VMUL:
        return "vmul";
    case BinaryMInstruction::VDIV:
        return "vdiv";
    case BinaryMInstruction::LSL:
        return "lsl";
    case BinaryMInstruction::LSR:
        return "lsr";
    case BinaryMInstruction::ASR:
        return "asr";
    default:
        break;
    }
}

LoadMInstruction::LoadMInstruction(MachineBlock *p, int op,
                                   MachineOperand *dst, MachineOperand *src1, MachineOperand *src2,
                                   int cond)
{
    this->parent = p;
    this->type = MachineInstruction::LOAD;
    this->op = op;
    this->cond = cond;
    this->def_list.push_back(dst);
    this->use_list.push_back(src1);
    dst->setParent(this);
    src1->setParent(this);
    if (src2)
    {
        this->use_list.push_back(src2);
        src2->setParent(this);
    }
}

void LoadMInstruction::output()
{
    switch (op)
    {
    case LDR:
        fprintf(yyout, "\tldr");
        break;
    case VLDR:
        fprintf(yyout, "\tvldr.32");
        break;
    default:
        break;
    }
    this->PrintCond();
    fprintf(yyout, " ");
    this->def_list[0]->output();
    fprintf(yyout, ", ");

    if (this->use_list[0]->isImm())
    {
        fprintf(yyout, "=%d\n", this->use_list[0]->getVal());
        return;
    }

    if (this->use_list[0]->isReg() || this->use_list[0]->isVReg())
        fprintf(yyout, "[");

    this->use_list[0]->output();
    if (this->use_list.size() > 1)
    {
        fprintf(yyout, ", ");
        this->use_list[1]->output();
    }

    if (this->use_list[0]->isReg() || this->use_list[0]->isVReg())
        fprintf(yyout, "]");
    fprintf(yyout, "\n");
}

StoreMInstruction::StoreMInstruction(MachineBlock *p, int op,
                                     MachineOperand *src, MachineOperand *dst, MachineOperand *off,
                                     int cond)
{
    // TODO
    this->parent = p;
    this->type = MachineInstruction::STORE;
    this->op = op;
    this->cond = cond;
    use_list.push_back(src);
    use_list.push_back(dst);
    src->setParent(this);
    dst->setParent(this);
    if (off != nullptr)
    {
        use_list.push_back(off);
        off->setParent(this);
    }
}

void StoreMInstruction::output()
{
    // TODO
    switch (op)
    {
    case STR:
        fprintf(yyout, "\tstr");
        break;
    case VSTR:
        fprintf(yyout, "\tvstr.32");
        break;
    default:
        break;
    }
    this->PrintCond();
    fprintf(yyout, " ");
    this->use_list[0]->output();
    fprintf(yyout, ", ");
    // Store address
    if (this->use_list[1]->isReg() || this->use_list[1]->isVReg())
        fprintf(yyout, "[");
    this->use_list[1]->output();
    if (this->use_list.size() > 2)
    {
        fprintf(yyout, ", ");
        this->use_list[2]->output();
    }
    if (this->use_list[1]->isReg() || this->use_list[1]->isVReg())
        fprintf(yyout, "]");
    fprintf(yyout, "\n");
}

MovMInstruction::MovMInstruction(MachineBlock *p, int op,
                                 MachineOperand *dst, MachineOperand *src,
                                 int cond)
{
    // TODO
    this->parent = p;
    this->type = MachineInstruction::MOV;
    this->op = op;
    this->cond = cond;
    this->def_list.push_back(dst);
    this->use_list.push_back(src);
    dst->setParent(this);
    src->setParent(this);
}

void MovMInstruction::output()
{
    // TODO
    switch (op)
    {
    case MOV:
        fprintf(yyout, "\tmov");
        break;
    case MVN:
        fprintf(yyout, "\tmvn");
        break;
    case VMOV:
        fprintf(yyout, "\tvmov");
        break;
    case VMOV32:
        fprintf(yyout, "\tvmov.f32");
        break;
    default:
        break;
    }
    PrintCond();
    fprintf(yyout, " ");
    def_list[0]->output();
    fprintf(yyout, ", ");
    if (op == VMOV32 && use_list[0]->isImm())
    {
        unsigned int imm = (unsigned int)use_list[0]->getVal();
        float val = reinterpret_cast<float &>(imm);
        fprintf(yyout, "#%f", val);
    }
    else
        use_list[0]->output();
    fprintf(yyout, "\n");
}

BranchMInstruction::BranchMInstruction(MachineBlock *p, int op,
                                       MachineOperand *dst,
                                       int cond)
{
    // TODO
    this->parent = p;
    this->type = MachineInstruction::BRANCH;
    this->op = op;
    this->cond = cond;
    this->use_list.push_back(dst);
    dst->setParent(this);
}

void BranchMInstruction::output()
{
    // TODO
    switch (op)
    {
    case B:
        fprintf(yyout, "\tb");
        break;
    case BX:
        fprintf(yyout, "\tbx");
        break;
    case BL:
        fprintf(yyout, "\tbl");
        break;
    default:
        break;
    }
    PrintCond();
    fprintf(yyout, " ");
    use_list[0]->output();
    fprintf(yyout, "\n");
}

CmpMInstruction::CmpMInstruction(MachineBlock *p, int op,
                                 MachineOperand *src1, MachineOperand *src2,
                                 int cond)
{
    // TODO
    this->parent = p;
    this->type = MachineInstruction::CMP;
    this->op = op;
    this->cond = cond;
    use_list.push_back(src1);
    use_list.push_back(src2);
    src1->setParent(this);
    src2->setParent(this);
}

void CmpMInstruction::output()
{
    // TODO
    switch (op)
    {
    case CMP:
        fprintf(yyout, "\tcmp");
        break;
    case VCMP:
        fprintf(yyout, "\tvcmp.f32");
        break;
    default:
        break;
    }
    PrintCond();
    fprintf(yyout, " ");
    use_list[0]->output();
    fprintf(yyout, ", ");
    use_list[1]->output();
    fprintf(yyout, "\n");
}

StackMInstruction::StackMInstruction(MachineBlock *p, int op,
                                   std::vector<MachineOperand *> srcs,
                                   int cond)
{
    // TODO
    this->parent = p;
    this->type = MachineInstruction::STACK;
    this->op = op;
    this->cond = cond;
    for (auto operand : srcs)
    {
        use_list.push_back(operand);
        operand->setParent(this);
    }
}

void StackMInstruction::output()
{
    // TODO
    switch (op)
    {
    case POP:
        fprintf(yyout, "\tpop ");
        break;
    case PUSH:
        fprintf(yyout, "\tpush ");
        break;
    case VPOP:
        fprintf(yyout, "\tvpop ");
        break;
    case VPUSH:
        fprintf(yyout, "\tvpush ");
        break;
    default:
        break;
    }
    fprintf(yyout, "{");
    for (long unsigned int i = 0; i < use_list.size(); i++)
    {
        if (i != 0)
        {
            fprintf(yyout, ", ");
        }
        use_list[i]->output();
    }
    fprintf(yyout, "}\n");
}

VcvtMInstruction::VcvtMInstruction(MachineBlock *p, int op, MachineOperand *dst, MachineOperand *src, int cond)
{
    this->parent = p;
    this->type = MachineInstruction::VCVT;
    this->op = op;
    this->cond = cond;
    def_list.push_back(dst);
    use_list.push_back(src);
    dst->setParent(this);
    src->setParent(this);
}

void VcvtMInstruction::output()
{
    switch (op)
    {
    case FTS:
        fprintf(yyout, "\tvcvt.s32.f32 ");
        break;
    case STF:
        fprintf(yyout, "\tvcvt.f32.s32 ");
        break;
    default:
        break;
    }
    def_list[0]->output();
    fprintf(yyout, ", ");
    use_list[0]->output();
    fprintf(yyout, "\n");
}

VmrsMInstruction::VmrsMInstruction(MachineBlock *p, int cond)
{
    this->parent = p;
    this->type = MachineInstruction::VMRS;
    this->op = -1;
    this->cond = cond;
}

void VmrsMInstruction::output()
{
    // 这个指令目前只知道这一个用途，就是跟在vcmp后边传flag
    // 后期可以扩展
    fprintf(yyout, "\tvmrs APSR_nzcv, FPSCR\n");
}

MlaMInstruction::MlaMInstruction(MachineBlock *p, MachineOperand *dst, MachineOperand *src1, MachineOperand *src2, MachineOperand *src3, int cond)
{
    this->parent = p;
    this->type = MachineInstruction::MLA;
    this->op = -1;
    this->cond = cond;
    def_list.push_back(dst);
    use_list.push_back(src1);
    use_list.push_back(src2);
    use_list.push_back(src3);
    dst->setParent(this);
    src1->setParent(this);
    src2->setParent(this);
    src3->setParent(this);
}

void MlaMInstruction::output()
{
    fprintf(yyout, "\tmla ");
    def_list[0]->output();
    fprintf(yyout, ", ");
    use_list[0]->output();
    fprintf(yyout, ", ");
    use_list[1]->output();
    fprintf(yyout, ", ");
    use_list[2]->output();
    fprintf(yyout, "\n");
}

MlsMInstruction::MlsMInstruction(MachineBlock *p, MachineOperand *dst, MachineOperand *src1, MachineOperand *src2, MachineOperand *src3, int cond)
{
    this->parent = p;
    this->type = MachineInstruction::MLS;
    this->op = -1;
    this->cond = cond;
    def_list.push_back(dst);
    use_list.push_back(src1);
    use_list.push_back(src2);
    use_list.push_back(src3);
    dst->setParent(this);
    src1->setParent(this);
    src2->setParent(this);
    src3->setParent(this);
}

void MlsMInstruction::output()
{
    fprintf(yyout, "\tmls ");
    def_list[0]->output();
    fprintf(yyout, ", ");
    use_list[0]->output();
    fprintf(yyout, ", ");
    use_list[1]->output();
    fprintf(yyout, ", ");
    use_list[2]->output();
    fprintf(yyout, "\n");
}

OffsetBinaryMInstruction::OffsetBinaryMInstruction(MachineBlock *p, int op, MachineOperand *dst, MachineOperand *src1, MachineOperand *src2, MachineOperand *src3, int offset, int cond)
{
    this->parent = p;
    this->type = MachineInstruction::OFFBINARY;
    this->op = op;
    this->cond = cond;
    this->offset = offset;
    this->def_list.push_back(dst);
    this->use_list.push_back(src1);
    this->use_list.push_back(src2);
    this->use_list.push_back(src3);
    dst->setParent(this);
    src1->setParent(this);
    src2->setParent(this);
    src3->setParent(this);
}

void OffsetBinaryMInstruction::output()
{
    switch (this->op)
    {
    case OffsetBinaryMInstruction::ADD:
        fprintf(yyout, "\tadd");
        break;
    case OffsetBinaryMInstruction::SUB:
        fprintf(yyout, "\tsub");
        break;
    default:
        break;
    }
    this->PrintCond();
    fprintf(yyout, " ");
    this->def_list[0]->output();
    fprintf(yyout, ", ");
    this->use_list[0]->output();
    fprintf(yyout, ", ");
    this->use_list[1]->output();
    fprintf(yyout, ", ");
    switch (this->offset)
    {
    case OffsetBinaryMInstruction::LSL:
        fprintf(yyout, "lsl");
        break;
    case OffsetBinaryMInstruction::ASR:
        fprintf(yyout, "asr");
        break;
    default:
        break;
    }
    this->use_list[2]->output();
    fprintf(yyout, "\n");
}

VNegMInstruction::VNegMInstruction(MachineBlock *p, MachineOperand *dst, MachineOperand *src, int cond)
{
    this->parent = p;
    this->type = MachineInstruction::VNEG;
    this->op = -1;
    this->cond = cond;
    def_list.push_back(dst);
    use_list.push_back(src);
    dst->setParent(this);
    src->setParent(this);
}

void VNegMInstruction::output()
{
    fprintf(yyout, "\tvneg.f32 ");
    def_list[0]->output();
    fprintf(yyout, ", ");
    use_list[0]->output();
    fprintf(yyout, "\n");
}

MachineFunction::MachineFunction(MachineUnit *p, SymbolEntry *sym_ptr)
{
    this->parent = p;
    this->sym_ptr = sym_ptr;
    this->stack_size = 0;
    addSavedRegs(11);
    addSavedRegs(14);
};

unsigned long long MachineBlock::inst_num = 0ull;

void MachineBlock::insertBefore(MachineInstruction *insertee, MachineInstruction *pin)
{
    auto it = std::find(inst_list.begin(), inst_list.end(), pin);
    inst_list.insert(it, insertee);
}

void MachineBlock::insertAfter(MachineInstruction *insertee, MachineInstruction *pin)
{
    auto it = std::find(inst_list.begin(), inst_list.end(), pin);
    if (it == inst_list.end())
    {
        pin->output();
        fflush(yyout);
    }
    assert(it != inst_list.end());
    it++;
    inst_list.insert(it, insertee);
}

void MachineBlock::output()
{
    fprintf(yyout, ".L%d:\n", this->no);
    for (auto iter : inst_list)
    {
        iter->output();
        inst_num++;
        //这里，我设置每隔666条指令就打一个文字池
        if (inst_num >= 666) 
        {                   
            inst_num = 0;
            int ltorg_num = this->getParent()->getParent()->getLtorgNum();
            fprintf(yyout, "\tb .LT%d\n", ltorg_num);
            this->getParent()->getParent()->printLTORG();
            fprintf(yyout, ".LT%d:\n", ltorg_num);
        }
    }
}

void MachineFunction::backPatch(std::vector<MachineOperand *> saved_regs)
{
    std::vector<MachineOperand *> rregs; // 创建一个向量用于存储通用寄存器
    std::vector<MachineOperand *> fregs; // 创建一个向量用于存储浮点寄存器

    // 分类保存的寄存器到通用寄存器和浮点寄存器
    for (auto reg : saved_regs)
    {
        if (reg->isFReg())
            fregs.push_back(reg); // 如果是浮点寄存器，加入 fregs
        else
            rregs.push_back(reg); // 否则，加入 rregs
    }

    // 遍历所有不确定的指令（unsure_insts），进行后期修正
    for (auto inst : unsure_insts)
    {
        // 如果指令是栈操作指令
        if (inst->isStack())
        {
            // 对于VPOP指令
            if (inst->isVPOP())
            {
                // 如果没有浮点寄存器需要保存，删除这个指令
                if (fregs.empty())
                {
                    inst->getParent()->eraseInst(inst);
                }
                // 否则，设置VPOP指令需要恢复的浮点寄存器
                else
                {
                    ((StackMInstruction *)inst)->setRegs(fregs);
                }
            }
            // 对于POP指令
            else if (inst->isPOP())
                // 设置POP指令需要恢复的通用寄存器
                ((StackMInstruction *)inst)->setRegs(rregs);
        }
        // 如果指令是加载指令
        else if (inst->isLoad())
            // 设置加载指令的偏移量，基于保存的寄存器数量
            ((LoadMInstruction *)inst)->setOff(saved_regs.size() * 4);
    }
}


std::vector<MachineOperand *> MachineFunction::getAllSavedRegs()
{
    std::vector<MachineOperand *> saved_regs; // 创建一个用于存储寄存器操作数的向量

    // 遍历保存的寄存器编号
    for (auto reg_no : this->saved_regs)
    {
        // 判断寄存器是通用寄存器还是专用寄存器
        if (reg_no < 15) // 检查寄存器编号是否小于15，表示通用寄存器
        {
            // 注：实际上，寄存器编号小于11的才是需要保存的通用寄存器
            saved_regs.push_back(new MachineOperand(MachineOperand::REG, reg_no));
        }
        else if (reg_no > 15) // 检查寄存器编号是否大于15，表示专用寄存器，如浮点寄存器
        {
            // 注：前16个专用寄存器（s0-s15）用来传递参数，这里不考虑它们
            saved_regs.push_back(new MachineOperand(MachineOperand::REG, reg_no, true)); // 添加到保存列表，标记为浮点寄存器
        }
    }

    return saved_regs; // 返回保存的所有寄存器列表
}


std::vector<MachineOperand *> MachineFunction::getSavedRegs()
{
    std::vector<MachineOperand *> saved_regs; // 创建一个用于存储寄存器操作数的向量
    for (auto reg_no : this->saved_regs) // 遍历保存的寄存器编号
    {
        if (reg_no < 15) // 检查寄存器编号是否小于15（常规寄存器的范围）
            saved_regs.push_back(new MachineOperand(MachineOperand::REG, reg_no)); // 添加到保存列表
    }
    return saved_regs; // 返回保存的寄存器列表
}


std::vector<MachineOperand *> MachineFunction::getSavedFRegs()
{
    std::vector<MachineOperand *> saved_regs; // 创建一个用于存储寄存器操作数的向量
    for (auto reg_no : this->saved_regs) // 遍历保存的寄存器编号
    {
        if (reg_no > 15) // 检查寄存器编号是否大于15（浮点寄存器的范围）
            saved_regs.push_back(new MachineOperand(MachineOperand::REG, reg_no, true)); // 添加到保存列表，标记为浮点寄存器
    }
    return saved_regs; // 返回保存的浮点寄存器列表
}


void MachineFunction::output()
{
    std::string funcName = this->sym_ptr->toStr() + "\0";
    const char *func_name = funcName.c_str() + 1;
    fprintf(yyout, "\t.global %s\n", func_name);
    fprintf(yyout, "\t.type %s , %%function\n", func_name);
    fprintf(yyout, "%s:\n", func_name);
    // TODO
    /* Hint:
     *  1. Save fp
     *  2. fp = sp
     *  3. Save callee saved register
     *  4. Allocate stack space for local variable */
    //函数序言，并处理了帧指针（fp）和栈指针（sp）。它保存了被调用者保存的寄存器，并为局部变量分配了栈空间。
    auto entry = block_list[0];//入口块
    //声明函数和设置帧指针和栈指针
    auto fp = new MachineOperand(MachineOperand::REG, 11);
    auto sp = new MachineOperand(MachineOperand::REG, 13);
    //保存被调用者保存的寄存器，将调用完之后需要恢复的保存到栈，等到时候好方便恢复
    std::vector<MachineOperand *> save_regs = getSavedRegs();
    std::vector<MachineOperand *> save_fregs = getSavedFRegs();
    (new StackMInstruction(entry, StackMInstruction::PUSH, save_regs))->output();
    //如果有浮点，就处理一下浮点去
    if (!save_fregs.empty())
        (new StackMInstruction(entry, StackMInstruction::VPUSH, save_fregs))->output();
    //将栈指针的当前值复制到帧指针寄存器中
    (new MovMInstruction(entry, MovMInstruction::MOV, fp, sp))->output();

    //分配栈空间，这就是一点一点减就行，可以理解为偏移量
    int stackSize = stack_size;
    auto stSize = new MachineOperand(MachineOperand::IMM, stackSize);
    if (stack_size != 0)
    {
        if (AsmBuilder::isLegalImm(stackSize))
        {
            (new BinaryMInstruction(entry, BinaryMInstruction::SUB, sp, sp, stSize))->output();
        }
        else
        {
            if (stackSize & 0xff)
                (new BinaryMInstruction(entry, BinaryMInstruction::SUB, sp, sp, new MachineOperand(MachineOperand::IMM, stackSize & 0xff)))->output();
            if (stackSize & 0xff00)
                (new BinaryMInstruction(entry, BinaryMInstruction::SUB, sp, sp, new MachineOperand(MachineOperand::IMM, stackSize & 0xff00)))->output();
            if (stackSize & 0xff0000)
                (new BinaryMInstruction(entry, BinaryMInstruction::SUB, sp, sp, new MachineOperand(MachineOperand::IMM, stackSize & 0xff0000)))->output();
            if (stackSize & 0xff000000)
                (new BinaryMInstruction(entry, BinaryMInstruction::SUB, sp, sp, new MachineOperand(MachineOperand::IMM, stackSize & 0xff000000)))->output();
        }
    }
    //在ARM架构中，某些操作（特别是涉及到浮点数的操作）需要sp按照8字节边界对齐，规定就是规定
    fprintf(yyout, "\tlsr sp, sp, #3\n");
    fprintf(yyout, "\tlsl sp, sp, #3\n");
    //保存寄存器和输出代码块
    backPatch(getAllSavedRegs());
    for (auto iter : block_list)
        iter->output();
}

void MachineUnit::PrintGlobalDecl()
{
    //把常量和别的分开，其中不是常量放在data区，是常量放在只读区
    std::vector<SymbolEntry *> commonVar;
    std::vector<SymbolEntry *> constVar;
    for (auto se : global_vars)
    {
        if ((se->getType()->isInt() && ((IntType *)se->getType())->isConst()) || (se->getType()->isFloat() && ((FloatType *)se->getType())->isConst()) || (se->getType()->isArray() && ((ArrayType *)se->getType())->isConst()))
        {
            constVar.push_back(se);
        }
        else
        {
            commonVar.push_back(se);
        }
    }
    if (commonVar.size() > 0)
    {
        fprintf(yyout, ".data\n");
        for (auto se : commonVar)
        {
            if (se->getType()->isArray())
            {
                if (((IdentifierSymbolEntry *)se)->isInitial() && !((IdentifierSymbolEntry *)se)->isAllZero()) // 数组有初始化
                {
                    fprintf(yyout, ".global %s\n", se->toStr().c_str() + 1);
                    //注意这地方的size我已经除完8了，别重除
                    fprintf(yyout, ".size %s, %d\n", se->toStr().c_str() + 1, (int)se->getType()->getSize());
                    fprintf(yyout, "%s:\n", se->toStr().c_str() + 1);
                    double *arrayValue = ((IdentifierSymbolEntry *)se)->getArrayValue();
                    int length = se->getType()->getSize() / 4;
                    if (((ArrayType *)se->getType())->getBaseType()->isFloat())
                    {
                        for (int i = 0; i < length; i++)
                        {
                            float value = (float)arrayValue[i];
                            uint32_t num = *((uint32_t *)&value);
                            fprintf(yyout, "\t.word %u\n", num);
                        }
                    }
                    else
                    {
                        for (int i = 0; i < length; i++)
                        {
                            fprintf(yyout, "\t.word %d\n", (int)arrayValue[i]);
                        }
                    }
                }
                else
                {
                    //.comm 可以在在bss段申请一段命名空间,该段空间的名称叫symbol, 长度为length.
                    //链接器干活
                    auto indexs = static_cast<ArrayType *>(static_cast<IdentifierSymbolEntry *>(se)->getType())->getIndexs();
                    fprintf(yyout, ".comm %s, %d\n", se->toStr().c_str() + 1, static_cast<int>(se->getType()->getSize() ));
                }
            }
            else
            {
                fprintf(yyout, ".global %s\n", se->toStr().c_str() + 1);
                fprintf(yyout, ".size %s, %d\n", se->toStr().c_str() + 1, (int)se->getType()->getSize());
                fprintf(yyout, "%s:\n", se->toStr().c_str() + 1);
                if (se->getType()->isInt()){
                    fprintf(yyout, "\t.word %d\n", (int)((IdentifierSymbolEntry *)se)->getValue());
                }
                if (se->getType()->isFloat())
                {
                    float value = (float)((IdentifierSymbolEntry *)se)->getValue();
                    uint32_t num = *((uint32_t *)&value);
                    fprintf(yyout, "\t.word %u\n", num);
                }
            }
        }
    }
    if (constVar.size() > 0)
    {
        fprintf(yyout, ".section .rodata\n");
        for (auto se : constVar)
        {
            if (se->getType()->isArray())
            {
                if (((IdentifierSymbolEntry *)se)->isInitial())
                {
                    fprintf(yyout, ".global %s\n", se->toStr().c_str() + 1);
                    fprintf(yyout, ".size %s, %d\n", se->toStr().c_str() + 1, (int)se->getType()->getSize() / 8);
                    fprintf(yyout, "%s:\n", se->toStr().c_str() + 1);
                    double *arrayValue = ((IdentifierSymbolEntry *)se)->getArrayValue();
                    int length = se->getType()->getSize() / 4;
                    if (((ArrayType *)se->getType())->getBaseType()->isFloat())
                    {
                        for (int i = 0; i < length; i++)
                        {
                            float value = (float)arrayValue[i];
                            uint32_t num = *((uint32_t *)&value);
                            fprintf(yyout, "\t.word %u\n", num);
                        }
                    }
                    else
                    {
                        for (int i = 0; i < length; i++)
                        {
                            fprintf(yyout, "\t.word %d\n", (int)arrayValue[i]);
                        }
                    }
                }
                else
                {
                    fprintf(yyout, ".comm %s, %d\n", se->toStr().c_str() + 1, (int)se->getType()->getSize() );
                }
            }
            else
            {
                fprintf(yyout, ".global %s\n", se->toStr().c_str() + 1);
                fprintf(yyout, ".size %s, %d\n", se->toStr().c_str() + 1, (int)se->getType()->getSize() / 8);
                fprintf(yyout, "%s:\n", se->toStr().c_str() + 1);
                if (se->getType()->isInt())
                    fprintf(yyout, "\t.word %d\n", (int)((IdentifierSymbolEntry *)se)->getValue());
                if (se->getType()->isFloat())
                {
                    float value = (float)((IdentifierSymbolEntry *)se)->getValue();
                    uint32_t num = *((uint32_t *)&value);
                    fprintf(yyout, "\t.word %u\n", num);
                }
            }
        }
    }
}

void MachineUnit::printLTORG()
{
    // 打印文字池
    fprintf(yyout, "\n.LTORG\n");
    if (global_vars.size() > 0)
    {
        for (auto se : global_vars)
        {
            fprintf(yyout, "addr_%s_%d:\n", se->toStr().c_str() + 1, ltorg_num);
            fprintf(yyout, "\t.word %s\n", se->toStr().c_str() + 1);
        }
    }
    ltorg_num++;
}

void MachineUnit::output()
{
    // TODO
    fprintf(yyout, 
        "\t.cpu cortex-a72\n"
        "\t.arch armv8-a\n"
        "\t.fpu vfpv3-d16\n"
        "\t.arch_extension crc\n"
        "\t.arm\n");
    PrintGlobalDecl();
    fprintf(yyout, "\n\t.text\n");
    for (auto iter : func_list)
        iter->output();
    printLTORG();
}

SMULLMInstruction::SMULLMInstruction(MachineBlock *p, MachineOperand *lo, MachineOperand *hi, MachineOperand *src1, MachineOperand *src2, int cond)
{
    this->parent = p;
    this->type = MachineInstruction::SMULL;
    this->op = -1;
    this->cond = cond;
    def_list.push_back(lo);
    def_list.push_back(hi);
    use_list.push_back(src1);
    use_list.push_back(src2);
    lo->setParent(this);
    hi->setParent(this);
    src1->setParent(this);
    src2->setParent(this);
}
//有符号长乘法
void SMULLMInstruction::output()
{
    fprintf(yyout, "\tsmull ");
    def_list[0]->output();
    fprintf(yyout, ", ");
    def_list[1]->output();
    fprintf(yyout, ", ");
    use_list[0]->output();
    fprintf(yyout, ", ");
    use_list[1]->output();
    fprintf(yyout, "\n");
}