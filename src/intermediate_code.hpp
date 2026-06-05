#pragma once
#include "ast_node.hpp"
#include "symbol_table.hpp"
#include <string>
#include <vector>
#include <iostream>
#include <fstream>

enum OpcodeType {
    OP_INT,
    OP_LIT,
    OP_LOD,
    OP_STO,
    OP_OPR,
    OP_JMP,
    OP_JPC,
    OP_CAL,
    OP_RET,
};

enum OprType {
    OPR_NEG   = 1,
    OPR_ADD   = 2,
    OPR_SUB   = 3,
    OPR_MUL   = 4,
    OPR_DIV   = 5,
    OPR_MOD   = 6,
    OPR_EQL   = 7,
    OPR_NEQ   = 8,
    OPR_LSS   = 9,
    OPR_GEQ   = 10,
    OPR_GTR   = 11,
    OPR_LEQ   = 12,
    OPR_WRT   = 13,
    OPR_WRTLN = 14,
    OPR_ODD   = 15,
    OPR_NOT   = 16,
    OPR_AND   = 17,
    OPR_OR    = 18,
};

struct Instruction {
    OpcodeType op;
    int        lvl;
    int        arg;

    Instruction(OpcodeType o, int l, int a)
        : op(o), lvl(l), arg(a) {}

    void print(std::ostream& out, int index) const;

    std::string opName() const;
};

class IntermediateCodeGenerator {
public:

    std::vector<Instruction> code;

    const SymbolTable& symtab;

    explicit IntermediateCodeGenerator(const SymbolTable& st) : symtab(st), labelCounter(0) {}

    void generate(ASTNode* root);

    void print(std::ostream& out) const;

    void printToFile(const std::string& path) const;

    void genProgram(ASTNode* node);
    void genBlock(ASTNode* node);
    void genDeclPart(ASTNode* node);
    void genProcDecl(ASTNode* node);
    void genFuncDecl(ASTNode* node);

    void genExpr(ASTNode* node);
    void genAssign(ASTNode* node);
    void genProcCall(ASTNode* node);
    void genVar(ASTNode* node);

    void genCompound(ASTNode* node);
    void genStmtList(ASTNode* node);
    void genStmt(ASTNode* node);
    void genIf(ASTNode* node);
    void genWhile(ASTNode* node);
    void genFor(ASTNode* node);
    void genRepeat(ASTNode* node);

    int emit(OpcodeType op, int lvl, int arg);

    int nextAddr() const { return (int)code.size(); }

    void backpatch(int addr, int target);

    int frameSize(int btabIdx) const;

    int varAddress(int tabIdx) const;

private:
    int labelCounter;
};

class Interpreter;
