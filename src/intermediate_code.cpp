#include "intermediate_code.hpp"
#include <iomanip>
#include <sstream>

// Instruction helpers

std::string Instruction::opName() const {
    switch (op) {
        case OP_INT: return "INT";
        case OP_LIT: return "LIT";
        case OP_LOD: return "LOD";
        case OP_STO: return "STO";
        case OP_OPR: return "OPR";
        case OP_JMP: return "JMP";
        case OP_JPC: return "JPC";
        case OP_CAL: return "CAL";
        case OP_RET: return "RET";
        default:     return "???";
    }
}

void Instruction::print(std::ostream& out, int index) const {
    out << index << " " << opName() << " " << lvl << " " << arg << "\n";
}

// IntermediateCodeGenerator — helpers

int IntermediateCodeGenerator::emit(OpcodeType op, int lvl, int arg) {
    int idx = (int)code.size();
    code.emplace_back(op, lvl, arg);
    return idx;
}

void IntermediateCodeGenerator::backpatch(int addr, int target) {
    code[addr].arg = target;
}

int IntermediateCodeGenerator::frameSize(int btabIdx) const {
    return 3 + symtab.btab[btabIdx].vsze;
}

int IntermediateCodeGenerator::varAddress(int tabIdx) const {
    return 3 + symtab.tab[tabIdx].adr;
}

void IntermediateCodeGenerator::print(std::ostream& out) const {
    for (int i = 0; i < (int)code.size(); i++) {
        code[i].print(out, i);
    }
}

void IntermediateCodeGenerator::printToFile(const std::string& path) const {
    std::ofstream f(path);
    if (f.is_open()) {
        print(f);
        f.close();
    }
}

// STUB: Fungsi-fungsi di bawah ini diisi oleh masing-masing anggota

void IntermediateCodeGenerator::generate(ASTNode* root) {
    if (!root) return;
    genProgram(root);
}

void IntermediateCodeGenerator::genProgram(ASTNode* node) {
    // TODO:
    // 1. Emit INT 0 frameSize(btab[1])  -- frame program utama
    // 2. genBlock untuk children[0] (AST_BLOCK)
    // 3. Emit RET
    (void)node;
}

void IntermediateCodeGenerator::genBlock(ASTNode* node) {
    // TODO:
    (void)node;
}

void IntermediateCodeGenerator::genDeclPart(ASTNode* node) {
    // TODO:
    (void)node;
}

void IntermediateCodeGenerator::genProcDecl(ASTNode* node) {
    // TODO:
    (void)node;
}

void IntermediateCodeGenerator::genFuncDecl(ASTNode* node) {
    // TODO:
    (void)node;
}

void IntermediateCodeGenerator::genExpr(ASTNode* node) {
    // TODO:
    (void)node;
}

void IntermediateCodeGenerator::genAssign(ASTNode* node) {
    // TODO:
    (void)node;
}

void IntermediateCodeGenerator::genProcCall(ASTNode* node) {
    // TODO: (write/writeln) & (user-defined proc)
    (void)node;
}

void IntermediateCodeGenerator::genVar(ASTNode* node) {
    // TODO:
    (void)node;
}

void IntermediateCodeGenerator::genCompound(ASTNode* node) {
    // TODO:
    (void)node;
}

void IntermediateCodeGenerator::genStmtList(ASTNode* node) {
    // TODO:
    (void)node;
}

void IntermediateCodeGenerator::genStmt(ASTNode* node) {
    // TODO:
    (void)node;
}

void IntermediateCodeGenerator::genIf(ASTNode* node) {
    // TODO:
    (void)node;
}

void IntermediateCodeGenerator::genWhile(ASTNode* node) {
    // TODO:
    (void)node;
}

void IntermediateCodeGenerator::genFor(ASTNode* node) {
    // TODO:
    (void)node;
}

void IntermediateCodeGenerator::genRepeat(ASTNode* node) {
    // TODO:
    (void)node;
}
