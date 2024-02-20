// #include "LiveVariableAnalysis.h"
// #include "MachineCode.h"
// #include <iostream>
// #include <algorithm>
// #include <queue>
// #include <chrono>

// extern FILE *yyout;

// void LiveVariableAnalysis::pass(MachineUnit *unit)
// {
//     for (auto &func : unit->getFuncs())
//     {
//         computeUsePos(func);
//         computeDefUse(func);
//         iterate(func);
//     }
// }

// void LiveVariableAnalysis::pass(MachineFunction *func)
// {
//     computeUsePos(func);
//     computeDefUse(func);
//     iterate(func);
// }

// void LiveVariableAnalysis::computeDefUse(MachineFunction *func)
// {
//     for (auto &block : func->getBlocks())
//     {
//         for (auto inst = block->getInsts().begin(); inst != block->getInsts().end(); inst++)
//         {
//             auto user = (*inst)->getUse();
//             std::set<MachineOperand *> temp(user.begin(), user.end());
//             set_difference(temp.begin(), temp.end(),
//                            def[block].begin(), def[block].end(), inserter(use[block], use[block].end()));
//             auto defs = (*inst)->getDef();
//             for (auto &d : defs)
//                 def[block].insert(all_uses[*d].begin(), all_uses[*d].end());
//         }
//     }
// }

// void LiveVariableAnalysis::iterate(MachineFunction *func)
// {
//     std::set<MachineBlock *> workList;
//     for (auto &block : func->getBlocks())
//     {
//         block->live_in.clear();
//         workList.insert(block);
//     }

//     while (!workList.empty())
//     {
//         auto block = *workList.begin();
//         workList.erase(workList.begin());
//         block->live_out.clear();
//         auto old = block->live_in;
//         for (auto &succ : block->getSuccs())
//         {
//             block->live_out.insert(succ->live_in.begin(), succ->live_in.end());
//         }
//         block->live_in = use[block];
//         set_difference(block->live_out.begin(), block->live_out.end(),
//                        def[block].begin(), def[block].end(), inserter(block->live_in, block->live_in.end()));
//         if (old != block->live_in)
//         {
//             for (auto &pred : block->getPreds())
//                 workList.insert(pred);
//         }
//     }
// }

// void LiveVariableAnalysis::computeUsePos(MachineFunction *func)
// {
//     for (auto &block : func->getBlocks())
//     {
//         for (auto &inst : block->getInsts())
//         {
//             auto uses = inst->getUse();
//             for (auto &use : uses)
//             {
//                 all_uses[*use].insert(use);
//             }
//         }
//     }
// }