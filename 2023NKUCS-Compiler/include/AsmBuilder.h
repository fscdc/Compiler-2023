#ifndef __ASMBUILDER_H__
#define __ASMBUILDER_H__

#include "MachineCode.h"
#include <bitset>

class AsmBuilder
{
private:
    MachineUnit *mUnit;         // mahicne unit
    MachineFunction *mFunction; // current machine code function;
    MachineBlock *mBlock;       // current machine code block;
    int cmpOpcode;              // CmpInstruction opcode, for CondInstruction;
    std::set<unsigned int> legalVMOVImms;

public:
    std::map<Operand *, std::pair<MachineOperand *, MachineOperand *>> smullSig2Doub;
    AsmBuilder()
    {
        legalVMOVImms.clear();
        auto genLegalFloat = [&](unsigned char imm)
        {
            std::bitset<8> bits(imm);
            std::bitset<32> fbits(0);
            fbits[31] = bits[7];
            fbits[30] = (~bits[6]);
            fbits[29] = fbits[28] = fbits[27] = fbits[26] = fbits[25] = bits[6];
            fbits[24] = bits[5];
            fbits[23] = bits[4];
            fbits[22] = bits[3];
            fbits[21] = bits[2];
            fbits[20] = bits[1];
            fbits[19] = bits[0];
            unsigned int fval = fbits.to_ulong();
            return fval;
        };
        unsigned char imm = 0;
        unsigned int floatNum;
        for (imm = 0; imm <= 255; imm++)
        {
            floatNum = genLegalFloat(imm);
            legalVMOVImms.insert(floatNum);
            if (imm == 255)
                break;
        }
    };
    void setUnit(MachineUnit *unit) { this->mUnit = unit; };
    void setFunction(MachineFunction *func) { this->mFunction = func; };
    void setBlock(MachineBlock *block) { this->mBlock = block; };
    void setCmpOpcode(int opcode) { this->cmpOpcode = opcode; };
    MachineUnit *getUnit() { return this->mUnit; };
    MachineFunction *getFunction() { return this->mFunction; };
    MachineBlock *getBlock() { return this->mBlock; };
    int getCmpOpcode() { return this->cmpOpcode; };
    static bool isLegalImm(int imm)
    {
        unsigned int num = (unsigned int)imm;
        // 判断是不是8位移动生成
        for (int i = 0; i < 16; i++)
        {
            if (num <= 0xff)
            {
                return true;
            }
            num = ((num << 2) | (num >> 30));
        }
        return false;
    }
    bool couldUseVMOV(int imm)
    {
        unsigned int uimm = (unsigned int)imm;
        return legalVMOVImms.find(uimm) != legalVMOVImms.end();
    }
    static int isPowNumber(int imm)
    {
        if ((imm & (imm - 1)) != 0)
            return -1;
        int x = 0;
        while (imm > 1)
        {
            imm >>= 1;
            x++;
        }
        return x;
    }
};

#endif