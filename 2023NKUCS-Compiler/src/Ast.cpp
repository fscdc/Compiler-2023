#include "Ast.h"
#include "SymbolTable.h"
#include "Unit.h"
#include "Instruction.h"
#include "IRBuilder.h"
#include <string>
#include <assert.h>
#include "Type.h"
#include <queue>

extern FILE *yyout;
int Node::counter = 0;
IRBuilder* Node::builder = nullptr;
Type* returntype = nullptr;
bool llvm_memset_func;
extern Unit unit;

//创建用于表示0的操作数
SymbolEntry *se_const0 = new ConstantSymbolEntry(TypeSystem::constIntType, 0);
ExprNode *temp_const0 = new Constant(se_const0);
Operand *src0_const0 = temp_const0->getOperand();

//创建表示浮点数0.0的操作数
SymbolEntry *se_const0_float = new ConstantSymbolEntry(TypeSystem::constFloatType, 0.0);
ExprNode *temp_const0_float = new Constant(se_const0_float);
Operand *src0_const0_float = temp_const0_float->getOperand();


//声明全局指令
std::vector<Instruction *> global; //存放全局指令，在根结点输出时候一起输出

std::queue<double *>arrayValue; //所有全局数组中的每个元素的初值
std::queue<Operand *>global_arr_ids; //所有全局数组的id

#define now_bb (builder->getInsertBB())

Operand* Node::typeCast(Type* targetType, Operand* operand) {
    // 首先判断是否真的需要类型转化
    if(!TypeSystem::needCast(operand->getType(), targetType)) {
        fprintf(stderr, "No need to take typecast\n");
        return operand;
    }
    fprintf(stderr, "Need to take typecast\n");
    BasicBlock *bb = builder->getInsertBB();
    Operand* retOperand = new Operand(new TemporarySymbolEntry(targetType, SymbolTable::getLabel()));
    //bool->int
    if(operand->getType()->isBool() && targetType->isInt()) {
        // 插入一条符号扩展指令
        new ZextInstruction(retOperand,operand , bb);
        return retOperand;
    }
    //int->float
    else if(operand->getType()->isInt() && targetType->isFloat()) {
        // 插入一条类型转化指令
        new IntFloatCastInstruction(IntFloatCastInstruction::S2F, retOperand,operand , bb);
        return retOperand;
    }
    //float->int
    else if(operand->getType()->isFloat() && targetType->isInt()) {
        // 插入一条类型转化指令
        new IntFloatCastInstruction(IntFloatCastInstruction::F2S, retOperand,operand, bb);
        return retOperand;
    }
    return operand;
}

Node::Node()
{
    seq = counter++;
    next = nullptr;
}

void Node::setNext(Node *node)
{
    Node *temp = this;
    // 找到最后一个结点，然后设置
    while (temp->getNext())
    {
        temp = temp->getNext();
    }
    temp->next = node;
}

Node *Node::getNext()
{
    return next;
}


//回填函数，用于处理那些在代码生成阶段尚未确定最终目标地址的跳转指令
//参数说明：list包含了需要进行回填操作的指令集；target表示要设置为跳转目标的基本块；branch指示是否处理真分支或假分支
void Node::backPatch(std::vector<Instruction *> &list, BasicBlock *target, bool branch)
{
    if (branch)
        for (auto &inst : list)
        //遍历 list 中的每个对象
        {
            if (inst->isCond())//如果它是一个条件跳转指令，将target设置为真分支的目标。
                dynamic_cast<CondBrInstruction *>(inst)->setTrueBranch(target);
            else if (inst->isUncond())//如果它是一个无条件跳转指令，将target设置为跳转目标。
                dynamic_cast<UncondBrInstruction *>(inst)->setBranch(target);
        }
    else
        for (auto &inst : list)
        //遍历 list 中的每个对象，下述逻辑类似上文
        {
            if (inst->isCond())
                dynamic_cast<CondBrInstruction *>(inst)->setFalseBranch(target);
            else if (inst->isUncond())
                dynamic_cast<UncondBrInstruction *>(inst)->setBranch(target);
        }
}

//合并两个指令集
std::vector<Instruction *> Node::merge(std::vector<Instruction *> &list1, std::vector<Instruction *> &list2)
{
    std::vector<Instruction *> res(list1);
    res.insert(res.end(), list2.begin(), list2.end());
    return res;
}

