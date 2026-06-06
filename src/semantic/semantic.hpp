#pragma once
#include "ast_node.hpp"
#include "symbol_table.hpp"
#include "parse_tree_node.hpp"
#include <string>
#include <vector>
#include <stdexcept>
#include <iostream>

struct SemanticError : public std::runtime_error {
    std::string msg;
    SemanticError(const std::string& m)
        : std::runtime_error("Semantic Error: " + m), msg(m) {}
};

class SemanticAnalyzer {
public:
    SymbolTable symtab;
    std::vector<std::string> errors;

    SemanticAnalyzer();

    ASTNode* analyze(ParseTreeNode* parseTree);

    ASTNode* visitProgram(ParseTreeNode* n);
    ASTNode* visitBlock(ParseTreeNode* n);
    ASTNode* visitDeclarationPart(ParseTreeNode* n);

    ASTNode* visitConstDeclaration(ParseTreeNode* n);
    ASTNode* visitTypeDeclaration(ParseTreeNode* n);
    ASTNode* visitVarDeclaration(ParseTreeNode* n);
    ASTNode* visitSubprogramDeclaration(ParseTreeNode* n);
    ASTNode* visitProcedureDeclaration(ParseTreeNode* n);
    ASTNode* visitFunctionDeclaration(ParseTreeNode* n);
    ASTNode* visitFormalParameterList(ParseTreeNode* n, int& lastParam, int& psze);

    std::pair<int,int> visitType(ParseTreeNode* n);
    std::pair<int,int> visitArrayType(ParseTreeNode* n);
    int evalConstant(ParseTreeNode* n);
    int typeOfConstant(ParseTreeNode* n);

    ASTNode* visitCompoundStatement(ParseTreeNode* n);
    ASTNode* visitStatementList(ParseTreeNode* n);
    ASTNode* visitStatement(ParseTreeNode* n);
    ASTNode* visitAssignStatement(ParseTreeNode* n);
    ASTNode* visitIfStatement(ParseTreeNode* n);
    ASTNode* visitWhileStatement(ParseTreeNode* n);
    ASTNode* visitForStatement(ParseTreeNode* n);
    ASTNode* visitRepeatStatement(ParseTreeNode* n);
    ASTNode* visitCaseStatement(ParseTreeNode* n);
    ASTNode* visitProcCall(ParseTreeNode* n);

    ASTNode* visitExpression(ParseTreeNode* n);
    ASTNode* visitSimpleExpression(ParseTreeNode* n);
    ASTNode* visitTerm(ParseTreeNode* n);
    ASTNode* visitFactor(ParseTreeNode* n);
    ASTNode* visitVariable(ParseTreeNode* n);

    void printResults(std::ostream& out) const;

private:

    std::string tokVal(ParseTreeNode* n) const;
    std::string tokTy(ParseTreeNode* n) const;
    bool isNT(ParseTreeNode* n, const std::string& name) const;
    bool isTok(ParseTreeNode* n, const std::string& type) const;
    ParseTreeNode* findChild(ParseTreeNode* n, const std::string& name) const;
    std::vector<ParseTreeNode*> childrenNamed(ParseTreeNode* n, const std::string& name) const;

    void addError(const std::string& msg);
};
