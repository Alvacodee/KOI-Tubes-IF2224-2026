#include "intermediate_code.hpp"
#include <iomanip>
#include <sstream>

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

void IntermediateCodeGenerator::generate(ASTNode* root) {
    if (!root) return;
    genProgram(root);
}

void IntermediateCodeGenerator::genProgram(ASTNode* node) {

    (void)node;
}

void IntermediateCodeGenerator::genBlock(ASTNode* node) {

    (void)node;
}

void IntermediateCodeGenerator::genDeclPart(ASTNode* node) {

    (void)node;
}

void IntermediateCodeGenerator::genProcDecl(ASTNode* node) {

    (void)node;
}

void IntermediateCodeGenerator::genFuncDecl(ASTNode* node) {

    (void)node;
}

void IntermediateCodeGenerator::genExpr(ASTNode* node) {

    (void)node;
}

void IntermediateCodeGenerator::genAssign(ASTNode* node) {

    (void)node;
}

void IntermediateCodeGenerator::genProcCall(ASTNode* node) {

    (void)node;
}

void IntermediateCodeGenerator::genVar(ASTNode* node) {

    (void)node;
}

void IntermediateCodeGenerator::genCompound(ASTNode* node) {

    (void)node;
}

void IntermediateCodeGenerator::genStmtList(ASTNode* node) {

    (void)node;
}

void IntermediateCodeGenerator::genStmt(ASTNode* node) {

    (void)node;
}

void IntermediateCodeGenerator::genIf(ASTNode* node) {

    (void)node;
}

void IntermediateCodeGenerator::genWhile(ASTNode* node) {

    (void)node;
}

void IntermediateCodeGenerator::genFor(ASTNode* node) {

    (void)node;
}

void IntermediateCodeGenerator::genRepeat(ASTNode* node) {

    (void)node;
}
