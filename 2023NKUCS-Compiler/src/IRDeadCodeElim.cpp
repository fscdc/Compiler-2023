#include "IRDeadCodeElim.h"
#include "Type.h"
#include <vector>

using namespace std;

//初始化，也会初步标记一些关键指令
void IRDeadCodeElim::initalize(Function *func)
{
    worklist.clear();
    // 遍历函数的每一个基本块，完成初始化标记操作
    for (auto it = func->begin(); it != func->end(); it++)
    {
        (*it)->unsetMark();//基本块初始默认是未标记的
        (*it)->cleanAllMark();//每个基本块的中所有指令默认是未标记的
        //遍历指令，标记一些明显的指令
        for (auto it1 = (*it)->begin(); it1 != (*it)->end(); it1 = it1->getNext())
        {      
            if (it1->isCritical())
            {
                //如果是关键指令，就标记它及它所在的基本块
                it1->setMark();
                it1->getParent()->setMark();
                worklist.push_back(it1);
            }
        }
    }
}


void IRDeadCodeElim::markforins(Function *func) {
    //计算函数的反向数据流
    func->computeRDF();

    while (!worklist.empty()) {
        auto ins = worklist.back();
        worklist.pop_back();
        //遍历所有引用，并把未标记的标记上
        auto uses = ins->getUse();
        for (auto it : uses) {
            auto def = it->getDef();
            if (def && !def->getMark()) {
                def->setMark();
                def->getParent()->setMark();
                worklist.push_back(def);
            }
        }

        //处理指令的定义
        auto def = ins->getDef();
        if (def) {
            // 遍历定义的所有引用
            for (auto use = def->use_begin(); use != def->use_end(); use++) {
                // 如果引用未被标记且是无条件或有条件的，进行标记并添加到工作列表
                if (!(*use)->getMark()&& ((*use)->isUncond() || (*use)->isCond())) {
                (*use)->setMark();
                (*use)->getParent()->setMark();
                worklist.push_back(*use);
                }
            }
        }

        //当前指令所在基本块的RDF集合基本块
        auto block = ins->getParent();
        for (auto b : block->domFrontier) {
            Instruction *in = b->rbegin();
            //如果指令未被标记且是无条件或有条件的，进行标记并添加到工作列表
            if (!in->getMark() && (in->isCond() || in->isUncond())) {
                in->setMark();
                in->getParent()->setMark();
                worklist.push_back(in);
            }
        }

        //由于做了mem2reg，这里要处理phi指令，增加对于phi前驱的block的保留
        for (auto in = block->begin(); in != block->end(); in = in->getNext()) {
            if (!in->isPhi())
                continue;
            auto phi = (PhiInstruction *)in;
            for (auto it : phi->getSrcs()) {
                Instruction *in = it.first->rbegin();
                if (!in->getMark() && (in->isCond() || in->isUncond())) {
                    in->setMark();
                    in->getParent()->setMark();
                    worklist.push_back(in);
                }
            }
        }
    }
}

bool IRDeadCodeElim::removeins(Function *func) {
    vector<Instruction *> temp;
    bool ret = false;
    //遍历函数中的所有基本块的指令
    for (auto &block : func->getBlockList()) {
        for (auto it = block->begin(); it != block->end(); it = it->getNext()) {
            //检查没有被标记的指令
            if (!it->getMark()) {
                if (it->isRet()) {
                    // 如果返回指令没有被标记，替换它的操作数为0
                    auto zero =new Operand(new ConstantSymbolEntry(TypeSystem::intType, 0));
                    it->replaceUse(it->getUse()[0], zero);
                    continue;
                }
                //无条件跳转不删除
                if (!it->isUncond()) {
                    temp.push_back(it);
                }
                //处理条件跳转
                if (it->isCond()) {
                    // 获取与当前基本块相关联的标记过的后支配节点
                    BasicBlock *b = func->getMarkBranch(block);
                    if (!b) {
                        // 如果没有找到，说明整个函数可能无用，不再处理
                        return false;
                    }
                    new UncondBrInstruction(b, block);
                    //处理一下前驱后继关系
                    block->cleanAllSucc();
                    block->addSucc(b);
                    b->addPred(block);
                }
            }
        }
    }
    if (temp.size())
        ret = true;
    // 移除所有标记为删除的指令
    for (auto i : temp) {
        //要把指令的use也都给移除掉，最后再在基本块中删除这条指令
        for (auto useOp : i->getUse())
        {
            useOp->removeUse(i);
        }
        i->getParent()->remove(i);
    }
    return ret;
}

