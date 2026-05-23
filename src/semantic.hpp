#pragma once
#include "ast_node.hpp"
#include "symbol_table.hpp"
#include "parse_tree_node.hpp"
#include <string>
#include <vector>
#include <stdexcept>
#include <iostream>

// Error semantik
struct SemanticError : public std::runtime_error {
    std::string msg;
    SemanticError(const std::string& m)
        : std::runtime_error("Semantic Error: " + m), msg(m) {}
};

// =============================================
// SemanticAnalyzer
// Traversal top-down pada parse tree,
// bangun AST, isi symbol table, anotasi tipe
// =============================================
class SemanticAnalyzer {
public:
    SymbolTable symtab;
    std::vector<std::string> errors;

    SemanticAnalyzer();

    // Entry point
    ASTNode* analyze(ParseTreeNode* parseTree);

    // ---------- Visitor per node ----------

    // Program & Block
    ASTNode* visitProgram(ParseTreeNode* n);
    ASTNode* visitBlock(ParseTreeNode* n);
    ASTNode* visitDeclarationPart(ParseTreeNode* n);

    // Deklarasi (semantic_decl.cpp)
    ASTNode* visitConstDeclaration(ParseTreeNode* n);
    ASTNode* visitTypeDeclaration(ParseTreeNode* n);
    ASTNode* visitVarDeclaration(ParseTreeNode* n);
    ASTNode* visitSubprogramDeclaration(ParseTreeNode* n);
    ASTNode* visitProcedureDeclaration(ParseTreeNode* n);
    ASTNode* visitFunctionDeclaration(ParseTreeNode* n);
    ASTNode* visitFormalParameterList(ParseTreeNode* n, int& lastParam, int& psze);

    // Tipe (semantic_decl.cpp)
    std::pair<int,int> visitType(ParseTreeNode* n);
    std::pair<int,int> visitArrayType(ParseTreeNode* n);
    int evalConstant(ParseTreeNode* n);
    int typeOfConstant(ParseTreeNode* n);

    // Bagian 3 (INI SEHARUSNYA PUNYA LO WAN)
    // ASTNode* visitCompoundStatement(ParseTreeNode* n);
    // ASTNode* visitStatementList(ParseTreeNode* n);
    // ASTNode* visitStatement(ParseTreeNode* n);
    // ASTNode* visitAssignStatement(ParseTreeNode* n);
    // ASTNode* visitIfStatement(ParseTreeNode* n);
    // ASTNode* visitWhileStatement(ParseTreeNode* n);
    // ASTNode* visitForStatement(ParseTreeNode* n);
    // ASTNode* visitRepeatStatement(ParseTreeNode* n);
    // ASTNode* visitCaseStatement(ParseTreeNode* n);
    // ASTNode* visitProcCall(ParseTreeNode* n);

    // // Ekspresi (stubs untuk Part 3, diisi di semantic_expr.cpp)
    // ASTNode* visitExpression(ParseTreeNode* n);
    // ASTNode* visitSimpleExpression(ParseTreeNode* n);
    // ASTNode* visitTerm(ParseTreeNode* n);
    // ASTNode* visitFactor(ParseTreeNode* n);
    // ASTNode* visitVariable(ParseTreeNode* n);

    // Output
    void printResults(std::ostream& out) const;

private:
    // Helper traversal parse tree
    std::string tokVal(ParseTreeNode* n) const;
    std::string tokTy(ParseTreeNode* n) const;
    bool isNT(ParseTreeNode* n, const std::string& name) const;
    bool isTok(ParseTreeNode* n, const std::string& type) const;
    ParseTreeNode* findChild(ParseTreeNode* n, const std::string& name) const;
    std::vector<ParseTreeNode*> childrenNamed(ParseTreeNode* n, const std::string& name) const;

    // Catat error tanpa throw (agar bisa lanjut parsing)
    void addError(const std::string& msg);
};
