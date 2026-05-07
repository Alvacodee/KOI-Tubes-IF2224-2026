#include "parser.hpp"

SyntaxError::SyntaxError(int line, int col, const std::string& found, const std::string& expected)
    : std::runtime_error(
        "Syntax Error at line " + std::to_string(line) +
        ", col " + std::to_string(col) +
        ": found " + found +
        ", expected " + expected),
      line(line), col(col), found(found), expected(expected) {}

Parser::Parser(const std::vector<Token>& tokens)
    : tokens(tokens), currentIndex(0) {}

Token Parser::peek() const {
    if (currentIndex < (int)tokens.size())
        return tokens[currentIndex];
    return Token(TokenType::EOF_TOK, "", -1, -1);
}

Token Parser::advance() {
    Token t = tokens[currentIndex];
    currentIndex++;
    return t;
}

bool Parser::isAtEnd() const {
    return currentIndex >= (int)tokens.size();
}

ParseTreeNode* Parser::match(TokenType expectedType) {
    if (isAtEnd()) {
        throw SyntaxError(-1, -1, "EOF", tokenTypeName(expectedType));
    }
    Token t = peek();
    if (t.type != expectedType) {
        throw SyntaxError(t.line, t.col, tokenTypeName(t.type), tokenTypeName(expectedType));
    }
    advance();
    std::string nodeName = tokenTypeName(t.type);
    if (!t.value.empty()) {
        nodeName += "(" + t.value + ")";
    }
    return new ParseTreeNode(nodeName);
}

ParseTreeNode* Parser::parse() {
    return parseProgram();
}

ParseTreeNode* Parser::parseProgram() {
    ParseTreeNode* node = new ParseTreeNode("<program>");
    node->addChild(parseProgramHeader());
    node->addChild(parseBlock());
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
    node->addChild(parseStatementList());
    node->addChild(match(TokenType::ENDSY));
    return node;
}

ParseTreeNode* Parser::parseStatementList() {
    ParseTreeNode* node = new ParseTreeNode("<statement-list>");
    node->addChild(parseStatement());
    while (peek().type == TokenType::SEMICOLON) {
        node->addChild(match(TokenType::SEMICOLON));
        if (peek().type != TokenType::ENDSY && peek().type != TokenType::UNTILSY) {
            node->addChild(parseStatement());
        }
    }
    return node;
}

ParseTreeNode* Parser::parseStatement() {
    TokenType t = peek().type;

    if (t == TokenType::BEGINSY) {
        return parseCompoundStatement(); 
    } else if (t == TokenType::IFSY) {
        return parseIfStatement();
    } else if (t == TokenType::CASESY) {
        return parseCaseStatement();
    } else if (t == TokenType::WHILESY) {
        return parseWhileStatement();
    } else if (t == TokenType::REPEATSY) {
        return parseRepeatStatement();
    } else if (t == TokenType::FORSY) {
        return parseForStatement();
    } else if (t == TokenType::IDENT) {
        if (currentIndex + 1 < (int)tokens.size() && tokens[currentIndex + 1].type == TokenType::LPARENT) {
            return parseProcFuncCall();
        } else {
            return parseAssignmentStatement();
        }
    }

    return new ParseTreeNode("<empty-statement>");
}

ParseTreeNode* Parser::parseAssignmentStatement() {
    ParseTreeNode* node = new ParseTreeNode("<assignment-statement>");
    node->addChild(parseVariable());
    node->addChild(match(TokenType::BECOMES));
    node->addChild(parseExpression());
    return node;
}

ParseTreeNode* Parser::parseIfStatement() {
    ParseTreeNode* node = new ParseTreeNode("<if-statement>");
    node->addChild(match(TokenType::IFSY));
    node->addChild(parseExpression());
    node->addChild(match(TokenType::THENSY));
    node->addChild(parseStatement());
    if (peek().type == TokenType::ELSESY) {
        node->addChild(match(TokenType::ELSESY));
        node->addChild(parseStatement());
    }
    return node;
}

ParseTreeNode* Parser::parseCaseStatement() {
    ParseTreeNode* node = new ParseTreeNode("<case-statement>");
    node->addChild(match(TokenType::CASESY));
    node->addChild(parseExpression());
    node->addChild(match(TokenType::OFSY));
    node->addChild(parseCaseBlock());
    node->addChild(match(TokenType::ENDSY));
    return node;
}

ParseTreeNode* Parser::parseCaseBlock() {
    ParseTreeNode* node = new ParseTreeNode("<case-block>");

    node->addChild(parseConstant());

    while (peek().type == TokenType::COMMA) {
        node->addChild(match(TokenType::COMMA));
        node->addChild(parseConstant());
    }

    node->addChild(match(TokenType::COLON));
    node->addChild(parseStatement());

    while (peek().type == TokenType::SEMICOLON) {
        node->addChild(match(TokenType::SEMICOLON));
        if (peek().type != TokenType::ENDSY) {
            node->addChild(parseConstant());
            while (peek().type == TokenType::COMMA) {
                node->addChild(match(TokenType::COMMA));
                node->addChild(parseConstant());
            }
            node->addChild(match(TokenType::COLON));
            node->addChild(parseStatement());
        }
    }
    return node;
}

ParseTreeNode* Parser::parseWhileStatement() {
    ParseTreeNode* node = new ParseTreeNode("<while-statement>");
    node->addChild(match(TokenType::WHILESY));
    node->addChild(parseExpression());
    node->addChild(match(TokenType::DOSY));
    node->addChild(parseStatement());
    return node;
}

ParseTreeNode* Parser::parseRepeatStatement() {
    ParseTreeNode* node = new ParseTreeNode("<repeat-statement>");
    node->addChild(match(TokenType::REPEATSY));
    node->addChild(parseStatementList());
    node->addChild(match(TokenType::UNTILSY));
    node->addChild(parseExpression());
    return node;
}

ParseTreeNode* Parser::parseForStatement() {
    ParseTreeNode* node = new ParseTreeNode("<for-statement>");
    node->addChild(match(TokenType::FORSY));
    node->addChild(match(TokenType::IDENT));
    node->addChild(match(TokenType::BECOMES));
    node->addChild(parseExpression());
    if (peek().type == TokenType::TOSY) {
        node->addChild(match(TokenType::TOSY));
    } else {
        node->addChild(match(TokenType::DOWNTOSY));
    }
    node->addChild(parseExpression());
    node->addChild(match(TokenType::DOSY));
    node->addChild(parseStatement());
    return node;
}