#include "IRComSubExprElim.h"
#include <queue>

bool IRComSubExprElim::localCSE(Function *func)
{
    bool result = true;
    std::vector<Expr> exprs;
    //遍历每个函数的所有基本块的所有指令
    for (auto block = func->begin(); block != func->end(); block++)
    {
        exprs.clear();
        for (auto inst = (*block)->begin(); inst != (*block)->end(); inst = inst->getNext())
        {
            //跳过没有识别为表达式的指令
            if (skip(inst))
                continue;
            //需要考虑无效化，坑死我了，这地方需要去再额外判定一下，load指令是不行的
            if(inst->isLoad())
                continue;
            auto preInstIt = std::find(exprs.begin(), exprs.end(), Expr(inst));
            if (preInstIt != exprs.end())
            {
                // TODO: 把对当前指令的def的use改成对于preInst的def的use，并删除当前指令。
                Instruction* preinstruction = preInstIt->inst;//获取到指令
                //创建一个包含所有使用当前指令定义的结果的指令列表
                std::vector<Instruction *> uses(inst->getDef()->getUse());
                result = false;
                //遍历前文指令列表，并将其替换为使用之前找到的指令
                for (auto useInst : uses)
                {
                    useInst->replaceUse(inst->getDef(), preinstruction->getDef());
                }

                preinstruction = inst->getPrev();
                //从其父基本块中安全地移除当前指令
                (*block)->strongRemove(inst);
                inst = preinstruction;                
            }
            else
            {
                exprs.emplace_back(inst);
                 /**
                 * 这里不需要考虑表达式注销的问题
                 * 因为ir是ssa形式的代码，目前来说应该不会有这样的情况，这种是错的
                 * a = b + c
                 * b = d + f
                 */
            }
        }
    }
    return result;
}

bool IRComSubExprElim::globalCSE(Function *func)
{
    exprVec.clear();
    ins2Expr.clear();
    genBB.clear();
    killBB.clear();
    inBB.clear();
    outBB.clear();
    expr2Op.clear();
    outBBOp.clear();    
    bool result = true; 
    calGenKill(func);
    calInOut(func);
    result = removeGlobalCSE(func);
    return result;
}

void IRComSubExprElim::calGenKill(Function *func)
{
    // 计算gen
    std::vector<int> removeExpr;
    for (auto block = func->begin(); block != func->end(); block++)
    {
        for (auto inst = (*block)->begin(); inst != (*block)->end(); inst = inst->getNext())
        {         
            if (skip(inst))
                continue;
            Expr expr(inst);
            // 对于表达式a + b，我们只需要全局记录一次，重复出现的话，用同一个id即可
            auto it = find(exprVec.begin(), exprVec.end(), expr);
            int ind = it - exprVec.begin();
            if (it == exprVec.end())
            {
                exprVec.push_back(expr);
            }
            ins2Expr[inst] = ind;
            genBB[*block].insert(ind);
            expr2Op[*block][ind] = inst->getDef();//注意这句，坑我debug3小时
            /*
                一个基本块内不会出现这种 t1 = t2 + t3
                                       t2 = ...
                所以这里，之后gen的表达式不会kill掉已经gen的表达式
                就算是phi指令，也是并行取值，所以问题不大哦
            */
        }
    }
    // 计算kill
    for (auto block = func->begin(); block != func->end(); block++)
    {
        for (auto inst = (*block)->begin(); inst != (*block)->end(); inst = inst->getNext())
        {
            if (inst->getDef() != nullptr)
            {
                for (auto useInst : inst->getDef()->getUse())
                {
                    if (!skip(useInst))
                        killBB[*block].insert(ins2Expr[useInst]);
                }
            }
        }
    }
}

void IRComSubExprElim::calInOut(Function *func)
{
    std::set<int> U;
    for (size_t i = 0; i < exprVec.size(); i++)
        U.insert(i);
    auto entry = func->getEntry();
    inBB[entry].clear();
    outBB[entry] = genBB[entry];
    //初始化除entry外的基本块的out为U
    std::set<BasicBlock *> workList;
    for (auto block = func->begin(); block != func->end(); block++)
    {
        if (*block != entry) {
            outBB[*block] = U;
            workList.insert(*block);
        }
    }
    //不断迭代直到收敛
    while (!workList.empty())
    {
        auto block = *workList.begin();
        workList.erase(workList.begin());
        // 计算in[block] = U outBB[predBB];
        std::set<int> in[2];
        if (block->getNumOfPred() > 0)
            in[0] = outBB[*block->pred_begin()];
        auto it = block->pred_begin();
        it++;
        int turn = 1;
        for (; it != block->pred_end(); it++)
        {
            in[turn].clear();
            std::set_intersection(outBB[*it].begin(), outBB[*it].end(), in[turn ^ 1].begin(), in[turn ^ 1].end(), inserter(in[turn], in[turn].begin()));
            turn ^= 1;
        }
        inBB[block] = in[turn ^ 1];
        // 计算outBB[block] = (inBB[block] - killBB[block]) U genBB[block];
        std::set<int> midDif;
        std::set<int> out;
        std::set_difference(inBB[block].begin(), inBB[block].end(), killBB[block].begin(), killBB[block].end(), inserter(midDif, midDif.begin()));
        std::set_union(genBB[block].begin(), genBB[block].end(), midDif.begin(), midDif.end(), inserter(out, out.begin()));
        if (out != outBB[block])
        {
            outBB[block] = out;
            for (auto succ = block->succ_begin(); succ != block->succ_end(); succ++)
                workList.insert(*succ);
        }
    }
}