void Ast::genCode(Unit *unit)
{
    fprintf(stderr, "Ast::genCode\n");
    IRBuilder *builder = new IRBuilder(unit);
    Node::setIRBuilder(builder);
    root->genCode();

    //声明sysy运行时库（链接库）
    // fprintf(yyout, "declare i32 @getint()\n");
    // fprintf(yyout, "declare void @putint(i32)\n");
    // fprintf(yyout, "declare i32 @getch()\n");
    // fprintf(yyout, "declare void @putch(i32)\n");
    // fprintf(yyout, "declare float @getfloat()\n");
    // fprintf(yyout, "declare void @putfloat(float)\n");  
    // fprintf(yyout, "declare i32 @getarray(i32*)\n");
    // fprintf(yyout, "declare i32 @getfarray(float*)\n");
    // fprintf(yyout, "declare void @putarray(i32, i32*)\n");
    // fprintf(yyout, "declare void @putfarray(i32, float*)\n");
    // fprintf(yyout, "declare void @starttime()\n");
    // fprintf(yyout, "declare void @stoptime()\n");  
    // fprintf(yyout, "declare void @putf(i8*, ...)\n\n");

    // //输出全局指令（比如全局常变量声明的那种）
    // for (int i = 0; i < int(global.size()); i++)
    // {
    //     global[i]->output();
    // }
}
//二元运算
void BinaryExpr::genCode()
{
    fprintf(stderr, "BinaryExpr::genCode\n");
    BasicBlock *bb = builder->getInsertBB();//获取当前基本块
    Type* maxType = TypeSystem::getMaxType(expr1->getSymPtr()->getType(), expr2->getSymPtr()->getType());
    //处理and操作
    if (op == AND)
    {
        Function *func = bb->getParent();//获取基本块所属函数
        expr1->genCode();
        //检查操作数不能是空，即void类型（类型检查基础要求3）
        if (expr1->getOperand() == nullptr || expr2->getOperand() == nullptr)
        {
            fprintf(stderr, "BinaryExpr can't be void type\n");
            assert(expr1->getOperand() != nullptr);
            assert(expr2->getOperand() != nullptr);
        }
        Instruction *test = bb->rbegin();//获取基本块的最后一条指令
        //考虑if(10&&21)，第一个表达式为算术表达式
        if (!test->isCond() && !test->isUncond())
        {
            int opcode = CmpInstruction::NE;
            Operand *src1 = expr1->getOperand();
            Operand *src2 = src0_const0;
            //dst比较指令目的寄存器
            SymbolEntry *tse = new TemporarySymbolEntry(TypeSystem::boolType, SymbolTable::getLabel());
            Operand *dst = new Operand(tse);
            Operand *n1 = src1;
            //bool->int转换是通过零扩展指令实现（类型检查基础要求2）
            if (src1->getType() == TypeSystem::boolType)
            {
                SymbolEntry *s = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
                n1 = new Operand(s);
                new ZextInstruction(n1, src1, bb);
            }
            if(src1->getType() == TypeSystem::floatType){
                src2 = src0_const0_float;
            }
            //将 expr1 的结果与 0 比较
            new CmpInstruction(opcode, dst, n1, src2, bb);
            //等待回填
            Instruction *temp = new CondBrInstruction(nullptr, nullptr, dst, bb);
            //将这个条件分支指令添加到 expr1 的真假跳转列表中。
            expr1->trueList().push_back(temp);
            expr1->falseList().push_back(temp);
        }
        //新创建一个基本块，如果第一个表达式结果为真，则跳到这个基本块
        BasicBlock *trueBB = new BasicBlock(func);
        //修正 expr1 的真跳转列表中的跳转目标为新创建的 trueBB。
        backPatch(expr1->trueList(), trueBB, true);

        builder->setInsertBB(trueBB);//设置当前的第二个表达式对应的指令要插入到的基本块为trueBB

        expr2->genCode();
        //下面过程类似上文
        bb = builder->getInsertBB();
        test = bb->rbegin();
        if (!test->isCond() && !test->isUncond())
        {
            int opcode = CmpInstruction::NE;
            Operand *src1 = expr2->getOperand();
            Operand *src2 = src0_const0;
            SymbolEntry *tse = new TemporarySymbolEntry(TypeSystem::boolType, SymbolTable::getLabel());
            Operand *dst = new Operand(tse);
            Operand *n1 = src1;
            // bool->int转换是通过零扩展指令实现（类型检查基础要求2）
            if (src1->getType() == TypeSystem::boolType)
            {
                SymbolEntry *s = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
                n1 = new Operand(s);
                new ZextInstruction(n1, src1, bb);
            }
            if(src1->getType() == TypeSystem::floatType){
                src2 = src0_const0_float;
            }
            new CmpInstruction(opcode, dst, n1, src2, bb);
            //等待回填
            Instruction *temp = new CondBrInstruction(nullptr, nullptr, dst, bb);
            expr2->trueList().push_back(temp);
            expr2->falseList().push_back(temp);
        }
        //真列表 (true_list) 由 expr2 的真列表构成。
        true_list = expr2->trueList();
        //假列表 (false_list) 由 expr1 和 expr2 的假列表合并而成。
        false_list = merge(expr1->falseList(), expr2->falseList());
    }
    //处理or操作，处理是类似and操作的，但是有所区别
    else if (op == OR)
    {
        Function *func = bb->getParent();//获取基本块所属函数
        expr1->genCode();
        //检查操作数不能是空，即void类型（类型检查基础要求3）
        if (expr1->getOperand() == nullptr)
        {
            fprintf(stderr, "BinaryExpr can't be void type\n");
            assert(expr1->getOperand() != nullptr);
        }
        Instruction *test = bb->rbegin();
        if (!test->isCond() && !test->isUncond())
        {
            int opcode = CmpInstruction::NE;
            Operand *src1 = expr1->getOperand();
            Operand *src2 = src0_const0;
            SymbolEntry *tse = new TemporarySymbolEntry(TypeSystem::boolType, SymbolTable::getLabel());
            Operand *dst = new Operand(tse);
            Operand *n1 = src1;
            // bool->int转换是通过零扩展指令实现（类型检查基础要求2）
            if (src1->getType() == TypeSystem::boolType)
            {
                SymbolEntry *s = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
                n1 = new Operand(s);
                new ZextInstruction(n1, src1, bb);
            }
            if(src1->getType() == TypeSystem::floatType){
                src2 = src0_const0_float;
            }
            new CmpInstruction(opcode, dst, n1, src2, bb);                      
            //等待回填  
            Instruction *temp = new CondBrInstruction(nullptr, nullptr, dst, bb);
            expr1->trueList().push_back(temp);
            expr1->falseList().push_back(temp);
        }

        BasicBlock *ntrueBB = new BasicBlock(func);

        //与and操作区别： 修正 expr1 的假跳转列表中的跳转目标为新创建的 ntrueBB。
        backPatch(expr1->falseList(), ntrueBB, false);

        builder->setInsertBB(ntrueBB);

        expr2->genCode();
        if (expr2->getOperand() == nullptr)
        {
            fprintf(stderr, "BinaryExpr can't be void type\n");
            assert(expr2->getOperand() != nullptr);
        }
        bb = builder->getInsertBB();
        test = bb->rbegin();
        if (!test->isCond() && !test->isUncond())
        {
            int opcode = CmpInstruction::NE;
            Operand *src1 = expr2->getOperand();
            Operand *src2 = src0_const0;
            SymbolEntry *tse = new TemporarySymbolEntry(TypeSystem::boolType, SymbolTable::getLabel());
            Operand *dst = new Operand(tse);
            Operand *n1 = src1;
            // bool->int转换是通过零扩展指令实现（类型检查基础要求2）
            if (src1->getType() == TypeSystem::boolType)
            {
                SymbolEntry *s = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
                n1 = new Operand(s);
                new ZextInstruction(n1, src1, bb);
            }
            if(src1->getType() == TypeSystem::floatType){
                src2 = src0_const0_float;
            }
            new CmpInstruction(opcode, dst, n1, src2, bb);
            //等待回填
            Instruction *temp = new CondBrInstruction(nullptr, nullptr, dst, bb);
            expr2->trueList().push_back(temp);
            expr2->falseList().push_back(temp);
        }

        //与and主要区别所在：对于 OR 操作，整体的假列表是 expr2 的假列表，真列表是 expr1 和 expr2 的真列表的合并。
        false_list = expr2->falseList();
        true_list = merge(expr1->trueList(), expr2->trueList());
    }
    //处理关系判断
    else if (op >= LESS && op <= NOTEQL) // LESS, MORE, LEQ, MEQ, EQ, NE
    {
        expr1->genCode();
        expr2->genCode();
        //检查操作数不能是空，即void类型（类型检查基础要求3）
        if (expr1->getOperand() == nullptr || expr2->getOperand() == nullptr)
        {
            fprintf(stderr, "BinaryExpr can't be void type\n");
            assert(expr1->getOperand() != nullptr);
            assert(expr2->getOperand() != nullptr);
        }
        //通过getOperand函数得到子表达式的目的操作数
        Operand *src1 = typeCast(maxType, expr1->getOperand());
        Operand *src2 = typeCast(maxType, expr2->getOperand());
        
        int opcode = -1;
        switch (op)
        {
        case LESS:
            opcode = CmpInstruction::L;
            break;
        case GREATER:
            opcode = CmpInstruction::G;
            break;
        case LESSEQL:
            opcode = CmpInstruction::LE;
            break;
        case GREATEREQL:
            opcode = CmpInstruction::GE;
            break;
        case EQL:
            opcode = CmpInstruction::E;
            break;
        case NOTEQL:
            opcode = CmpInstruction::NE;
            break;
        }
        SymbolEntry *tse = new TemporarySymbolEntry(TypeSystem::boolType, SymbolTable::getLabel());
        dst = new Operand(tse);
        new CmpInstruction(opcode, dst, src1, src2, bb);
    }
    //处理加减乘除膜
    else if (op >= ADD && op <= MOD)
    {
        expr1->genCode();
        expr2->genCode();
        //检查操作数不能是空，即void类型（类型检查基础要求3）
        if (expr1->getOperand() == nullptr || expr2->getOperand() == nullptr)
        {
            fprintf(stderr, "BinaryExpr can't be void type\n");
            assert(expr1->getOperand() != nullptr);
            assert(expr2->getOperand() != nullptr);
        }
        Operand *src1 = typeCast(maxType, expr1->getOperand());
        Operand *src2 = typeCast(maxType, expr2->getOperand());
        int opcode = -1;
        switch (op)
        {
        case ADD:
            opcode = BinaryInstruction::ADD;
            break;
        case SUB:
            opcode = BinaryInstruction::SUB;
            break;
        case MUL:
            opcode = BinaryInstruction::MUL;
            break;
        case DIV:
            opcode = BinaryInstruction::DIV;
            break;
        case MOD:
            opcode = BinaryInstruction::MOD;
            break;
        }      
        new BinaryInstruction(opcode, dst, src1, src2, bb);
    }
}

void Constant::genCode()
{
    // we don't need to generate code.
}

