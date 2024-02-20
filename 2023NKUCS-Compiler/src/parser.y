%code top{
    #include <iostream>
    #include <assert.h>
    #include <vector>
    #include <stack>
    #include "parser.h"
    extern Ast ast;
    int yylex();
    int yyerror( char const * );
    Type *curType = nullptr;
    int useforwhilecheck=0;//用于类型检查
    std::vector<int> arr_indices;
    std::vector<int> decl_arr_indices;


    ExprNode **initArray = nullptr; // 这个存储当前数组的初始化数组的基地址
    int idx; // 这个是上边那个initArray的索引
    std::stack<std::vector<int>> dimesionStack; // 维度栈

    int spillPos = 1; // 这个记录当前的参数的溢出位置
    int intArgNum = 0; // 这个记录的是当前函数参数是第几个参数，因为前四个参数用寄存器存，之后的参数用栈传递
    int floatArgNum = 0;
}

%code requires {
    #include "Ast.h"
    #include "SymbolTable.h"
    #include "Type.h"
}

%union {
    int itype;
    float ftype;
    char* strtype;
    StmtNode* stmttype;
    ExprNode* exprtype;
    Type* type;
    SymbolEntry* se;
}

%start Program
%token <strtype> ID 
%token <itype> INTEGER
%token <ftype> FLOATNUMBER
%token IF ELSE WHILE
%token INT VOID FLOAT 
%token LPAREN RPAREN LBRACE RBRACE LBRACKET RBRACKET SEMICOLON
%token ADD SUB MUL DIV MOD
%token LESS GREATER LESSEQL GREATEREQL EQL NOTEQL
%token AND OR NOT
%token ASSIGN
%token RETURN
%token CONST
%token COMMA
%token BREAK CONTINUE

%nterm <stmttype> Stmts Stmt
%nterm <stmttype> AssignStmt BlockStmt IfStmt WhileStmt ReturnStmt ExprStmt NullStmt BreakStmt ContinueStmt DeclStmt
%nterm <stmttype> VarDeclStmt VarDefList VarDef 
%nterm <stmttype> ConstDeclStmt ConstDefList ConstDef 
%nterm <exprtype> FuncRParams  
%nterm <stmttype> FuncDef FuncFParam FuncFParams
%nterm <exprtype> Exp AddExp MULExp Cond LOrExp PrimaryExp LVal RelExp LAndExp UnaryExp EqlExp DeclArrayIndices
%nterm <exprtype> ArrayIndices 
%nterm <exprtype> ConstInitVal ConstInitValList InitVal InitValList
%nterm <type> Type 



%precedence THEN
%precedence ELSE
%%
Program
    : Stmts {
        ast.setRoot($1);
    }
    ;
Stmts
    : Stmt {$$=$1;}
    | Stmts Stmt{
        $$ = new SeqNode($1, $2);
    }
    ;
Stmt
    : AssignStmt {$$=$1;}
    | BlockStmt {$$=$1;}
    | IfStmt {$$=$1;}
    | WhileStmt {$$=$1;}
    | ReturnStmt {$$=$1;}
    | DeclStmt {$$=$1;}
    | FuncDef {$$=$1;}
    | ExprStmt {$$=$1;}
    | NullStmt {$$=$1;}
    | ContinueStmt {$$=$1;}
    | BreakStmt {$$=$1;}
    ;
Type
    : INT {
        $$ = curType = TypeSystem::intType;
    }
    | FLOAT {
        $$ = curType = TypeSystem::floatType;
    }
    | VOID {
        $$ = curType = TypeSystem::voidType;
    }
    ;

ExprStmt
    :
    Exp SEMICOLON {
        $$ = new ExprStmt($1);
    }
    ;
LVal
    : ID {
        SymbolEntry *se = identifiers->lookup($1);

        //变量（常量）使用时未声明（类型检查基础要求1）
        if(se == nullptr)
        {
            fprintf(stderr, "identifier \"%s\" is undefined\n", (char*)$1);
            delete [](char*)$1;
            assert(se != nullptr);
        }

        $$ = new Id(se);
        delete []$1;
    }
    | ID ArrayIndices{  //数组
        SymbolEntry *se = identifiers->lookup($1);

        //变量（常量）使用时未声明（类型检查基础要求1）
        if(se == nullptr)
        {
            fprintf(stderr, "identifier \"%s\" is undefined\n", (char*)$1);
            delete [](char*)$1;
            assert(se != nullptr);
        }

        $$ = new Id(se, $2);
        delete []$1;
    }
    ;