bool IRComSubExprElim::removeGlobalCSE(Function *func)
{
    // TODO: 根据计算出的gen kill in out进行全局公共子表达式消除
    bool result = true;
    bool outChanged = true;
    while (outChanged)
    {
        outChanged = false;
        //遍历函数的所有基本块
        for (auto bb = func->begin(); bb != func->end(); bb++)
        {
            //对于每个基本块，创建一个新的 out 集合 newOutBBOp，用于存储子表达式及其相关操作数。
            std::set<std::pair<int, Operand *>> newOutBBOp;
            //对于 out 集合中的每个表达式，如果它不在 in 集合中，或者它在 kill 集合中，则将其添加到新的 out 集合中。
            for (auto &outExpr : outBB[*bb])
            {
                // 如果In无，则用新的，如果In有，且kill，则用新的
                if (inBB[*bb].find(outExpr) == inBB[*bb].end() || killBB[*bb].find(outExpr) != killBB[*bb].end())
                {
                    newOutBBOp.insert(std::make_pair(outExpr, expr2Op[*bb][outExpr]));
                }
            }
            //如果一个基本块有多个前驱，就算有许多表达式，也不处理，因为这地方有phi指令，继承也继承的不对
            //所以这里仅处理只有一个前驱的基本块
            if ((*bb)->getNumOfPred() == 1)
            {
                auto preBB = *(*bb)->pred_begin();
                for (auto &inExpr : outBBOp[preBB])
                {
                    //从这个前驱基本块的 out 集合中继承表达式
                    if (killBB[*bb].find(inExpr.first) == killBB[*bb].end())
                    {
                        newOutBBOp.insert(std::make_pair(inExpr.first, inExpr.second));
                    }
                }
            }
            //如果新计算的 out 集合与原有的不同，就更新一下并维护一下outChanged
            if (newOutBBOp != outBBOp[*bb])
            {
                outBBOp[*bb] = newOutBBOp;
                outChanged = true;
                fprintf(stderr,"outBBp getvalue\n");
            }
        }
    }
    //遍历函数 func 中的所有基本块
    for (auto bb = func->begin(); bb != func->end(); bb++)
    {
        auto preds = (*bb)->getPred();
        bool selfLoop = (preds.size() == 2 && std::find(preds.begin(), preds.end(), *bb) != preds.end());
        //如果不是单个前驱或者自循环就跳过
        if ( (preds.size() != 1) && !selfLoop)
            continue;
        // 确定前驱基本块
        BasicBlock *preBB = nullptr;
        preBB = preds.size() == 1 ? preds[0] : (preds[0] == *bb ? preds[1] : preds[0]);

        std::map<int, Operand *> preBBOutOp;//构建一个映射，包含前驱基本块的输出操作
        for (auto &pa : outBBOp[preBB])//遍历前驱基本块的输出操作，将其存在新创建的这个映射中
        {
            preBBOutOp[pa.first] = pa.second;//其中 pa.first 是整数键，pa.second 是与该键相关联的 Operand 指针。
            fprintf(stderr, "preBBOutOp updated: key=%d, Operand pointer=%p\n", pa.first, static_cast<void*>(pa.second));
        }
        //遍历基本块中的指令
        for (auto inst = (*bb)->begin(); inst != (*bb)->end(); inst = inst->getNext())
        {
            if (skip(inst))
                continue;
            //检查当前指令是否在前驱基本块的输出操作 preBBOutOp 中
            bool inInst = (preBBOutOp.find(ins2Expr[inst]) != preBBOutOp.end());
            if (!inInst || inst->isLoad())
                continue;
            //找到就进行elim
            result = false;
            std::vector<Instruction *> uses(inst->getDef()->getUse());
            for (auto useInst : uses)
            {
                //遍历当前指令的所有使用点，并将它们替换为前驱基本块的对应操作
                useInst->replaceUse(inst->getDef(), preBBOutOp[ins2Expr[inst]]);
            }
            //移除当前指令并更新迭代器
            auto preInst = inst->getPrev();
            (*bb)->strongRemove(inst);
            inst = preInst;
        }
    }
    return result;
}


