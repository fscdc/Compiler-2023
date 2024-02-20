//基本块
#ifndef __BASIC_BLOCK_H__
#define __BASIC_BLOCK_H__
#include <vector>
#include <set>
#include "Instruction.h"
#include <unordered_set>

class Function;

class BasicBlock
{
    typedef std::unordered_set<BasicBlock *>::iterator bb_iterator;

private:
    std::unordered_set<BasicBlock *> pred, succ;//表示基本块的前驱（pred）和后继（succ）基本块
    Instruction *head;//表示基本块中的头指令
    Function *parent;//表示这个基本块属于的函数
    int no;//表示基本块的编号

    bool mark;

public:
    BasicBlock(Function *);
    ~BasicBlock();
    void insertFront(Instruction *, bool);
    void insertBack(Instruction *);
    void insertBefore(Instruction *, Instruction *);
    void remove(Instruction *);
    bool empty() const { return head->getNext() == head;}
    void output() const;
    bool succEmpty() const { return succ.empty(); };
    bool predEmpty() const { return pred.empty(); };
    void addSucc(BasicBlock *);
    void removeSucc(BasicBlock *);
    void addPred(BasicBlock *);
    void removePred(BasicBlock *);
    int getNo() { return no; };
    Function *getParent() { return parent; };
    Instruction* begin() { return head->getNext();};
    Instruction* end() { return head;};
    Instruction* rbegin() { return head->getPrev();};//获取BB的最后一条指令
    Instruction* rend() { return head;};
    bb_iterator succ_begin() { return succ.begin(); };
    bb_iterator succ_end() { return succ.end(); };
    bb_iterator pred_begin() { return pred.begin(); };
    bb_iterator pred_end() { return pred.end(); };
    int getNumOfPred() const { return pred.size(); };
    int getNumOfSucc() const { return succ.size(); };

    void strongRemove(Instruction *);
    
    int order;
    std::unordered_set<BasicBlock *> domFrontier;

    void unsetMark() { mark = false; };
    void cleanAllMark();
    void setMark() { mark = true; };
    bool getMark() { return mark; };
    void cleanAllSucc();

    std::vector<BasicBlock *> getPred() { return std::vector<BasicBlock *>(pred.begin(), pred.end()); };
    std::vector<BasicBlock *> getSucc() { return std::vector<BasicBlock *>(succ.begin(), succ.end()); };
    void insertAfter(Instruction *dst, Instruction *src)
    {
        dst->setNext(src->getNext());
        src->getNext()->setPrev(dst);

        dst->setPrev(src);
        src->setNext(dst);

        dst->setParent(this);
    }
    void genMachineCode(AsmBuilder*);
};

#endif