void Id::genCode()
{
    auto now_dst = dst;
    IdentifierSymbolEntry* se = (IdentifierSymbolEntry*)symbolEntry;
    Operand* addr = se->getAddr();

    if(se->getType()->isArray()) {
        std::vector<Operand *> offs;
        ExprNode *tmp = index;
        while (tmp)
        {
            tmp->genCode();
            offs.push_back((builder->isDuplicate() ? tmp->getDupOperand() : tmp->getOperand()));
            tmp = (ExprNode *)tmp->getNext();
        }
        if (this->isPointer)
        {
            //数组作为函数参数传递指针，生成一条gep指令就OK了
            offs.push_back(new Operand(new ConstantSymbolEntry(TypeSystem::intType, 0)));
            new GepInstruction(now_dst, addr, offs, builder->getInsertBB());
            return;
        }
        if (((ArrayType *)se->getType())->getBaseType()->isInt())
            addr = new Operand(new TemporarySymbolEntry(new PointerType(TypeSystem::intType), SymbolTable::getLabel()));
        else if (((ArrayType *)se->getType())->getBaseType()->isFloat())
            addr = new Operand(new TemporarySymbolEntry(new PointerType(TypeSystem::floatType), SymbolTable::getLabel()));
        
        new GepInstruction(addr, se->getAddr(), offs, builder->getInsertBB());
    }
    else if(se->getType()->isPTR()) {
        ExprNode *tmp = index;
        if (tmp == nullptr)
        {
            //没有索引 说明应该也是作为参数传递的，取数组指针
            new LoadInstruction(now_dst, addr, builder->getInsertBB());
            return;
        }
        Operand *base = new Operand(new TemporarySymbolEntry(((PointerType *)(addr->getType()))->getValType(), SymbolTable::getLabel()));
        new LoadInstruction(base, addr, builder->getInsertBB());
        std::vector<Operand *> offs;
        while (tmp)
        {
            tmp->genCode();
            offs.push_back((builder->isDuplicate() ? tmp->getDupOperand() : tmp->getOperand()));
            tmp = (ExprNode *)tmp->getNext();
        }
        if (((ArrayType *)((PointerType *)se->getType())->getValType())->getBaseType()->isInt())
            addr = new Operand(new TemporarySymbolEntry(new PointerType(TypeSystem::intType), SymbolTable::getLabel()));
        else if (((ArrayType *)((PointerType *)se->getType())->getValType())->getBaseType()->isFloat())
            addr = new Operand(new TemporarySymbolEntry(new PointerType(TypeSystem::floatType), SymbolTable::getLabel()));
        new GepInstruction(addr, base, offs, builder->getInsertBB(), true);
    }
    new LoadInstruction(now_dst, addr, builder->getInsertBB());

}

void IfStmt::genCode()
{
    fprintf(stderr, "IfStmt::genCode\n");
    Function *func = builder->getInsertBB()->getParent();
    //创建两个新的基本块：一个用于条件为真的BB，另一个用于语句结束后的BB。
    BasicBlock *then_bb = new BasicBlock(func);
    BasicBlock *end_bb = new BasicBlock(func);

    cond->genCode();//生成条件表达式cond的IR
    //获取当前基本块和其的最后一条指令。
    BasicBlock *bb = builder->getInsertBB();
    Instruction *test = bb->rbegin();
    //检查最后一条指令是否是条件指令
    if (!test->isCond() && !test->isUncond())
    {
        Operand *src1 = cond->getOperand();
        Operand *n = src1;
        //int->bool的转换，通过这个整数和0比较来转换（类型检查基础要求2）
        if (src1->getType() == TypeSystem::intType)
        {
            SymbolEntry *tse = new TemporarySymbolEntry(TypeSystem::boolType, SymbolTable::getLabel());
            Operand *dst = new Operand(tse);
            new CmpInstruction(CmpInstruction::NE, dst, n, src0_const0, bb);
            n = dst;
        }
        // float->bool的转换，通过这个浮点数和0.0比较来转换（类型检查进阶要求3）
        else if (src1->getType() == TypeSystem::floatType)
        {
            SymbolEntry *tse = new TemporarySymbolEntry(TypeSystem::boolType, SymbolTable::getLabel());
            Operand *dst = new Operand(tse);
            new CmpInstruction(CmpInstruction::NE, dst, n, src0_const0_float, bb);
            n = dst;
        }
        //创建一个条件分支指令，暂不设置具体的跳转目标，并将该指令加入到条件表达式的真假列表
        //以便后续根据条件表达式的值确定跳转的具体目标
        //等待回填
        Instruction *temp = new CondBrInstruction(nullptr, nullptr, n, bb);
        cond->trueList().push_back(temp);
        cond->falseList().push_back(temp);
    }

    //回填操作，用于确定真假分支的目标指令列表
    backPatch(cond->trueList(), then_bb, true);
    backPatch(cond->falseList(), end_bb, false);

    //将插入点设置为then_bb
    builder->setInsertBB(then_bb);
    //如果存在条件为真时执行的语句
    if (thenStmt != nullptr)
        thenStmt->genCode();
    //更新then_bb为当前插入点，在then_bb结束处添加一个无条件跳转到end_bb的指令。
    then_bb = builder->getInsertBB();
    new UncondBrInstruction(end_bb, then_bb);
    //将插入点设置为end_bb
    builder->setInsertBB(end_bb);
}

//类似if的处理，只不过多了一个对else部分的处理
void IfElseStmt::genCode()
{
    fprintf(stderr, "IfElseStmt::genCode\n");
    Function *func;
    //相对于if多了一个条件为假的elsebb基本块
    BasicBlock *then_bb, *else_bb, *end_bb;
    func = builder->getInsertBB()->getParent();
    then_bb = new BasicBlock(func);
    else_bb = new BasicBlock(func);
    end_bb = new BasicBlock(func);

    cond->genCode();

    BasicBlock *bb = builder->getInsertBB();
    Instruction *test = bb->rbegin();
    if (!test->isCond() && !test->isUncond())
    {
        Operand *src1 = cond->getOperand();
        Operand *n = src1;
        //int->bool的转换，通过这个整数和0比较来转换（类型检查基础要求2）
        if (src1->getType() == TypeSystem::intType)
        {
            SymbolEntry *tse = new TemporarySymbolEntry(TypeSystem::boolType, SymbolTable::getLabel());
            Operand *dst = new Operand(tse);
            new CmpInstruction(CmpInstruction::NE, dst, src1, src0_const0, bb);
            n = dst;
        }
        // float->bool的转换，通过这个浮点数和0.0比较来转换（类型检查进阶要求3）
        else if (src1->getType() == TypeSystem::floatType)
        {
            SymbolEntry *tse = new TemporarySymbolEntry(TypeSystem::boolType, SymbolTable::getLabel());
            Operand *dst = new Operand(tse);
            new CmpInstruction(CmpInstruction::NE, dst, n, src0_const0_float, bb);
            n = dst;
        }        
        //等待回填
        Instruction *temp = new CondBrInstruction(nullptr, nullptr, n, bb);
        cond->trueList().push_back(temp);
        cond->falseList().push_back(temp);
    }
    //[关键**]将条件为真时的跳转目标设置为then_bb，条件为假时的跳转目标设置为else_bb
    backPatch(cond->trueList(), then_bb, true);
    backPatch(cond->falseList(), else_bb, false);

    //设置插入点为then_bb，生成条件为真时的代码
    builder->setInsertBB(then_bb);
    if (thenStmt != nullptr)
        thenStmt->genCode();
    then_bb = builder->getInsertBB();
    //在then_bb末尾添加一个无条件跳转到end_bb
    new UncondBrInstruction(end_bb, then_bb);

    //将插入点设置为else_bb，生成条件为假时的代码
    builder->setInsertBB(else_bb);
    if (elseStmt != nullptr)
        elseStmt->genCode();
    else_bb = builder->getInsertBB();
    //在else_bb末尾添加一个无条件跳转到end_bb
    new UncondBrInstruction(end_bb, else_bb);
    //将插入点设置为end_bb
    builder->setInsertBB(end_bb);
}

