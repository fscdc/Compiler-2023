#include <iostream>
#include <string.h>
#include <unistd.h>
#include "common.h"
#include "Ast.h"
#include "SymbolTable.h"
#include "Unit.h"
#include "Mem2Reg.h"
#include "PhiElim.h"
#include "IRComSubExprElim.h"
#include "IRDeadCodeElim.h"
#include "IRPeepHole.h"
#include "MachineCode.h"
// #include "LinearScan.h"
#include "GraphColor.h"
extern FILE *yyin;
extern FILE *yyout;

int yyparse();

Ast ast;
Unit unit;
MachineUnit mUnit;
char outfile[256] = "a.out";
dump_type_t dump_type = ASM;

bool optforir = true;
bool optforregall = true;

int main(int argc, char *argv[])
{
    int opt;
    while ((opt = getopt(argc, argv, "Siato:")) != -1)
    {
        switch (opt)
        {
        case 'o':
            strcpy(outfile, optarg);
            break;
        case 'a':
            dump_type = AST;
            break;
        case 't':
            dump_type = TOKENS;
            break;
        case 'i':
            dump_type = IR;
            break;
        case 'S':
            dump_type = ASM;
            break;
        default:
            fprintf(stderr, "Usage: %s [-o outfile] infile\n", argv[0]);
            exit(EXIT_FAILURE);
            break;
        }
    }
    if (optind >= argc)
    {
        fprintf(stderr, "no input file\n");
        exit(EXIT_FAILURE);
    }
    if (!(yyin = fopen(argv[optind], "r")))
    {
        fprintf(stderr, "%s: No such file or directory\nno input file\n", argv[optind]);
        exit(EXIT_FAILURE);
    }
    if (!(yyout = fopen(outfile, "w")))
    {
        fprintf(stderr, "%s: fail to open output file\n", outfile);
        exit(EXIT_FAILURE);
    }

    //运行时库的加入，我在maincpp中就提前加入，将运行时库函数都存入符号表
    // getint
    std::vector<Type*> vec;
    std::vector<SymbolEntry*> vec1;
    Type* funcType = new FunctionType(TypeSystem::intType, vec,vec1);
    SymbolTable* st = identifiers;
    while(st->getPrev())
        st = st->getPrev();
    SymbolEntry* se = new IdentifierSymbolEntry(funcType, "getint", st->getLevel(), true);
    st->install("getint", se);

    // getch
    funcType = new FunctionType(TypeSystem::intType, vec,vec1);
    st = identifiers;
    while(st->getPrev())
        st = st->getPrev();
    se = new IdentifierSymbolEntry(funcType,"getch", st->getLevel(), true);
    st->install("getch", se);

    // getfloat
    funcType = new FunctionType(TypeSystem::floatType, vec,vec1);
    st = identifiers;
    while(st->getPrev())
        st = st->getPrev();
    se = new IdentifierSymbolEntry(funcType, "getfloat", st->getLevel(), true);
    st->install("getfloat", se);

    // getarray
    ArrayType* arr = new ArrayType({}, TypeSystem::intType);
    vec.push_back(arr);
    funcType = new FunctionType(TypeSystem::intType, vec,vec1);
    st = identifiers;
    while(st->getPrev())
        st = st->getPrev();
    se = new IdentifierSymbolEntry(funcType, "getarray", st->getLevel(), true);
    st->install("getarray", se);

    // getfarray
    arr = new ArrayType({}, TypeSystem::floatType);
    funcType = new FunctionType(TypeSystem::intType, vec,vec1);
    st = identifiers;
    while(st->getPrev())
        st = st->getPrev();
    se = new IdentifierSymbolEntry(funcType, "getfarray", st->getLevel(), true);
    st->install("getfarray", se);

    // putint
    vec.clear();
    vec1.clear();
    vec.push_back(TypeSystem::intType);
    funcType = new FunctionType(TypeSystem::voidType, vec,vec1);
    st = identifiers;
    while(st->getPrev())
        st = st->getPrev();
    se = new IdentifierSymbolEntry(funcType, "putint", st->getLevel(), true);
    st->install("putint", se);

    // putch
    vec.clear();
    vec1.clear();
    vec.push_back(TypeSystem::intType);
    funcType = new FunctionType(TypeSystem::voidType, vec,vec1);
    st = identifiers;
    while(st->getPrev())
        st = st->getPrev();
    se = new IdentifierSymbolEntry(funcType, "putch", st->getLevel(), true);
    st->install("putch", se);

    // putfloat
    vec.clear();
    vec1.clear();
    vec.push_back(TypeSystem::floatType);
    funcType = new FunctionType(TypeSystem::voidType, vec,vec1);
    st = identifiers;
    while(st->getPrev())
        st = st->getPrev();
    se = new IdentifierSymbolEntry(funcType, "putfloat", st->getLevel(), true);
    st->install("putfloat", se);


    // putarray
    vec.clear();
    vec1.clear();
    vec.push_back(TypeSystem::intType);
    arr = new ArrayType({}, TypeSystem::intType);
    vec.push_back(arr);
    funcType = new FunctionType(TypeSystem::voidType, vec,vec1);
    st = identifiers;
    while(st->getPrev())
        st = st->getPrev();
    se = new IdentifierSymbolEntry(funcType, "putarray", st->getLevel(), true);
    st->install("putarray", se);

    // putfarray
    vec.clear();
    vec1.clear();
    vec.push_back(TypeSystem::intType);
    arr = new ArrayType({}, TypeSystem::floatType);
    vec.push_back(arr);
    funcType = new FunctionType(TypeSystem::voidType, vec,vec1);
    st = identifiers;
    while(st->getPrev())
        st = st->getPrev();
    se = new IdentifierSymbolEntry(funcType, "putfarray", st->getLevel(), true);
    st->install("putfarray", se);


    // putf
    vec.clear();
    vec1.clear();
    StringType* str = new StringType(0);
    vec.push_back(str);
    funcType = new FunctionType(TypeSystem::voidType, vec,vec1);
    st = identifiers;
    while(st->getPrev())
        st = st->getPrev();
    se = new IdentifierSymbolEntry(funcType, "putf", st->getLevel(), true);
    st->install("putf", se);

    // starttime
    vec.clear();
    vec1.clear();
    funcType = new FunctionType(TypeSystem::voidType, vec,vec1);
    st = identifiers;
    while(st->getPrev())
        st = st->getPrev();
    se = new IdentifierSymbolEntry(funcType, "starttime", st->getLevel(), true);
    st->install("starttime", se);

    // stoptime
    vec.clear();
    vec1.clear();
    funcType = new FunctionType(TypeSystem::voidType, vec,vec1);
    st = identifiers;
    while(st->getPrev())
        st = st->getPrev();
    se = new IdentifierSymbolEntry(funcType, "stoptime", st->getLevel(), true);
    st->install("stoptime", se);    




    yyparse();
    if(dump_type == AST)
        ast.output();
    ast.typeCheck();
    ast.genCode(&unit);

        
    Mem2Reg m2r(&unit);
    IRComSubExprElim cse(&unit);
    IRDeadCodeElim dse(&unit);
    IRPeepHole iph(&unit);
    PhiElimination pe(&unit);


    //开启IR阶段的优化
    if(optforir){
        m2r.pass();
        cse.pass();
        dse.pass();
        iph.pass();
        pe.pass();     
    }

    

    if(dump_type == IR)
        unit.output();

    
    //目标代码生成
    unit.genMachineCode(&mUnit);

    if(optforregall){
        GraphColor graphColor(&mUnit);
        graphColor.allocateRegisters();   
    }
    else{
        //节省点编译时间
        // LinearScan linearScan(&mUnit);
        // linearScan.allocateRegisters();
    }
 
    
    if(dump_type == ASM)
        mUnit.output();
    return 0;
}
