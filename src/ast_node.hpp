#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <fstream>

// =============================================
// Jenis node AST
// =============================================
enum ASTKind {
    // Program structure
    AST_PROGRAM, AST_BLOCK, AST_DECL_PART,
    // Deklarasi
    AST_CONST_DECL, AST_TYPE_DECL, AST_VAR_DECL,
    AST_PROC_DECL, AST_FUNC_DECL,
    AST_PARAM_LIST, AST_PARAM_GROUP,
    // Statement
    AST_COMPOUND, AST_STMT_LIST,
    AST_ASSIGN, AST_IF, AST_WHILE, AST_FOR,
    AST_REPEAT, AST_CASE, AST_CASE_BLOCK,
    AST_PROC_CALL, AST_EMPTY,
    // Ekspresi
    AST_BINOP, AST_UNOP,
    AST_VAR, AST_ARRAY_ACCESS, AST_FIELD_ACCESS,
    AST_INT_LIT, AST_REAL_LIT, AST_CHAR_LIT,
    AST_STR_LIT, AST_BOOL_LIT,
};

// =============================================
// Kode tipe dasar (sesuai Lampiran B contoh)
// =============================================
const int T_NONE    = 0;  // void / belum diketahui
const int T_INTEGER = 1;
const int T_REAL    = 2;
const int T_BOOLEAN = 3;
const int T_CHAR    = 4;
const int T_ARRAY   = 5;
const int T_RECORD  = 6;
const int T_STRING  = 7;

// =============================================
// Kelas objek identifier
// =============================================
const int OBJ_CONST    = 0;
const int OBJ_VARIABLE = 1;
const int OBJ_TYPE     = 2;
const int OBJ_PROC     = 3;
const int OBJ_FUNC     = 4;
const int OBJ_PROGRAM  = 5;
const int OBJ_KEYWORD  = 6;  // reserved word (placeholder)

// nama tipe untuk output
inline std::string typeName(int tc) {
    switch (tc) {
        case T_INTEGER: return "integer";
        case T_REAL:    return "real";
        case T_BOOLEAN: return "boolean";
        case T_CHAR:    return "char";
        case T_ARRAY:   return "array";
        case T_RECORD:  return "record";
        case T_STRING:  return "string";
        default:        return "none";
    }
}

inline std::string objName(int obj) {
    switch (obj) {
        case OBJ_CONST:    return "const";
        case OBJ_VARIABLE: return "variable";
        case OBJ_TYPE:     return "type";
        case OBJ_PROC:     return "procedure";
        case OBJ_FUNC:     return "function";
        case OBJ_PROGRAM:  return "program";
        case OBJ_KEYWORD:  return "keyword";
        default:           return "?";
    }
}

// =============================================
// Node AST
// =============================================
struct ASTNode {
    ASTKind kind;
    std::string value;   // lexeme untuk literal / nama identifier
    std::string op;      // operator untuk BinOp / UnOp

    // Anotasi semantik (diisi saat semantic analysis)
    int typeCode = T_NONE;  // kode tipe hasil
    int typeRef  = 0;       // ref ke atab/btab untuk tipe komposit
    int tabIndex = -1;      // indeks ke tab
    int level    = 0;       // lexical level

    std::vector<ASTNode*> children;

    ASTNode(ASTKind k, const std::string& v = "", const std::string& o = "")
        : kind(k), value(v), op(o) {}

    ~ASTNode() {
        for (auto* c : children) delete c;
    }

    void add(ASTNode* child) {
        if (child) children.push_back(child);
    }

    // Cetak decorated AST ke stream
    void print(std::ostream& out,
               const std::string& prefix = "",
               bool isLast = true) const;

    void printToFile(const std::string& path) const;
};

// Factory helper
inline ASTNode* makeAST(ASTKind k, const std::string& v = "") {
    return new ASTNode(k, v);
}