ArrayIndices
    : LBRACKET Exp RBRACKET {
        $$ = $2;
    }
    | ArrayIndices LBRACKET Exp RBRACKET {
        $$ = $1;
        $$->setNext($3);
    }
    ;

AssignStmt
    :
    LVal ASSIGN Exp SEMICOLON {
        $$ = new AssignStmt($1, $3);
    }
    ;
BlockStmt
    :   LBRACE 
        {   
            identifiers = new SymbolTable(identifiers);
        } 
        Stmts RBRACE 
        {
            $$ = new CompoundStmt($3);
            SymbolTable *top = identifiers;
            identifiers = identifiers->getPrev();
            delete top;
        }
    |
    LBRACE RBRACE {
        $$ = new CompoundStmt(new NullStmt());
    }
    ;
NullStmt
    : 
    SEMICOLON {
        $$ = new NullStmt();
    }
    ;
IfStmt
    //%prec THEN的作用是解决悬空-else的二义性问题，他会将终结符THEN的优先级赋给下述产生式
    //这样的话，下述第二个候选式优先级更高（即else优先匹配）
    : IF LPAREN Cond RPAREN Stmt %prec THEN {
        $$ = new IfStmt($3, $5);
    }
    | IF LPAREN Cond RPAREN Stmt ELSE Stmt {
        $$ = new IfElseStmt($3, $5, $7);
    }
    ;
WhileStmt
    :
    WHILE LPAREN Cond RPAREN{
        useforwhilecheck++;
    } Stmt {
        $$ = new WhileStmt($3,$6);
        useforwhilecheck--;
    }
    ;
BreakStmt
    : 
    BREAK SEMICOLON {
        //静态检查：检查break是否在while中（类型检查基础要求6）
        if (!useforwhilecheck) {
            fprintf(stderr, "\"break\" not in whilestmt\n");
            assert(useforwhilecheck);        
        }
        $$ = new BreakStmt();
    }
    ;
ContinueStmt
    : 
    CONTINUE SEMICOLON {
        //静态检查：检查continue是否在while中（类型检查基础要求6）
        if (!useforwhilecheck) {
            fprintf(stderr, "\"continue\" not in whilestmt\n");
            assert(useforwhilecheck);        
        }
        $$ = new ContinueStmt();
    }
    ;
ReturnStmt
    :
    RETURN Exp SEMICOLON{
        //返回值的类型检查会主体在ReturnStmt::checkRet中实现（类型检查基础要求5）
        $$ = new ReturnStmt($2);
    }
    |
    RETURN SEMICOLON {
        $$ = new ReturnStmt(nullptr);
    }
    ;

//********************表达式begin**************************
// 算数表达式
Exp
    :
    AddExp {$$ = $1;}
    ;
// 关系+逻辑运算
Cond
    :
    LOrExp {$$ = $1;}
    ;
// 基本表达式：数字/ID、括号括起来的表达式
PrimaryExp
    :
    LVal {
        $$ = $1;
    }
    | INTEGER {
        SymbolEntry *se = new ConstantSymbolEntry(TypeSystem::intType, $1);
        $$ = new Constant(se);
    }
    | FLOATNUMBER {
        SymbolEntry *se = new ConstantSymbolEntry(TypeSystem::floatType, $1);
        $$ = new Constant(se);
    }
    |
    LPAREN Exp RPAREN {
        $$ = $2;
    }
    ;
FuncRParams //函数调用时的参数，实参
    : Exp {
        $$ = $1;
    }
    | FuncRParams COMMA Exp {
        $$ = $1;
        $$->setNext($3);
    }
    | %empty {
       $$ = nullptr;
    }
    ;
