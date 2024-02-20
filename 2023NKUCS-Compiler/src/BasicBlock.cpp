#include "BasicBlock.h"
#include "Function.h"
#include <algorithm>
#include "AsmBuilder.h"

extern FILE* yyout;

void BasicBlock::cleanAllMark()
{
    for (auto it = begin(); it != end(); it = it->getNext())
    {
        it->unsetMark();
    }
}

void BasicBlock::cleanAllSucc()
{
    for (auto i : succ)
        i->removePred(this);
    succ.clear();
    // std::vector<BasicBlock*>().swap(succ);
}


// insert the instruction to the front of the basicblock.
void BasicBlock::insertFront(Instruction *inst, bool isArray = false)
{
    Instruction *pin = head->getNext();
    if (isArray)
    {
        while (pin && pin->isAlloca())
        {
            pin = pin->getNext();
        }
        pin = pin ? pin : head;
    }
    insertBefore(inst, pin);
}

// insert the instruction to the back of the basicblock.
void BasicBlock::insertBack(Instruction *inst) 
{
    insertBefore(inst, head);
}

// insert the instruction dst before src.
void BasicBlock::insertBefore(Instruction *dst, Instruction *src)
{
    //todo1
    // 让dst插到src前面
    dst->setPrev(src->getPrev());
    dst->setNext(src);
    src->getPrev()->setNext(dst);
    src->setPrev(dst);
    dst->setParent(this);
}

// remove the instruction from intruction list.
void BasicBlock::remove(Instruction *inst)
{
    inst->getPrev()->setNext(inst->getNext());
    inst->getNext()->setPrev(inst->getPrev());
}

void BasicBlock::output() const
{
    fprintf(yyout, "B%d:", no);

    if (!pred.empty())
    {
        fprintf(yyout, "%*c; preds = %%B%d", 32, '\t', (*pred.begin())->getNo());
        auto i = pred.begin();
        i++;
        for (i; i != pred.end(); i++)
            fprintf(yyout, ", %%B%d", (*i)->getNo());
    }
    fprintf(yyout, "\n");
    for (auto i = head->getNext(); i != head; i = i->getNext())
    {
        i->output();
    }
}

void BasicBlock::addSucc(BasicBlock *bb)
{
    succ.insert(bb);
}

// remove the successor basicclock bb.
void BasicBlock::removeSucc(BasicBlock *bb)
{
    auto it = std::find(succ.begin(), succ.end(), bb);
    if (it == succ.end())
    {
        return;
    }
    succ.erase(it);
}

void BasicBlock::addPred(BasicBlock *bb)
{
    pred.insert(bb);
}

// remove the predecessor basicblock bb.
void BasicBlock::removePred(BasicBlock *bb)
{
    auto it = std::find(pred.begin(), pred.end(), bb);
    if (it == pred.end())
    {
        return;
    }
    pred.erase(it);
}

BasicBlock::BasicBlock(Function *f)
{
    this->no = SymbolTable::getLabel();
    f->insertBlock(this);
    parent = f;
    head = new DummyInstruction();
    head->setParent(this);
}

BasicBlock::~BasicBlock()
{
    Instruction *inst;
    inst = head->getNext();
    while (inst != head)
    {
        Instruction *t;
        t = inst;
        inst = inst->getNext();
        delete t;
    }
    for(auto &bb:pred)
        bb->removeSucc(this);
    for(auto &bb:succ)
        bb->removePred(this);
    parent->remove(this);
}


void BasicBlock::strongRemove(Instruction *inst)
{
    for (auto &use : inst->getUse())
        use->removeUse(inst);
    remove(inst);
}

void BasicBlock::genMachineCode(AsmBuilder* builder) 
{
    auto cur_func = builder->getFunction();
    auto cur_block = new MachineBlock(cur_func, no);
    builder->setBlock(cur_block);
    for (auto i = head->getNext(); i != head; i = i->getNext())
    {
        i->genMachineCode(builder);
    }
    cur_func->InsertBlock(cur_block);
}