void CompoundStmt::genCode()
{
    fprintf(stderr, "CompoundStmt::genCode\n");
    if (stmt != nullptr)
        stmt->genCode();
}

void SeqNode::genCode()
{
    fprintf(stderr, "SeqNode::genCode\n");
    stmt1->genCode();
    if (stmt2 != nullptr)
        stmt2->genCode();
}

// 函数参数/变量/常量声明
void DeclStmt::genCode()
{
    fprintf(stderr, "DeclStmt::genCode\n");
    //获取对应的标识符表项
    IdentifierSymbolEntry *se = dynamic_cast<IdentifierSymbolEntry *>(id->getSymPtr());
    //全局常变量
    if (se->isGlobal())//如果是一个全局
    {
        SymbolEntry *addr_se = new IdentifierSymbolEntry(*se);
        addr_se->setType(new PointerType(se->getType()));
        Operand *addr = new Operand(addr_se);
        se->setAddr(addr);//将对应的地址存在se中
        Instruction *globalinstr;
        unit.addGlobalVar(se);
        if (expr != nullptr)//如果存在初始化表达式
        {
            double globalvalue = expr->getValue();
            if(se->getType()->isInt()){
                Operand *useforglobal = (new Constant(new ConstantSymbolEntry(TypeSystem::intType, globalvalue)))->getOperand();
                globalinstr = new GlobalInstruction(new Operand(id->getSymPtr()), useforglobal, se);    
            }
            else if(se->getType()->isFloat()){
                Operand *useforglobal = (new Constant(new ConstantSymbolEntry(TypeSystem::floatType, globalvalue)))->getOperand();
                globalinstr = new GlobalInstruction(new Operand(id->getSymPtr()), useforglobal, se);   
            }
        }
        else
        {
            globalinstr = new GlobalInstruction(new Operand(id->getSymPtr()), nullptr, se);
        }
        global.push_back(globalinstr);//全局指令会添加到这个vector中
        if (se->getType()->isArray()) //如果全局变量为数组
        {
            if (exprArray == nullptr)
            {
                se->setArrayValue(nullptr);
            }
            else
            {
                global_arr_ids.push(new Operand(id->getSymPtr()));
                int size = se->getType()->getSize() / TypeSystem::intType->getSize(); //数组中元素的个数
                double* new_arrayValue = new double[size]{}; //存储每一个元素对应的初值（初始值为0）
                se->setArrayValue(new_arrayValue);
                for (int i = 0; i < size; i++)
                {
                    if (exprArray[i])
                        new_arrayValue[i] = exprArray[i]->getValue();
                }
                arrayValue.push(new_arrayValue);
            }
        }
    }
    else if (se->isLocal() || se->isParam())//如果是一个局部的或者参数
    {
        //获取当前所属函数和入口基本块
        Function *func = builder->getInsertBB()->getParent();
        BasicBlock *entry = func->getEntry();

        Type *type = new PointerType(se->getType());//其是类型基于se的类型的一个指针
        SymbolEntry *addr_se = new TemporarySymbolEntry(type, SymbolTable::getLabel());//表示符号表中刚创建的指针类型的地址
        Operand *addr = new Operand(addr_se);//表示刚刚号表项创建的临时符

        Instruction *alloca = new AllocaInstruction(addr, se); //在函数的堆栈上分配空间，用于存储se代表的局部变量，addr作为这个指令的操作数，表示分配的地址
        entry->insertFront(alloca,se->getType()->isArray());//刚刚创建的分配指令插入到函数的入口基本块的前面
        se->setAddr(addr); //addr存储在符号表条目se中。在后续的代码生成中，需要引用这个局部变量时，就可以使用这个地址

        if (se->isParam())//如果是参数
        {
            //把参数的值存在栈中
            type = se->getType();
            //SymbolEntry *s = new TemporarySymbolEntry(type, SymbolTable::getLabel());
            //Operand *src = new Operand(s);
            BasicBlock *bb = builder->getInsertBB();//获取当前的基本块
            new StoreInstruction(addr, se->getArgAddr(), bb);//将src（即参数的值）存储到addr所指向的地址
            func->pushParamsOperand(se->getArgAddr());//同时把参数值存到操作数这个vector中
        }

        if (expr != nullptr)//如果有初始化表达式
        {
            //将src（初始化表达式的值）存储到addr所指向的地址（即变量的分配空间）。这个指令被添加到基本块bb中。
            expr->genCode();
            BasicBlock *bb = builder->getInsertBB();
            Operand *src = typeCast(se->getType(),expr->getOperand());
            new StoreInstruction(addr, src, bb);
        }

        if(se->getType()->isArray() && exprArray) {
            //获取数组元素的类型
            Type *eleType = ((ArrayType *)se->getType())->getBaseType();
            Type *baseType = eleType->isFloat() ? TypeSystem::floatType : TypeSystem::intType;

            //获取数组元素的下标
            std::vector<int> indexs = ((ArrayType *)se->getType())->getIndexs();
            //计算数组元素的个数
            int size = se->getType()->getSize() / TypeSystem::intType->getSize();
            //将偏移量初始化为0
            std::vector<Operand *> offs;
            for (size_t j = 0; j < indexs.size(); j++)
            {
                offs.push_back(new Operand(new ConstantSymbolEntry(TypeSystem::intType, 0)));
            }
            //先用GEP指令获取数组的第一个元素
            indexs = ((ArrayType *)se->getType())->getIndexs();
            Operand *ele_addr = new Operand(new TemporarySymbolEntry(new PointerType(new ArrayType({}, baseType)), SymbolTable::getLabel()));
            new GepInstruction(ele_addr, se->getAddr(), offs, builder->getInsertBB());
            Operand *srcforfirstzero = typeCast(baseType, src0_const0);
            //为第一个元素赋初值
            if (exprArray[0])
            {
                exprArray[0]->genCode();
                Operand *srcforfirst = typeCast(baseType, exprArray[0]->getOperand());
                new StoreInstruction(ele_addr, srcforfirst, builder->getInsertBB());
            }
            else {
                new StoreInstruction(ele_addr, srcforfirstzero, builder->getInsertBB());
            }

            //为后面的元素赋初值
            auto step = 1ull;
            for (int i = 1; i < size; i++)
            {
                if (exprArray[i]) //如果这个元素在程序中有初值，则用程序给出的值进行赋值
                {
                    Operand *next_addr = new Operand(new TemporarySymbolEntry(new PointerType(new ArrayType({}, baseType)), SymbolTable::getLabel()));
                    new GepInstruction(next_addr, ele_addr, {new Operand(new ConstantSymbolEntry(TypeSystem::intType, step))}, builder->getInsertBB(), true);
                    step = 1;
                    ele_addr = next_addr;
                    exprArray[i]->genCode();
                    Operand *srcforafter = typeCast(baseType, exprArray[i]->getOperand());
                    new StoreInstruction(ele_addr, srcforafter, builder->getInsertBB());
                }
                else //如果这个元素在程序中没有初值，则初值赋为0
                {
                    Operand *next_addr = new Operand(new TemporarySymbolEntry(new PointerType(new ArrayType({}, baseType)), SymbolTable::getLabel()));
                    new GepInstruction(next_addr, ele_addr, {new Operand(new ConstantSymbolEntry(TypeSystem::intType, step))}, builder->getInsertBB(), true);
                    step = 1;
                    ele_addr = next_addr;
                    new StoreInstruction(ele_addr, srcforfirstzero, builder->getInsertBB());
                }
            }
        }
    }
    //可以递归的处理后面的声明
    if (this->getNext())
    {
        this->getNext()->genCode();
    }
}

void DeclStmt::setInitArray(ExprNode **exprArray)
{
    // 能走到这一步，id就是个数组
    this->exprArray = exprArray;
}