//单目运算(双目算数表达式)
UnaryExp
    :
    PrimaryExp{$$=$1;}
    |
    ADD UnaryExp {$$ = $2;}
    |
    SUB UnaryExp {
        SymbolEntry *se = new TemporarySymbolEntry($2->getType(), SymbolTable::getLabel());
        $$ = new UnaryExpr(se,UnaryExpr::MINUS,$2);
    }
    |
    NOT UnaryExp {
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        $$ = new UnaryExpr(se,UnaryExpr::NOT,$2);
    }
    |
    ID LPAREN FuncRParams RPAREN {
        SymbolEntry *se = identifiers->lookup($1);
        SymbolEntry *tmp = new TemporarySymbolEntry(dynamic_cast<FunctionType*>(se->getType())->getRetType(), SymbolTable::getLabel());
        //检查未声明函数（类型检查基础要求4）
        if(se == nullptr){
            fprintf(stderr, "function identifier \"%s\" is undefined\n", (char*)$1);
            delete [](char*)$1;
            assert(se != nullptr);        
        }
        //如果有参数，FuncCall函数中有检查函数形参是否与实参类型及数目匹配的逻辑（类型检查基础要求4）
        else if($3 != nullptr){
            $$ = new FuncCall(tmp,se, ((ExprNode*)$3));        
        }
        //如果没有参数
        else {
            $$ = new FuncCall(tmp,se, nullptr);
        }
    }
    ;
// 乘除模(双目算数表达式)
MULExp
    :
    UnaryExp {$$ = $1;}
    |
    MULExp MUL UnaryExp {
        SymbolEntry *se ;
        if ($1->getType()->isFloat() || $3->getType()->isFloat()){
            se = new TemporarySymbolEntry(TypeSystem::floatType, SymbolTable::getLabel());
        }
        else{
            se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        }
        $$ = new BinaryExpr(se, BinaryExpr::MUL, $1, $3);
    }
    |
    MULExp DIV UnaryExp {
        SymbolEntry *se ;
        if ($1->getType()->isFloat() || $3->getType()->isFloat()){
            se = new TemporarySymbolEntry(TypeSystem::floatType, SymbolTable::getLabel());
        }
        else{
            se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        }        
        $$ = new BinaryExpr(se, BinaryExpr::DIV, $1, $3);
    }
    |
    MULExp MOD UnaryExp {
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        $$ = new BinaryExpr(se, BinaryExpr::MOD, $1, $3);
    }
    ;
// 加减法(双目算数表达式)
AddExp
    :
    MULExp {$$ = $1;}
    |
    AddExp ADD MULExp
    {
        SymbolEntry *se ;
        if ($1->getType()->isFloat() || $3->getType()->isFloat()){
            se = new TemporarySymbolEntry(TypeSystem::floatType, SymbolTable::getLabel());
        }
        else{
            se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        }
        $$ = new BinaryExpr(se, BinaryExpr::ADD, $1, $3);
    }
    |
    AddExp SUB MULExp
    {
        SymbolEntry *se ;
        if ($1->getType()->isFloat() || $3->getType()->isFloat()){
            se = new TemporarySymbolEntry(TypeSystem::floatType, SymbolTable::getLabel());
        }
        else{
            se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        }    
        $$ = new BinaryExpr(se, BinaryExpr::SUB, $1, $3);
    }
    ;
// 关系表达式
RelExp
    :
    AddExp {$$ = $1;}
    |
    RelExp LESS AddExp
    {
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        $$ = new BinaryExpr(se, BinaryExpr::LESS, $1, $3);
    }
    |
    RelExp LESSEQL AddExp {
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        $$ = new BinaryExpr(se, BinaryExpr::LESSEQL, $1, $3);
    }
    | 
    RelExp GREATER AddExp {
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        $$ = new BinaryExpr(se, BinaryExpr::GREATER, $1, $3);
    }
    |
    RelExp GREATEREQL AddExp {
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        $$ = new BinaryExpr(se, BinaryExpr::GREATEREQL, $1, $3);
    }
    ;
// 等价表达式
EqlExp
    : 
    RelExp {$$=$1;}
    |
    EqlExp EQL RelExp {
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        $$ = new BinaryExpr(se, BinaryExpr::EQL, $1, $3);
    }
    |
    EqlExp NOTEQL RelExp {
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        $$ = new BinaryExpr(se, BinaryExpr::NOTEQL, $1, $3);
    }
    ;
//逻辑与
LAndExp
    :
    EqlExp {$$ = $1;}
    |
    LAndExp AND EqlExp
    {
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        $$ = new BinaryExpr(se, BinaryExpr::AND, $1, $3);
    }
    ;
//逻辑或
LOrExp
    :
    LAndExp {$$ = $1;}
    |
    LOrExp OR LAndExp
    {
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        $$ = new BinaryExpr(se, BinaryExpr::OR, $1, $3);
    }
    ;

//********************表达式end**************************

//********************变量常量begin**************************
DeclStmt
    : VarDeclStmt {$$ = $1;}
    | ConstDeclStmt {$$ = $1;}
    ;
