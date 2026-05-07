#pragma once
#include <vector>
#include <string>
#include <stdexcept>
#include "token.hpp"
#include "parse_tree_node.hpp"

class SyntaxError : public std::runtime_error {
public:
    int line;
    int col;
    std::string found;
    std::string expected;

    SyntaxError(int line, int col, const std::string& found, const std::string& expected);
};

class Parser {
public:
    explicit Parser(const std::vector<Token>& tokens);

    ParseTreeNode* parse();

private:
    std::vector<Token> tokens;
    int currentIndex;

    Token peek() const;
    Token advance();
    bool isAtEnd() const;

    ParseTreeNode* match(TokenType expectedType);

    ParseTreeNode* parseProgram();
    ParseTreeNode* parseProgramHeader();
    ParseTreeNode* parseBlock();
    ParseTreeNode* parseDeclarationPart();
    ParseTreeNode* parseCompoundStatement();
};
