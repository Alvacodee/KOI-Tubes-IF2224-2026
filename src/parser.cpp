#include "parser.hpp"
#include <sstream>

static std::string buildSyntaxErrorMsg(int line, const std::string& found, const std::string& expected) {
    std::ostringstream oss;
    oss << "Syntax error at line " << line
        << ": unexpected token '" << found
        << "', expected '" << expected << "'";
    return oss.str();
}

SyntaxError::SyntaxError(int line, int col, const std::string& found, const std::string& expected)
    : std::runtime_error(buildSyntaxErrorMsg(line, found, expected)),
      line(line), col(col), found(found), expected(expected) {}

Parser::Parser(const std::vector<Token>& tokens)
    : tokens(tokens), currentIndex(0) {}

Token Parser::peek() const {
    if (currentIndex < (int)tokens.size()) {
        return tokens[currentIndex];
    }
    return Token(TokenType::EOF_TOK, "EOF", -1);
}

Token Parser::advance() {
    Token current = peek();
    if (currentIndex < (int)tokens.size()) {
        currentIndex++;
    }
    return current;
}

bool Parser::isAtEnd() const {
    return currentIndex >= (int)tokens.size()
        || tokens[currentIndex].type == TokenType::EOF_TOK;
}

ParseTreeNode* Parser::match(TokenType expectedType) {
    Token current = peek();

    if (current.type == expectedType) {
        advance();
        std::string label;
        if (tokenHasValue(current.type)) {
            label = tokenTypeName(current.type) + " (" + current.value + ")";
        } else {
            label = tokenTypeName(current.type);
        }
        return new ParseTreeNode(label);
    }

    throw SyntaxError(
        current.line,
        current.col,
        tokenHasValue(current.type)
            ? tokenTypeName(current.type) + " (" + current.value + ")"
            : tokenTypeName(current.type),
        tokenTypeName(expectedType)
    );
}

ParseTreeNode* Parser::parse() {
    ParseTreeNode* root = parseProgram();
    return root;
}

ParseTreeNode* Parser::parseProgram() {
    ParseTreeNode* node = new ParseTreeNode("<program>");

    node->addChild(parseProgramHeader());
    node->addChild(parseDeclarationPart());
    node->addChild(parseCompoundStatement());
    node->addChild(match(TokenType::PERIOD));

    return node;
}

ParseTreeNode* Parser::parseProgramHeader() {
    ParseTreeNode* node = new ParseTreeNode("<program-header>");

    node->addChild(match(TokenType::PROGRAMSY));
    node->addChild(match(TokenType::IDENT));
    node->addChild(match(TokenType::SEMICOLON));

    return node;
}

ParseTreeNode* Parser::parseBlock() {
    ParseTreeNode* node = new ParseTreeNode("<block>");

    node->addChild(parseDeclarationPart());
    node->addChild(parseCompoundStatement());

    return node;
}

ParseTreeNode* Parser::parseCompoundStatement() {
    ParseTreeNode* node = new ParseTreeNode("<compound-statement>");

    node->addChild(match(TokenType::BEGINSY));
    node->addChild(match(TokenType::ENDSY));

    return node;
}