VarDeclStmt //完整的变量声明
    :
    Type VarDefList SEMICOLON {
        //变量的类型不能为void（类型检查基础要求1）
        if($1==TypeSystem::voidType)
        {
            fprintf(stderr, "variable type can't be void type\n");   
            assert($1!=TypeSystem::voidType);
        }
        $$ = $2;
    }
    ;

ConstDeclStmt //完整的常量声明
    :
    CONST Type ConstDefList SEMICOLON {
        //常量的类型不能为void（类型检查基础要求1）
        if($2==TypeSystem::voidType)
        {
            fprintf(stderr, "constant variable type can't be void type\n");   
            assert($2!=TypeSystem::voidType);
        }
        $$ = $3;
    }
    ;

VarDefList
    :
    VarDef {
        $$ = $1;
    }
    |
    VarDefList COMMA VarDef {
        $$ = $1;
        $1->setNext($3); //参数表的参数通过指针串起来
    }
    ;
ConstDefList
    :
    ConstDef {
        $$ = $1; 
    }
    |
    ConstDefList COMMA ConstDef {
        $$ = $1;
        $1->setNext($3);
    }
    ;
DeclArrayIndices
    : LBRACKET Exp RBRACKET {
        $$ = $2;
    }
    | DeclArrayIndices LBRACKET Exp RBRACKET {
        $$ = $1;
        $$->setNext($3);
    }
    ;
VarDef
    :
    ID {
        //检查变量是否重复声明（类型检查基础要求1）
        if(identifiers->lookuponlyforcurrent($1))
        {
            fprintf(stderr, "identifier \"%s\" is defined twice\n", (char*)$1);
            assert(identifiers->lookuponlyforcurrent($1)==nullptr);
        }
        SymbolEntry* se = new IdentifierSymbolEntry(curType, $1, identifiers->getLevel());
        $$ = new DeclStmt(new Id(se));
        identifiers->install($1, se);
        delete []$1;
    }   
    |
    ID ASSIGN Exp {
        //检查变量是否重复声明（类型检查基础要求1）
        if(identifiers->lookuponlyforcurrent($1))
        {
            fprintf(stderr, "identifier \"%s\" is defined twice\n", (char*)$1);
            assert(identifiers->lookuponlyforcurrent($1)==nullptr);
        }
        SymbolEntry* se = new IdentifierSymbolEntry(curType, $1, identifiers->getLevel());
        $$ = new DeclStmt(new Id(se), $3);
        identifiers->install($1, se);
        ((IdentifierSymbolEntry*)se)->setValue($3->getValue());
        delete []$1;
    } 
    |ID DeclArrayIndices{
        //检查变量是否重复声明（类型检查基础要求1）
        if(identifiers->lookuponlyforcurrent($1))
        {
            fprintf(stderr, "identifier \"%s\" is defined twice\n", (char*)$1);
            assert(identifiers->lookuponlyforcurrent($1)==nullptr);
        }
        
        ExprNode *expr = $2;
        std::vector<int> indexs;
        while(expr) {
            indexs.push_back(expr->getValue());
            expr = (ExprNode*)expr->getNext();
        }
        Type* arrType = new ArrayType(indexs, curType);

        SymbolEntry* se = new IdentifierSymbolEntry(arrType, $1, identifiers->getLevel());
        identifiers->install($1, se);
        $$ = new DeclStmt(new Id(se));
        delete []$1;
    }
    |ID DeclArrayIndices ASSIGN {
        //检查变量是否重复声明（类型检查基础要求1）
        if(identifiers->lookuponlyforcurrent($1))
        {
            fprintf(stderr, "identifier \"%s\" is defined twice\n", (char*)$1);
            assert(identifiers->lookuponlyforcurrent($1)==nullptr);
        }
        
        ExprNode *expr = $2;
        std::vector<int> indexs;
        if(expr== nullptr) printf("this is a null\n");
        while(expr) {
            indexs.push_back(expr->getValue());
            expr = (ExprNode*)expr->getNext();
        }
        auto arrType = new ArrayType(indexs, curType);

        SymbolEntry* se = new IdentifierSymbolEntry(arrType, $1, identifiers->getLevel());
        $<se>$ = se;
        identifiers->install($1, se);

        initArray = new ExprNode*[arrType->getSize() / arrType->getBaseType()->getSize()] {};
        idx = 0;
        std::stack<std::vector<int>>().swap(dimesionStack); // 这一句的作用就是清空栈
        dimesionStack.push(indexs);
        delete []$1;

    } InitVal {
        $$ = new DeclStmt(new Id($<se>4, $2));
        ((DeclStmt*)$$)->setInitArray(initArray);
        initArray = nullptr;
        idx = 0;
    }
    ;