void ReturnStmt::genCode()
{
    fprintf(stderr, "ReturnStmt::genCode\n");
    BasicBlock *bb = builder->getInsertBB();
    Operand *src = nullptr;
    this->retType = returntype;
    if (retValue != nullptr)
    {
        retValue->genCode();
        src = typeCast(this->retType, retValue->getOperand());
    }
    new RetInstruction(src, bb);
}

void AssignStmt::genCode()
{
    fprintf(stderr, "AssignStmt::genCode\n");
    //BasicBlock *bb = builder->getInsertBB();
    expr->genCode();
    //这个地址是用来确定赋值操作的目标位置
    IdentifierSymbolEntry *se = (IdentifierSymbolEntry *)lval->getSymPtr();
    Operand *addr = dynamic_cast<IdentifierSymbolEntry*>(lval->getSymPtr())->getAddr();
    Type* targetType = dynamic_cast<PointerType*>(addr->getType())->getBaseType();
    //fprintf(stderr, "1234%s",targetType->toStr().c_str());
    if (targetType->toStr().find("float") != std::string::npos) {
        targetType = TypeSystem::floatType;
    }   
    Operand *src = typeCast(targetType, expr->getOperand());

    /***
     * We haven't implemented array yet, the lval can only be ID. So we just store the result of the `expr` to the addr of the id.
     * If you want to implement array, you have to caculate the address first and then store the result into it.
     */
    if (se->getType()->isArray())
    {
        // 先算地址
        ExprNode *index = ((Id *)lval)->getIndex();
        std::vector<Operand *> offs;
        while (index)
        {
            index->genCode();
            offs.push_back(index->getOperand());
            index = (ExprNode *)index->getNext();
        }
        if (((ArrayType *)se->getType())->getBaseType()->isInt())
            addr = new Operand(new TemporarySymbolEntry(new PointerType(TypeSystem::intType), SymbolTable::getLabel()));
        else if (((ArrayType *)se->getType())->getBaseType()->isFloat())
            addr = new Operand(new TemporarySymbolEntry(new PointerType(TypeSystem::floatType), SymbolTable::getLabel()));
        new GepInstruction(addr, se->getAddr(), offs, now_bb);
    }
    else if (se->getType()->isPTR())
    {
        Operand *base = new Operand(new TemporarySymbolEntry(((PointerType *)(addr->getType()))->getValType(), SymbolTable::getLabel()));
        new LoadInstruction(base, addr, now_bb);
        ExprNode *tmp = ((Id *)lval)->getIndex();
        std::vector<Operand *> offs;
        while (tmp)
        {
            tmp->genCode();
            offs.push_back(tmp->getOperand());
            tmp = (ExprNode *)tmp->getNext();
        }
        if (((ArrayType *)(((PointerType *)se->getType())->getValType()))->getBaseType()->isInt())
            addr = new Operand(new TemporarySymbolEntry(new PointerType(TypeSystem::intType), SymbolTable::getLabel()));
        if (((ArrayType *)(((PointerType *)se->getType())->getValType()))->getBaseType()->isFloat())
            addr = new Operand(new TemporarySymbolEntry(new PointerType(TypeSystem::floatType), SymbolTable::getLabel()));
        new GepInstruction(addr, base, offs, now_bb, true);
    }
    new StoreInstruction(addr, src, now_bb);
}

// 一元运算表达式代码生成
void UnaryExpr::genCode()
{
    fprintf(stderr, "UnaryExpr::genCode\n");
    //检查操作数不为空
    if (expr->getOperand() == nullptr)
    {
        fprintf(stderr, "UnaryExpr can't be void type\n");
        assert(expr->getOperand() != nullptr);
    }

    BasicBlock *bb = builder->getInsertBB();
    expr->genCode();
    int opcode = -1;
    
    if (op == PLUS || op == MINUS)
    {
        switch (op)
        {
            case PLUS:
                opcode = BinaryInstruction::ADD;
                break;
            case MINUS:
                opcode = BinaryInstruction::SUB;
                break;
            default:
                break;
        }
        //Operand *src = expr->getOperand();
        dst = new Operand(symbolEntry);
        Operand *src;
        //Operand *n = src;
        //bool->int隐式类型转换，利用零扩展指令（类型检查基础要求2）
        // if (src->getType() == TypeSystem::boolType)
        // {
        //     SymbolEntry *s = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        //     n = new Operand(s);
        //     new ZextInstruction(n, src, bb);
        // }
        if(expr->getSymPtr()->getType()->isBool()) {
            src = typeCast(TypeSystem::intType, expr->getOperand());
            new BinaryInstruction(opcode, dst, src0_const0, src, bb);
        }
        else if(expr->getSymPtr()->getType()->isInt()){
            src = typeCast(TypeSystem::intType, expr->getOperand());
            new BinaryInstruction(opcode, dst, src0_const0, src, bb);
        }
        else if(expr->getSymPtr()->getType()->isFloat()) {
            src = typeCast(TypeSystem::floatType, expr->getOperand());
            new BinaryInstruction(opcode, dst, src0_const0_float, src, bb);
        }
    }
    else if (op == NOT)
    {
        Operand *src=expr->getOperand();
        //如果操作数是bool，则直接取非即可
        if (expr->getOperand()->getType() == TypeSystem::boolType)
        {
            new NotInstruction(this->dst, src, bb);
        }       

        else{
            src = typeCast(TypeSystem::intType, expr->getOperand());;
            new CmpInstruction(CmpInstruction::E, this->dst, src0_const0, src, bb);
        }
    }
}

void ContinueStmt::genCode()
{
    fprintf(stderr, "ContinueStmt::genCode\n");
    //获取当前插入基本块的父函数
    Function *func = builder->getInsertBB()->getParent();
    //获取当前插入的基本块
    BasicBlock *bb = builder->getInsertBB();
    //创建一个无条件跳转指令，将控制流跳转到当前循环语句的条件块
    new UncondBrInstruction(curr_whileStmt->getCondBlock(), bb);
    BasicBlock *continue_next_bb = new BasicBlock(func);
    //即将代码生成器的插入点移动到新创建的基本块continue_next_bb
    builder->setInsertBB(continue_next_bb);
}

void BreakStmt::genCode()
{
    fprintf(stderr, "BreakStmt::genCode\n");
    //获取当前插入基本块的父函数
    Function *func = builder->getInsertBB()->getParent();
    //获取当前插入的基本块
    BasicBlock *bb = builder->getInsertBB();
    //创建一个无条件跳转指令，将控制流跳转到当前循环语句的结束块
    new UncondBrInstruction(curr_whileStmt->getEndBlock(), bb);
    BasicBlock *break_next_bb = new BasicBlock(func);
    //即将代码生成器的插入点移动到新创建的基本块break_next_bb
    builder->setInsertBB(break_next_bb);
}
void WhileStmt::genCode()
{
    curr_whileStmt = this;//设置当前的WhileStmt指针，用于在BreakStmt::genCode()和ContinueStmt::genCode()中获取当前while循环的条件块和结束块
    fprintf(stderr, "WhileStmt::genCode\n");
    BasicBlock *cond_bb ,*loop_bb, *end_bb = nullptr;

    BasicBlock *bb = builder->getInsertBB();
    Function *func = bb->getParent();
    cond_bb = new BasicBlock(func);
    loop_bb = new BasicBlock(func);
    end_bb = new BasicBlock(func);

    //为WhileStmt中的condb和endb成员赋值
    this->condb = cond_bb;
    this->endb = end_bb;

    //将当前的插入基本块设置为cond_bb，之后生成条件表达式的代码
    builder->setInsertBB(cond_bb);
    cond->genCode();
    new UncondBrInstruction(cond_bb, bb);

    //将bb设置为当前插入基本块
    bb = builder->getInsertBB();
    //获取bb中最后一条指令的指针
    Instruction *test = bb->rbegin();
    //如果最后一条指令不是条件跳转指令或无条件跳转指令
    if (!test->isCond() && !test->isUncond())
    {
        //获取条件表达式的操作数
        Operand *src1 = cond->getOperand();
        Operand *n = src1;
        if (src1->getType() == TypeSystem::intType)
        {
            //比较src1是否为0，并将结果存在n中
            int opcode = CmpInstruction::NE;
            Operand *src2 = src0_const0;
            SymbolEntry *tse = new TemporarySymbolEntry(TypeSystem::boolType, SymbolTable::getLabel());
            Operand *dst = new Operand(tse);
            new CmpInstruction(opcode, dst, src1, src2, bb);
            n = dst;
        }
        //等待回填
        Instruction *temp = new CondBrInstruction(nullptr, nullptr, n, bb);
        cond->trueList().push_back(temp);
        cond->falseList().push_back(temp);
    }

    backPatch(cond->trueList(), loop_bb, true);//回填条件表达式的true列表，跳转到loop_bb
    backPatch(cond->falseList(), end_bb, false);//回填条件表达式的false列表，跳转到end_bb

    builder->setInsertBB(loop_bb);//将当前的插入基本块设置为loop_bb
    if (thenStmt != nullptr)//如果存在循环体
        thenStmt->genCode();
    loop_bb = builder->getInsertBB();//将loop_bb设置为当前插入基本块
    new UncondBrInstruction(cond_bb, loop_bb);//创建一个无条件跳转指令，跳转到cond_bb

    builder->setInsertBB(end_bb);//将当前的插入基本块设置为end_bb

}

