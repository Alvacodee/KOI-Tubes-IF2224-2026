#pragma once
#include "ast_node.hpp"
#include "symbol_table.hpp"
#include <string>
#include <vector>
#include <iostream>
#include <fstream>

// Opcode TAC (Three-Address Code)
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

// Nomor operasi untuk OPR
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

// Satu instruksi TAC
struct Instruction {
    OpcodeType op;
    int        lvl;
    int        arg;

    Instruction(OpcodeType o, int l, int a)
        : op(o), lvl(l), arg(a) {}

    void print(std::ostream& out, int index) const;

    std::string opName() const;
};

// Intermediate Code Generator
// Mengubah Decorated AST → daftar Instruction (TAC)
// Menggunakan pendekatan DFS-based (bukan DFS murni)
class IntermediateCodeGenerator {
public:
    // Daftar instruksi hasil generate
    std::vector<Instruction> code;

    // Reference ke symbol table (dari SemanticAnalyzer)
    const SymbolTable& symtab;

    // Constructor
    explicit IntermediateCodeGenerator(const SymbolTable& st)
        : symtab(st), labelCounter(0) {}

    // Entry point: generate dari AST_PROGRAM node
    void generate(ASTNode* root);

    // Cetak semua instruksi ke stream
    void print(std::ostream& out) const;

    // Simpan ke file
    void printToFile(const std::string& path) const;

    // ---- Interface ----

    // Program structure & fungsi/prosedur
    void genProgram(ASTNode* node);
    void genBlock(ASTNode* node);
    void genDeclPart(ASTNode* node);
    void genProcDecl(ASTNode* node);
    void genFuncDecl(ASTNode* node);

    // Expressions & statements
    void genExpr(ASTNode* node);
    void genAssign(ASTNode* node);
    void genProcCall(ASTNode* node);
    void genVar(ASTNode* node);

    // Control flow
    void genCompound(ASTNode* node);
    void genStmtList(ASTNode* node);
    void genStmt(ASTNode* node);
    void genIf(ASTNode* node);
    void genWhile(ASTNode* node);
    void genFor(ASTNode* node);
    void genRepeat(ASTNode* node);

    // ---- Helper internal ----

    // Emit satu instruksi, kembalikan index baris
    int emit(OpcodeType op, int lvl, int arg);

    // Kembalikan index baris instruksi berikutnya (untuk backpatch)
    int nextAddr() const { return (int)code.size(); }

    // Backpatch: ubah arg instruksi di baris `addr` menjadi `target`
    void backpatch(int addr, int target);

    // Hitung ukuran frame dari btab entry ke-b
    int frameSize(int btabIdx) const;

    // Cari address absolut variabel dari tab entry ke-i
    int varAddress(int tabIdx) const;

private:
    int labelCounter;
};

// Interpreter Header
// forward declaration

class Interpreter;