//常量的声明要求必须赋初值
ConstDef
    :
    //用Exp替代
    ID ASSIGN Exp {
        //检查变量是否重复声明（类型检查基础要求1）
        if(identifiers->lookuponlyforcurrent($1))
        {
            fprintf(stderr, "constant identifier \"%s\" is defined twice\n", (char*)$1);
            assert(identifiers->lookuponlyforcurrent($1)==nullptr);
        }
        SymbolEntry* se = nullptr;
        if (curType->isInt()) {
            se = new IdentifierSymbolEntry(TypeSystem::constIntType, $1, identifiers->getLevel());
            ((IdentifierSymbolEntry*)se)->setValue((int)($3->getValue()));
        }
        else if (curType->isFloat()) {
            se = new IdentifierSymbolEntry(TypeSystem::constFloatType, $1, identifiers->getLevel());
            ((IdentifierSymbolEntry*)se)->setValue((double)($3->getValue()));
        }
        identifiers->install($1, se);
        ((IdentifierSymbolEntry*)se)->setConstant();
        $$ = new DeclStmt(new Id(se), $3);
        delete []$1;
    } 
    |ID DeclArrayIndices ASSIGN {
        //检查变量是否重复声明（类型检查基础要求1）
        if(identifiers->lookuponlyforcurrent($1))
        {
            fprintf(stderr, "identifier \"%s\" is defined twice\n", (char*)$1);
            assert(identifiers->lookuponlyforcurrent($1)==nullptr);
        }
        

        ExprNode* expr = $2;
        std::vector<int> indexs;
        while(expr) {
            indexs.push_back(expr->getValue());
            expr = (ExprNode*)expr->getNext();
        }
        ArrayType* arrType = nullptr;
        if(curType->isInt()) {
            arrType = new ArrayType(indexs, TypeSystem::constIntType);
        }
        else if(curType->isFloat()) {
            arrType = new ArrayType(indexs, TypeSystem::constFloatType);
        }
        assert(arrType != nullptr);
        SymbolEntry* se = new IdentifierSymbolEntry(arrType, $1, identifiers->getLevel());
        ((IdentifierSymbolEntry*)se)->setConstant();
        $<se>$ = se;
        identifiers->install($1, se);

        initArray = new ExprNode*[arrType->getSize() / arrType->getBaseType()->getSize()] {};
        idx = 0;
        std::stack<std::vector<int>>().swap(dimesionStack); // 这一句的作用就是清空栈
        dimesionStack.push(indexs);

        delete []$1;
    } ConstInitVal {
        $$ = new DeclStmt(new Id($<se>4));
        ((DeclStmt*)$$)->setInitArray(initArray);
        initArray = nullptr;
        idx = 0;
    }
    ;
InitVal
    :
    Exp {
        $$ = $1;
        if(initArray != nullptr) {
            initArray[idx++] = $1;
        }
    } 
    |
    LBRACE {
        std::vector<int> dimesion = dimesionStack.top();
        dimesionStack.push({-1, idx});
        dimesion.erase(dimesion.begin());
        if (dimesion.size() <= 0) {
            dimesion.push_back(1);
        }
        dimesionStack.push(dimesion);
    } InitValList RBRACE {
        while (dimesionStack.top()[0] != -1) {
            dimesionStack.pop();
        }
        idx = dimesionStack.top()[1];
        dimesionStack.pop();
        std::vector<int> dimesion = dimesionStack.top();
        int size = 1;
        for (auto dim : dimesion) {
            size *= dim;
        }
        idx += size;
    }
    |
    LBRACE RBRACE {
        std::vector<int> dimesion = dimesionStack.top();
        int size = 1;
        for(auto dim : dimesion) {
            size *= dim;
        }
        idx += size;
    }
    ;

InitValList
    :
    InitVal 
    | 
    InitValList COMMA InitVal 
    ;