void NullStmt::genCode()
{
}

void Ast::typeCheck()
{
    if(root != nullptr)
        root->typeCheck();
}

void FuncCall::typeCheck()
{
}

void BinaryExpr::typeCheck()
{
    expr1->typeCheck();
    expr2->typeCheck();
}

void Constant::typeCheck()
{
}

void Id::typeCheck()
{
}

void IfStmt::typeCheck()
{
    cond->typeCheck();
    if (thenStmt)
        thenStmt->typeCheck();
}

void IfElseStmt::typeCheck()
{
    cond->typeCheck();
    if (thenStmt)
        thenStmt->typeCheck();
    if (elseStmt)
        elseStmt->typeCheck();
}

void CompoundStmt::typeCheck()
{
    if (stmt)
        stmt->typeCheck();
}

void SeqNode::typeCheck()
{
    if (stmt1)
        stmt1->typeCheck();
    if (stmt2)
        stmt2->typeCheck();
}

void ExprStmt::typeCheck()
{
    if (expr)
        expr->typeCheck();
}

void DeclStmt::typeCheck()
{
    id->typeCheck();
    if (expr)
        expr->typeCheck();
}

void ReturnStmt::typeCheck()
{
    if (retValue)
        retValue->typeCheck();
}

void AssignStmt::typeCheck()
{
    lval->typeCheck();
    expr->typeCheck();
}

void NullStmt::typeCheck()
{
}
void BreakStmt::typeCheck()
{
}
void ContinueStmt::typeCheck()
{
}
void UnaryExpr::typeCheck()
{
    expr->typeCheck();
}

void WhileStmt::typeCheck()
{
    cond->typeCheck();
    if (thenStmt)
        thenStmt->typeCheck();
}

void FunctionDef::typeCheck()
{
    //获取到函数声明的那个type，即应该是返回类型的type
    Type *retType = ((FunctionType *)(se->getType()))->getRetType();
    //调用checkRet方法进行检查
    if (stmt->checkRet(retType) == -1)
    {
        if (retType != TypeSystem::voidType){
            fprintf(stderr, "function has NO return statement OR function type doesn't match return type\n");
            assert(false);
        }
    }
    else if(stmt->checkRet(retType) == 0) {
        assert(false);
    }
}

//实现checkRet函数的主要逻辑部分：
//判断return 语句操作数和函数声明的返回值类型是否匹配
int ReturnStmt::checkRet(Type *retType)
{
    //判断没有返回值的情况：可能是有return语句没有返回值；也可能是没有return语句
    if (!retValue)
    {
        //如果返回值类型不是void，则代表有错误（上面两种情况）
        if (retType != TypeSystem::voidType)
        {
            fprintf(stderr, "function has NO return statement OR function type doesn't match return type\n");
            assert(retType == TypeSystem::voidType);
            return 0;
        }
        //如果返回值类型为void，则代表正确
        else
        {
            return 1;
        }
    }
    //如果有返回值，会进一步判断
    else
    {
        //返回值类型
        Type *valueType = retValue->getSymPtr()->getType();

        //同时为int或constint、float或constfloat即正确
        if (((retType == TypeSystem::constIntType || retType == TypeSystem::intType) && (valueType == TypeSystem::constIntType || valueType == TypeSystem::intType)) 
        || ((retType == TypeSystem::constFloatType || retType == TypeSystem::floatType) && (valueType == TypeSystem::constFloatType || valueType == TypeSystem::floatType)))
        {
            return 1;
        }
        //否则进一步判断
        else
        {
            //如果返回值是一个函数类型（函数嵌套），则去看是否匹配，如果匹配则正确
            if ( valueType->isFunc() 
            && (((retType == TypeSystem::constIntType || retType == TypeSystem::intType) && (((FunctionType *)valueType)->getRetType() == TypeSystem::constIntType || ((FunctionType *)valueType)->getRetType() == TypeSystem::intType)) 
            || ((retType == TypeSystem::constFloatType || retType == TypeSystem::floatType) && (((FunctionType *)valueType)->getRetType() == TypeSystem::constFloatType || ((FunctionType *)valueType)->getRetType() == TypeSystem::floatType))))
            {
                return 1;
            }
            if ( valueType->isArray() 
            && (((retType == TypeSystem::constIntType || retType == TypeSystem::intType) && (((ArrayType *)valueType)->getBaseType() == TypeSystem::constIntType || ((ArrayType *)valueType)->getBaseType() == TypeSystem::intType)) 
            || ((retType == TypeSystem::constFloatType || retType == TypeSystem::floatType) && (((ArrayType *)valueType)->getBaseType() == TypeSystem::constFloatType || ((ArrayType *)valueType)->getBaseType() == TypeSystem::floatType))))
            {
                return 1;
            }
            if ( valueType->isPTR() 
            && (((retType == TypeSystem::constIntType || retType == TypeSystem::intType) && (((PointerType *)valueType)->getBaseType() == TypeSystem::constIntType || ((PointerType *)valueType)->getBaseType() == TypeSystem::intType)) 
            || ((retType == TypeSystem::constFloatType || retType == TypeSystem::floatType) && (((PointerType *)valueType)->getBaseType() == TypeSystem::constFloatType || ((PointerType *)valueType)->getBaseType() == TypeSystem::floatType))))
            {
                return 1;
            }
            //如果仍没有判断正确则代表类型没有匹配上，报错
            fprintf(stderr, "function type doesn't match return type\n");
            return 0;
        }
    }
}


void BinaryExpr::output(int level)
{
    std::string op_str;
    switch(op)
    {
        case ADD:
            op_str = "add";
            break;
        case SUB:
            op_str = "sub";
            break;
        case MUL:
            op_str = "mul";
            break;
        case DIV:
            op_str = "div";
            break;
        case MOD:
            op_str = "mod";
            break;
        case AND:
            op_str = "and";
            break;
        case OR:
            op_str = "or";
            break;
        case LESS:
            op_str = "less";
            break;
        case GREATER:
            op_str = "greater";
            break;
        case LESSEQL:
            op_str = "lesseql";
            break;
        case GREATEREQL:
            op_str = "greatereql";
            break;
        case EQL:
            op_str = "eql";
            break;
        case NOTEQL:
            op_str = "noteql";
            break;
    }
    fprintf(yyout, "%*cBinaryExpr\top: %s\n", level, ' ', op_str.c_str());
    expr1->output(level + 4);
    expr2->output(level + 4);
}

