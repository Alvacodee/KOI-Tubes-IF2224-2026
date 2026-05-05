#pragma once
#include <vector>
#include <string>
#include <stdexcept>
#include "token.hpp"
#include "node.hpp"

// SyntaxError: dilempar saat parser menemukan token yang tidak sesuai grammar
struct SyntaxError : public std::runtime_error {
    int line;
    SyntaxError(const std::string& msg, int ln)
        : std::runtime_error(msg), line(ln) {}
};

// Parser mengimplementasikan Recursive Descent untuk grammar bahasa Arion.
// setiap non-terminal dalam grammar punya fungsi parse sendiri.
// input: list of tokens dari lexer. output: parse tree (shared_ptr<ParseNode>).
class Parser {
public:
    explicit Parser(const std::vector<Token>& tokens);

    // entry point — parse seluruh program dan return root node
    std::shared_ptr<ParseNode> parse();

private:
    std::vector<Token> tokens;
    size_t pos; // posisi token yang sedang dibaca

    //  Token navigation 
    Token& current();
    Token& peek(int offset = 1);
    Token  consume();
    Token  expect(TokenType type); // consume dan validasi tipe, lempar SyntaxError jika tidak cocok
    bool   check(TokenType type) const;
    bool   atEnd() const;

    // Expression, Term, Factor, Procedure/Function-call 
    std::shared_ptr<ParseNode> parseExpression();
    std::shared_ptr<ParseNode> parseSimpleExpression();
    std::shared_ptr<ParseNode> parseTerm();
    std::shared_ptr<ParseNode> parseFactor();
    std::shared_ptr<ParseNode> parseProcFuncCall();
    std::shared_ptr<ParseNode> parseParameterList();
    std::shared_ptr<ParseNode> parseVariable();
    std::shared_ptr<ParseNode> parseComponentVariable(std::shared_ptr<ParseNode> varNode);
    std::shared_ptr<ParseNode> parseIndexList();


    // KALO GA DIPAKE HAPUS AJA
    // KALO MO NYOBA TINGGAL UNCOMMENT INI SAMA YANG DI BAGIAN IMPLEMENTASI (PARSER.CPP)
    // KALO BAGIAN GUA (ZAHRAN) UDAH GUA PISAH DI MODUL EXPRESSION.CPP 

    // // Daniel: program structure
    // std::shared_ptr<ParseNode> parseProgram();
    // std::shared_ptr<ParseNode> parseProgramHeader();
    // std::shared_ptr<ParseNode> parseBlock();
    // std::shared_ptr<ParseNode> parseDeclarationPart();

    // // Neswa: declarations
    // std::shared_ptr<ParseNode> parseConstDeclaration();
    // std::shared_ptr<ParseNode> parseConstant();
    // std::shared_ptr<ParseNode> parseTypeDeclaration();
    // std::shared_ptr<ParseNode> parseType();
    // std::shared_ptr<ParseNode> parseArrayType();
    // std::shared_ptr<ParseNode> parseRange();
    // std::shared_ptr<ParseNode> parseEnumerated();
    // std::shared_ptr<ParseNode> parseRecordType();
    // std::shared_ptr<ParseNode> parseFieldList();
    // std::shared_ptr<ParseNode> parseFieldPart();
    // std::shared_ptr<ParseNode> parseVarDeclaration();
    // std::shared_ptr<ParseNode> parseIdentifierList();
    // std::shared_ptr<ParseNode> parseSubprogramDeclaration();
    // std::shared_ptr<ParseNode> parseProcedureDeclaration();
    // std::shared_ptr<ParseNode> parseFunctionDeclaration();
    // std::shared_ptr<ParseNode> parseFormalParameterList();
    // std::shared_ptr<ParseNode> parseParameterGroup();

    // // Awan: statements & control flow
    // std::shared_ptr<ParseNode> parseCompoundStatement();
    // std::shared_ptr<ParseNode> parseStatementList();
    // std::shared_ptr<ParseNode> parseStatement();
    // std::shared_ptr<ParseNode> parseAssignmentStatement();
    // std::shared_ptr<ParseNode> parseIfStatement();
    // std::shared_ptr<ParseNode> parseCaseStatement();
    // std::shared_ptr<ParseNode> parseCaseBlock();
    // std::shared_ptr<ParseNode> parseWhileStatement();
    // std::shared_ptr<ParseNode> parseRepeatStatement();
    // std::shared_ptr<ParseNode> parseForStatement();
};