ConstInitVal
    : Exp {
        $$ = $1;
        if (initArray != nullptr) {
            initArray[idx++] = $1;
        }
    }
    | LBRACE RBRACE {
        std::vector<int> dimesion = dimesionStack.top();
        int size = 1;
        for (auto dim : dimesion) {
            size *= dim;
        }
        idx += size;
    }
    | LBRACE {
        std::vector<int> dimesion = dimesionStack.top();
        dimesionStack.push({-1, idx});
        dimesion.erase(dimesion.begin());
        if (dimesion.size() <= 0) {
            dimesion.push_back(1);
        }
        dimesionStack.push(dimesion);
    } ConstInitValList RBRACE {
        while (dimesionStack.top()[0] != -1) {
            dimesionStack.pop();
        }
        idx = dimesionStack.top()[1];
        dimesionStack.pop();
        std::vector<int> dimesion = dimesionStack.top();
        int size = 1;
        for (auto dim : dimesion) {
            size *= dim;
        }
        idx += size;
    }
    ;
ConstInitValList
    : ConstInitVal
    | ConstInitValList COMMA ConstInitVal
    ;

//函数声明



FuncFParam
    : Type ID {
        SymbolEntry* se;
        int argNum;
        if ($1->isFloat()) {
            argNum = floatArgNum;
            if (argNum > 15) {
                argNum = spillPos;
                spillPos--;
            }
            floatArgNum++;
        }
        else {
            argNum = intArgNum;
            if (argNum > 3) {
                argNum = spillPos;
                spillPos--;
            }
            intArgNum++;
        }
        se = new IdentifierSymbolEntry($1, $2, identifiers->getLevel(), false, argNum);
        identifiers->install($2, se);
        $$ = new DeclStmt(new Id(se));
        delete []$2;
    }
    | Type ID LBRACKET RBRACKET {
        SymbolEntry* se;
        int argNum = intArgNum;
        if (argNum > 3) {
            argNum = spillPos;
            spillPos--;
        }
        intArgNum++;
        se = new IdentifierSymbolEntry(new PointerType(new ArrayType({}, $1)), $2, identifiers->getLevel(), false, argNum);
        identifiers->install($2, se);
        $$ = new DeclStmt(new Id(se));
        delete []$2;
    }
    | Type ID LBRACKET RBRACKET ArrayIndices {
        std::vector<int> indexs;
        ExprNode *expr = $5;
        while (expr) {
            indexs.push_back(expr->getValue());
            expr = (ExprNode*)expr->getNext();
        }
        int argNum = intArgNum;
        if (argNum > 3) {
            argNum = spillPos;
            spillPos--;
        }
        intArgNum++;
        SymbolEntry* se;
        se = new IdentifierSymbolEntry(new PointerType(new ArrayType(indexs, $1)), $2, identifiers->getLevel(), false, argNum);
        identifiers->install($2, se);
        $$ = new DeclStmt(new Id(se));
        delete []$2;
    }
    ;

FuncFParams
    :
    FuncFParam {
        $$ = $1;
    }
    |
    FuncFParams COMMA FuncFParam {
        $$ = $1;
        $$->setNext($3);
    }
    |
    %empty {
        $$ = nullptr;
    }
    ;

FuncDef
    :
    Type ID {
        //retType = $1;
        // needRet = true;
        identifiers = new SymbolTable(identifiers);
        spillPos = -1;
        intArgNum = 0;
        floatArgNum = 0;
    }
    LPAREN FuncFParams RPAREN {
        Type* funcType;
        std::vector<Type*> vec;
        std::vector<SymbolEntry*> vec1;
        DeclStmt* temp = (DeclStmt*)$5;
        while(temp){
            vec.push_back(temp->getId()->getSymPtr()->getType());
            vec1.push_back(temp->getId()->getSymPtr());
            temp = (DeclStmt*)(temp->getNext());
        }
        funcType = new FunctionType($1, vec, vec1);
        SymbolEntry* se = new IdentifierSymbolEntry(funcType, $2, identifiers->getPrev()->getLevel());
        //install中有检查重载函数的逻辑，在SymbolTable.cpp中查看（类型检查进阶要求1）
        identifiers->getPrev()->install($2, se);
    }
    BlockStmt
    {
        SymbolEntry *se = identifiers->lookup($2);
        assert(se != nullptr);
        $$ = new FunctionDef(se, $8, (DeclStmt*)$5);
        SymbolTable *top = identifiers;
        identifiers = identifiers->getPrev();
        delete top;        
        delete []$2;
    }
    ;






//********************函数定义end**************************

%%

int yyerror(char const* message)
{
    std::cerr<<message<<std::endl;
    return -1;
}