void Ast::output()
{
    fprintf(yyout, "program\n");
    if(root != nullptr)
        root->output(4);
}

void Constant::output(int level)
{
    std::string type, value;
    type = symbolEntry->getType()->toStr();
    value = symbolEntry->toStr();
    fprintf(yyout, "%*cIntegerLiteral\tvalue: %s\ttype: %s\n", level, ' ',
            value.c_str(), type.c_str());
}

void CompoundStmt::output(int level)
{
    fprintf(yyout, "%*cCompoundStmt\n", level, ' ');
    if (stmt)
        stmt->output(level + 4);
}

void SeqNode::output(int level)
{
    stmt1->output(level);
    stmt2->output(level);
}

void DeclStmt::output(int level)
{
    fprintf(yyout, "%*cDeclStmt\n", level, ' ');
    id->output(level + 4);
    if (expr)
        expr->output(level + 4);
    if (this->getNext())
    {
        this->getNext()->output(level);
    }
}

void IfStmt::output(int level)
{
    fprintf(yyout, "%*cIfStmt\n", level, ' ');
    cond->output(level + 4);
    thenStmt->output(level + 4);
}

void IfElseStmt::output(int level)
{
    fprintf(yyout, "%*cIfElseStmt\n", level, ' ');
    cond->output(level + 4);
    thenStmt->output(level + 4);
    elseStmt->output(level + 4);
}

void ReturnStmt::output(int level)
{
    fprintf(yyout, "%*cReturnStmt\n", level, ' ');
    retValue->output(level + 4);
}

void AssignStmt::output(int level)
{
    fprintf(yyout, "%*cAssignStmt\n", level, ' ');
    lval->output(level + 4);
    expr->output(level + 4);
}

void FunctionDef::output(int level)
{
    std::string name= se->toStr();
    std::string type= se->getType()->toStr();
    fprintf(yyout, "%*cFunctionDefine function name: %s, type: %s\n", level, ' ', 
            name.c_str(), type.c_str());
    if (FuncDefParams)
    {
        FuncDefParams->output(level + 4);
    }    
    stmt->output(level + 4);
}

void UnaryExpr::output(int level)
{
    std::string op_str;
    switch(op)
    {
        case MINUS:
            op_str = "minus";
            break;
        case NOT:
            op_str = "not";
            break;
    }
    fprintf(yyout, "%*cUnaryExpr\top: %s\n", level, ' ', op_str.c_str());
    expr->output(level + 4);
}


void Id::output(int level)
{
    std::string name, type;
    int scope;
    name = symbolEntry->toStr();
    type = symbolEntry->getType()->toStr();
    scope = dynamic_cast<IdentifierSymbolEntry*>(symbolEntry)->getScope();
    fprintf(yyout, "%*cId\tname: %s\tscope: %d\ttype: %s\n", level, ' ',
            name.c_str(), scope, type.c_str());
}

void NullStmt::output(int level)
{
    fprintf(yyout, "%*cNullStmt\n", level, ' ');
}


void WhileStmt::output(int level)
{
    fprintf(yyout, "%*cWhileStmt\n", level, ' ');
    cond->output(level + 4);
    thenStmt->output(level + 4);
}

void BreakStmt::output(int level)
{
    fprintf(yyout, "%*cBreakStmt\n", level, ' ');
}

void ContinueStmt::output(int level)
{
    fprintf(yyout, "%*cContinueStmt\n", level, ' ');
}

void ExprStmt::output(int level)
{
    fprintf(yyout, "%*cExprStmt\n", level, ' ');
    expr->output(level + 4);
}



/****************************函数begin*******************************************/
void FunctionDef::genCode()
{
    Unit *unit = builder->getUnit();
    Function *func = new Function(unit, se);
    BasicBlock *entry = func->getEntry();
    builder->setInsertBB(entry);
    returntype = ((FunctionType *)(se->getType()))->getRetType();
    //fprintf(stderr, "%s\n", se->toStr().c_str());
    // 检查是否是main函数
    if (se->toStr() == "@main") {
        while(!global_arr_ids.empty() && !arrayValue.empty()) {
            double* curr_arrayValue = arrayValue.front();
            Operand* currOp = global_arr_ids.front();
            IdentifierSymbolEntry *curr_se = dynamic_cast<IdentifierSymbolEntry *>(currOp->getSymbolEntry());

            Type *eleType = ((ArrayType *)curr_se->getType())->getBaseType();
            Type *baseType = eleType->isFloat() ? TypeSystem::floatType : TypeSystem::intType;
            std::vector<int> indexs = ((ArrayType *)curr_se->getType())->getIndexs();
            int size = curr_se->getType()->getSize() / TypeSystem::intType->getSize();
            std::vector<Operand *> offs;
            for (size_t j = 0; j < indexs.size(); j++)
            {
                offs.push_back(new Operand(new ConstantSymbolEntry(TypeSystem::intType, 0)));
            }

            indexs = ((ArrayType *)curr_se->getType())->getIndexs();
            Operand *ele_addr = new Operand(new TemporarySymbolEntry(new PointerType(new ArrayType({}, baseType)), SymbolTable::getLabel()));
            new GepInstruction(ele_addr, curr_se->getAddr(), offs, builder->getInsertBB());
            if (curr_arrayValue[0] != 0)
            {
                new StoreInstruction(ele_addr, new Operand(new ConstantSymbolEntry(eleType,curr_arrayValue[0])), builder->getInsertBB());
            }
            else {
                new StoreInstruction(ele_addr, src0_const0, builder->getInsertBB());
            }
            auto step = 1ull;
            for (int i = 1; i < size; i++)
            {
                if (curr_arrayValue[i] != 0)
                {
                    Operand *next_addr = new Operand(new TemporarySymbolEntry(new PointerType(new ArrayType({}, baseType)), SymbolTable::getLabel()));
                    new GepInstruction(next_addr, ele_addr, {new Operand(new ConstantSymbolEntry(TypeSystem::intType, step))}, builder->getInsertBB(), true);
                    step = 1;
                    ele_addr = next_addr;
                    new StoreInstruction(ele_addr, new Operand(new ConstantSymbolEntry(eleType,curr_arrayValue[i])), builder->getInsertBB());
                }
                else
                {
                    step++;
                }
            }

            global_arr_ids.pop();
            arrayValue.pop();
        }
    }

    //函数参数IR
    if (FuncDefParams)
        FuncDefParams->genCode();
    
    //函数体IR
    if (stmt)
        stmt->genCode();

    //根据基本块的前驱、后继关系进行流图的构造
    //遍历函数的所有基本块，为每个基本块处理控制流逻辑
    for (auto block = func->begin(); block != func->end(); block++)
    {
        //获取BB的第一条和最后一条指令
        Instruction *i = (*block)->begin();
        Instruction *last = (*block)->rbegin();
        //再次循环遍历指令
        while (i != last)
        {
            if (i->isCond() || i->isUncond())
            {
                //移除块中跳转指令
                (*block)->remove(i);
            }
            i = i->getNext();
        }
        //判断最后一条指令的类型并进行相应处理：
        //如果是条件跳转，处理真假分支并建立基本块间的前驱后继关系。
        if (last->isCond())
        {
            BasicBlock *truebranch, *falsebranch;
            truebranch =
                dynamic_cast<CondBrInstruction *>(last)->getTrueBranch();
            falsebranch =
                dynamic_cast<CondBrInstruction *>(last)->getFalseBranch();
            (*block)->addSucc(truebranch);
            (*block)->addSucc(falsebranch);
            truebranch->addPred(*block);
            falsebranch->addPred(*block);
        }
        //如果是无条件跳转，处理跳转目标并建立基本块间的链接
        else if (last->isUncond())
        {
            //获取要跳转的基本块
            BasicBlock *dst =
                dynamic_cast<UncondBrInstruction *>(last)->getBranch();
            //跳转块链接
            (*block)->addSucc(dst);
            dst->addPred(*block);
        }
        //如果不是返回指令
        else if (!last->isRet())
        {
            if (((FunctionType *)(se->getType()))->getRetType() == TypeSystem::voidType)
            {
                new RetInstruction(nullptr, *block);
            }
            else if (((FunctionType *)(se->getType()))->getRetType() == TypeSystem::floatType)
            {
                new RetInstruction(new Operand(new ConstantSymbolEntry(TypeSystem::floatType, 0)), *block);
            }      
            else if (((FunctionType *)(se->getType()))->getRetType() == TypeSystem::intType){
                new RetInstruction(new Operand(new ConstantSymbolEntry(TypeSystem::intType, 0)), *block);
            }                 
        }
    }
    // 把ret后面的指令清掉
    for (auto it = func->begin(); it != func->end(); it++)
    {
        auto block = *it;
        bool flag = false;
        for (auto i = block->begin(); i != block->end(); i = i->getNext())
        {
            if (flag)
            {
                block->remove(i);
                delete i;
                continue;
            }
            flag = i->isRet();
        }
        if (flag)
        {
            while (block->succ_begin() != block->succ_end())
            {
                auto b = *(block->succ_begin());
                block->removeSucc(b);
                b->removePred(block);
            }
        }
    }
    //小优化，移除不可达基本块
    bool removed;
    do {
        removed = false;
        for (auto it = func->begin(); it != func->end(); ++it) {
            auto block = *it;
            if (block != func->getEntry() && block->getNumOfPred() == 0) {
                delete block;
                removed = true;
                break; // 退出 for 循环
            }
        }
    } while (removed); // 如果在本次循环中删除了基本块，则继续循环，直到不可到

}

