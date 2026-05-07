#include "parser.hpp"

ParseTreeNode* Parser::parseExpression() {
    ParseTreeNode* node = new ParseTreeNode("<expression>");
    node->addChild(parseSimpleExpression());

    TokenType t = peek().type;
    if (t == TokenType::EQL || t == TokenType::NEQ ||
        t == TokenType::GTR || t == TokenType::GEQ ||
        t == TokenType::LSS || t == TokenType::LEQ) {

        ParseTreeNode* opNode = new ParseTreeNode("<relational-operator>");
        opNode->addChild(match(t));
        node->addChild(opNode);

        node->addChild(parseSimpleExpression());
    }

    return node;
}

ParseTreeNode* Parser::parseSimpleExpression() {
    ParseTreeNode* node = new ParseTreeNode("<simple-expression>");

    TokenType t = peek().type;
    if (t == TokenType::PLUS || t == TokenType::MINUS) {
        node->addChild(match(t));
    }

    node->addChild(parseTerm());

    t = peek().type;
    while (t == TokenType::PLUS || t == TokenType::MINUS || t == TokenType::ORSY) {
        ParseTreeNode* opNode = new ParseTreeNode("<additive-operator>");
        opNode->addChild(match(t));
        node->addChild(opNode);

        node->addChild(parseTerm());
        t = peek().type;
    }

    return node;
}

ParseTreeNode* Parser::parseTerm() {
    ParseTreeNode* node = new ParseTreeNode("<term>");
    node->addChild(parseFactor());

    TokenType t = peek().type;
    while (t == TokenType::TIMES || t == TokenType::RDIV ||
           t == TokenType::IDIV  || t == TokenType::IMOD ||
           t == TokenType::ANDSY) {

        ParseTreeNode* opNode = new ParseTreeNode("<multiplicative-operator>");
        opNode->addChild(match(t));
        node->addChild(opNode);

        node->addChild(parseFactor());
        t = peek().type;
    }

    return node;
}

ParseTreeNode* Parser::parseFactor() {
    ParseTreeNode* node = new ParseTreeNode("<factor>");
    TokenType t = peek().type;

    if (t == TokenType::INTCON || t == TokenType::REALCON ||
        t == TokenType::CHARCON || t == TokenType::STRING_TOK) {
        node->addChild(match(t));

    } else if (t == TokenType::LPARENT) {
        node->addChild(match(TokenType::LPARENT));
        node->addChild(parseExpression());
        node->addChild(match(TokenType::RPARENT));

    } else if (t == TokenType::NOTSY) {
        node->addChild(match(TokenType::NOTSY));
        node->addChild(parseFactor());

    } else if (t == TokenType::IDENT) {
        if (currentIndex + 1 < (int)tokens.size() && tokens[currentIndex + 1].type == TokenType::LPARENT) {
            node->addChild(parseProcFuncCall());
        } else {
            node->addChild(parseVariable());
        }

    } else {
        Token errToken = peek();
        throw SyntaxError(errToken.line, errToken.col, tokenTypeName(errToken.type), "factor (intcon/realcon/ident/string/etc)");
    }

    return node;
}

ParseTreeNode* Parser::parseProcFuncCall() {
    ParseTreeNode* node = new ParseTreeNode("<procedure/function-call>");

    node->addChild(match(TokenType::IDENT));

    if (peek().type == TokenType::LPARENT) {
        node->addChild(match(TokenType::LPARENT));

        if (peek().type != TokenType::RPARENT) {
            node->addChild(parseParameterList());
        }

        node->addChild(match(TokenType::RPARENT));
    }

    return node;
}

ParseTreeNode* Parser::parseParameterList() {
    ParseTreeNode* node = new ParseTreeNode("<parameter-list>");
    node->addChild(parseExpression());

    while (peek().type == TokenType::COMMA) {
        node->addChild(match(TokenType::COMMA));
        node->addChild(parseExpression());
    }

    return node;
}

ParseTreeNode* Parser::parseVariable() {
    ParseTreeNode* node = new ParseTreeNode("<variable>");

    ParseTreeNode* identNode = match(TokenType::IDENT);

    TokenType t = peek().type;
    if (t == TokenType::LBRACK || t == TokenType::PERIOD) {
        ParseTreeNode* baseVar = new ParseTreeNode("<variable>");
        baseVar->addChild(identNode);
        node->addChild(parseComponentVariable(baseVar));
    } else {
        node->addChild(identNode);
    }

    return node;
}

ParseTreeNode* Parser::parseComponentVariable(ParseTreeNode* varNode) {
    ParseTreeNode* node = new ParseTreeNode("<component-variable>");
    node->addChild(varNode);

    TokenType t = peek().type;
    if (t == TokenType::LBRACK) {
        node->addChild(match(TokenType::LBRACK));
        node->addChild(parseIndexList());
        node->addChild(match(TokenType::RBRACK));

        t = peek().type;
        if (t == TokenType::LBRACK || t == TokenType::PERIOD) {
            return parseComponentVariable(node);
        }

    } else if (t == TokenType::PERIOD) {
        node->addChild(match(TokenType::PERIOD));
        node->addChild(match(TokenType::IDENT));

        t = peek().type;
        if (t == TokenType::LBRACK || t == TokenType::PERIOD) {
            return parseComponentVariable(node);
        }
    }

    return node;
}

ParseTreeNode* Parser::parseIndexList() {
    ParseTreeNode* node = new ParseTreeNode("<index-list>");

    auto parseOneIndex = [&]() -> ParseTreeNode* {
        TokenType t = peek().type;
        if (t == TokenType::INTCON || t == TokenType::CHARCON || t == TokenType::IDENT) {
            return match(t);
        } else {
            Token errToken = peek();
            throw SyntaxError(errToken.line, errToken.col, tokenTypeName(errToken.type), "index (intcon/charcon/ident)");
        }
    };

    node->addChild(parseOneIndex());
    while (peek().type == TokenType::COMMA) {
        node->addChild(match(TokenType::COMMA));
        node->addChild(parseOneIndex());
    }

    return node;
}