FuncCall::FuncCall(SymbolEntry *tmp,SymbolEntry *se, ExprNode *params)
    : ExprNode(tmp), params(params)
{
    dst=nullptr;
    //计算实参数量
    long unsigned int actualParamCount = 0;
    for (ExprNode *node = params; node != nullptr; node = static_cast<ExprNode*>(node->getNext())) {
        actualParamCount++;
    }
    Type *type = nullptr;
    //遍历同名的函数列表，找到参数数量对应的函数
    for (SymbolEntry *entry = se; entry != nullptr; entry = entry->getNext()) {
        type = entry->getType();
        if (((FunctionType *)type) && ((FunctionType *)type)->getParamsType().size() == actualParamCount) {
            this->symbolEntry = entry;
            break;
        }
    }
    //如果找到了匹配的函数声明
    if (symbolEntry) {
        this->type = ((FunctionType *)type)->getRetType();
        //如果函数的返回类型不是 void，构造目标寄存器的保存操作数
        if (this->type != TypeSystem::voidType) {
            SymbolEntry *tempSE = new TemporarySymbolEntry(this->type, SymbolTable::getLabel());
            dst = new Operand(tempSE);
        }
        // //检查实参与形参的匹配（类型检查基础要求4）面向case编程ToT（由于有case就是不匹配要你去转换的）
        // std::vector<Type *> declaredParamTypes = ((FunctionType *)type)->getParamsType();//形参列表
        // auto paramIt = declaredParamTypes.begin();
        // ExprNode *paramNode = params;//实参
        // while (paramNode && paramIt != declaredParamTypes.end()) {
        //     //检查形参和实参类型是否匹配，
        //     if ((*paramIt)->getKind() != paramNode->getType()->getKind()) {
        //         fprintf(stderr, "arguments type not match in function %s\n", symbolEntry->toStr().c_str());
        //         assert(false); 
        //     }
        //     paramNode = static_cast<ExprNode*>(paramNode->getNext());
        //     ++paramIt;
        // }
        // if (paramNode || paramIt != declaredParamTypes.end()) {
        //     fprintf(stderr, "actual param numbers %d not match in function %s\n", int(actualParamCount), symbolEntry->toStr().c_str());
        //     assert(false);
        // }
    }
    else{
        fprintf(stderr, "function is undefined\n");
        assert(false);
    }
}

// 函数调用中间代码生成
void FuncCall::genCode()
{
    fprintf(stderr, "FuncCallExpr::genCode\n");
    Type *type = symbolEntry->getType();
    std::vector<Type *> declaredParamTypes = ((FunctionType *)type)->getParamsType();//形参列表
    auto paramIt = declaredParamTypes.begin();
    std::vector<Operand *> operands;
    ExprNode *temp = params;
    //遍历实参生成对应的IR
    while (temp && paramIt != declaredParamTypes.end())
    {
        temp->genCode();
        operands.push_back(typeCast(*paramIt,temp->getOperand()));
        temp = ((ExprNode *)temp->getNext());
        ++paramIt;
    }
    BasicBlock *bb = builder->getInsertBB();
    new CallInstruction(dst, this->symbolEntry, operands, bb);
}

void FuncCall::output(int level){
    std::string name = symbolEntry->toStr();// 获取函数名
    std::string type = symbolEntry->getType()->toStr();// 获取函数返回值类型
    fprintf(yyout, "%*cFuncCall function name: %s\n", level, ' ', 
            name.c_str());
    Node *temp = params;
    while (temp)
    {
        temp->output(level + 4);
        temp = temp->getNext();
    }
}
/****************************函数end*******************************************/

double BinaryExpr::getValue() {
    double value1 = expr1->getValue();
    double value2 = expr2->getValue();
    double value = 0.0;
    if(type->isFloat()) {
        float value1_f = (float)expr1->getValue();
        float value2_f = (float)expr2->getValue();
        float value_f = 0.0;
        switch(op) {
        case ADD:
            value_f = value1_f + value2_f;
            break;
        case SUB:
            value_f = value1_f - value2_f;
            break;
        case MUL:
            value_f = value1_f * value2_f;
            break;
        case DIV:
            value_f = value1_f / value2_f;
            break;
        case AND:
            value_f = value1_f && value2_f;
            break;
        case OR:
            value_f = value1_f || value2_f;
            break;
        case LESS:
            value_f = value1_f < value2_f;
            break;
        case GREATER:
            value_f = value1_f > value2_f;
            break;
        case LESSEQL:
            value_f = value1_f <= value2_f;
            break;
        case GREATEREQL:
            value_f = value1_f >= value2_f;
            break;
        case EQL:
            value_f = value1_f == value2_f;
            break;
        case NOTEQL:
            value_f = value1_f != value2_f;
            break;
        }
        return value_f;
    }
    else {
        switch (op)
        {
        case ADD:
            value = value1 + value2;
            break;
        case SUB:
            value = value1 - value2;
            break;
        case MUL:
            value = value1 * value2;
            break;
        case DIV:
            value = value1 / value2;
            break;
        case MOD:
            value = (int)value1 % (int)value2;
            break;
        case AND:
            value = value1 && value2;
            break;
        case OR:
            value = value1 || value2;
            break;
        case LESS:
            value = value1 < value2;
            break;
        case LESSEQL:
            value = value1 <= value2;
            break;
        case GREATER:
            value = value1 > value2;
            break;
        case GREATEREQL:
            value = value1 >= value2;
            break;
        case EQL:
            value = value1 == value2;
            break;
        case NOTEQL:
            value = value1 != value2;
            break;
        }
        return value;
    }
}

double UnaryExpr::getValue() {
    double value = expr->getValue();
    switch (op)
    {
    case PLUS:
        value = value;
        break;
    case MINUS:
        value = (-value);
        break;
    case NOT:
        value = (!value);
        break;
    }
    return value;
}

double Constant::getValue() {
    return ((ConstantSymbolEntry *)symbolEntry)->getValue();
}

double Id::getValue() {
    return ((IdentifierSymbolEntry *)symbolEntry)->getValue();
}


WhileStmt* curr_whileStmt = nullptr;


