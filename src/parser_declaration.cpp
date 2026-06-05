#include "parser.hpp"

ParseTreeNode* Parser::parseDeclarationPart() {
    ParseTreeNode* node = new ParseTreeNode("<declaration-part>");

    while (peek().type == TokenType::CONSTSY) {
        node->addChild(parseConstDeclaration());
    }

    while (peek().type == TokenType::TYPESY) {
        node->addChild(parseTypeDeclaration());
    }

    while (peek().type == TokenType::VARSY) {
        node->addChild(parseVarDeclaration());
    }

    while (peek().type == TokenType::PROCEDURESY || peek().type == TokenType::FUNCTIONSY) {
        node->addChild(parseSubprogramDeclaration());
    }

    return node;
}

ParseTreeNode* Parser::parseConstDeclaration() {
    ParseTreeNode* node = new ParseTreeNode("<const-declaration>");
    node->addChild(match(TokenType::CONSTSY));

    do {
        node->addChild(match(TokenType::IDENT));
        node->addChild(match(TokenType::EQL));
        node->addChild(parseConstant());
        node->addChild(match(TokenType::SEMICOLON));
    } while (peek().type == TokenType::IDENT);

    return node;
}

ParseTreeNode* Parser::parseConstant() {
    ParseTreeNode* node = new ParseTreeNode("<constant>");
    TokenType t = peek().type;

    if (t == TokenType::PLUS || t == TokenType::MINUS) {
        node->addChild(match(t));
        if (peek().type == TokenType::IDENT) node->addChild(match(TokenType::IDENT));
        else if (peek().type == TokenType::INTCON) node->addChild(match(TokenType::INTCON));
        else if (peek().type == TokenType::REALCON) node->addChild(match(TokenType::REALCON));
    } else if (t == TokenType::IDENT) {
        node->addChild(match(TokenType::IDENT));
    } else if (t == TokenType::INTCON) {
        node->addChild(match(TokenType::INTCON));
    } else if (t == TokenType::REALCON) {
        node->addChild(match(TokenType::REALCON));
    } else if (t == TokenType::CHARCON) {
        node->addChild(match(TokenType::CHARCON));
    } else if (t == TokenType::STRING_TOK) {
        node->addChild(match(TokenType::STRING_TOK));
    }
    return node;
}

ParseTreeNode* Parser::parseTypeDeclaration() {
    ParseTreeNode* node = new ParseTreeNode("<type-declaration>");
    node->addChild(match(TokenType::TYPESY));

    do {
        node->addChild(match(TokenType::IDENT));
        node->addChild(match(TokenType::EQL));
        node->addChild(parseType());
        node->addChild(match(TokenType::SEMICOLON));
    } while (peek().type == TokenType::IDENT);

    return node;
}

ParseTreeNode* Parser::parseVarDeclaration() {
    ParseTreeNode* node = new ParseTreeNode("<var-declaration>");
    node->addChild(match(TokenType::VARSY));

    do {
        node->addChild(parseIdentifierList());
        node->addChild(match(TokenType::COLON));
        node->addChild(parseType());
        node->addChild(match(TokenType::SEMICOLON));
    } while (peek().type == TokenType::IDENT);

    return node;
}

ParseTreeNode* Parser::parseIdentifierList() {
    ParseTreeNode* node = new ParseTreeNode("<identifier-list>");
    node->addChild(match(TokenType::IDENT));
    while (peek().type == TokenType::COMMA) {
        node->addChild(match(TokenType::COMMA));
        node->addChild(match(TokenType::IDENT));
    }
    return node;
}

ParseTreeNode* Parser::parseType() {
    ParseTreeNode* node = new ParseTreeNode("<type>");
    TokenType t = peek().type;

    if (t == TokenType::ARRAYSY) {
        node->addChild(parseArrayType());
    } else if (t == TokenType::LPARENT) {
        node->addChild(parseEnumerated());
    } else if (t == TokenType::RECORDSY) {
        node->addChild(parseRecordType());
    } else if (t == TokenType::PLUS || t == TokenType::MINUS || t == TokenType::INTCON ||
               t == TokenType::REALCON || t == TokenType::CHARCON || t == TokenType::STRING_TOK) {
        node->addChild(parseRange());
    } else if (t == TokenType::IDENT) {
        if (currentIndex + 1 < (int)tokens.size() && tokens[currentIndex + 1].type == TokenType::PERIOD) {
            node->addChild(parseRange());
        } else {
            node->addChild(match(TokenType::IDENT));
        }
    }
    return node;
}

ParseTreeNode* Parser::parseArrayType() {
    ParseTreeNode* node = new ParseTreeNode("<array-type>");
    node->addChild(match(TokenType::ARRAYSY));
    node->addChild(match(TokenType::LBRACK));

    if (peek().type == TokenType::IDENT) {
        if (currentIndex + 1 < (int)tokens.size() && tokens[currentIndex + 1].type == TokenType::PERIOD) {
            node->addChild(parseRange());
        } else {
            node->addChild(match(TokenType::IDENT));
        }
    } else {
        node->addChild(parseRange());
    }

    node->addChild(match(TokenType::RBRACK));
    node->addChild(match(TokenType::OFSY));
    node->addChild(parseType());
    return node;
}

ParseTreeNode* Parser::parseRange() {
    ParseTreeNode* node = new ParseTreeNode("<range>");
    node->addChild(parseConstant());
    node->addChild(match(TokenType::PERIOD));
    node->addChild(match(TokenType::PERIOD));
    node->addChild(parseConstant());
    return node;
}

ParseTreeNode* Parser::parseEnumerated() {
    ParseTreeNode* node = new ParseTreeNode("<enumerated>");
    node->addChild(match(TokenType::LPARENT));
    node->addChild(match(TokenType::IDENT));
    while (peek().type == TokenType::COMMA) {
        node->addChild(match(TokenType::COMMA));
        node->addChild(match(TokenType::IDENT));
    }
    node->addChild(match(TokenType::RPARENT));
    return node;
}

ParseTreeNode* Parser::parseRecordType() {
    ParseTreeNode* node = new ParseTreeNode("<record-type>");
    node->addChild(match(TokenType::RECORDSY));
    node->addChild(parseFieldList());
    node->addChild(match(TokenType::ENDSY));
    return node;
}

ParseTreeNode* Parser::parseFieldList() {
    ParseTreeNode* node = new ParseTreeNode("<field-list>");
    node->addChild(parseFieldPart());
    while (peek().type == TokenType::SEMICOLON) {
        node->addChild(match(TokenType::SEMICOLON));
        if (peek().type == TokenType::IDENT) {
            node->addChild(parseFieldPart());
        } else {
            break;
        }
    }
    return node;
}

ParseTreeNode* Parser::parseFieldPart() {
    ParseTreeNode* node = new ParseTreeNode("<field-part>");
    node->addChild(parseIdentifierList());
    node->addChild(match(TokenType::COLON));
    node->addChild(parseType());
    return node;
}

ParseTreeNode* Parser::parseSubprogramDeclaration() {
    ParseTreeNode* node = new ParseTreeNode("<subprogram-declaration>");
    if (peek().type == TokenType::PROCEDURESY) {
        node->addChild(parseProcedureDeclaration());
    } else {
        node->addChild(parseFunctionDeclaration());
    }
    return node;
}

ParseTreeNode* Parser::parseProcedureDeclaration() {
    ParseTreeNode* node = new ParseTreeNode("<procedure-declaration>");
    node->addChild(match(TokenType::PROCEDURESY));
    node->addChild(match(TokenType::IDENT));

    if (peek().type == TokenType::LPARENT) {
        node->addChild(parseFormalParameterList());
    }

    node->addChild(match(TokenType::SEMICOLON));
    node->addChild(parseBlock());
    node->addChild(match(TokenType::SEMICOLON));
    return node;
}

ParseTreeNode* Parser::parseFunctionDeclaration() {
    ParseTreeNode* node = new ParseTreeNode("<function-declaration>");
    node->addChild(match(TokenType::FUNCTIONSY));
    node->addChild(match(TokenType::IDENT));

    if (peek().type == TokenType::LPARENT) {
        node->addChild(parseFormalParameterList());
    }

    node->addChild(match(TokenType::COLON));
    node->addChild(match(TokenType::IDENT));
    node->addChild(match(TokenType::SEMICOLON));
    node->addChild(parseBlock());
    node->addChild(match(TokenType::SEMICOLON));
    return node;
}

ParseTreeNode* Parser::parseFormalParameterList() {
    ParseTreeNode* node = new ParseTreeNode("<formal-parameter-list>");
    node->addChild(match(TokenType::LPARENT));
    node->addChild(parseParameterGroup());

    while (peek().type == TokenType::SEMICOLON) {
        node->addChild(match(TokenType::SEMICOLON));
        node->addChild(parseParameterGroup());
    }

    node->addChild(match(TokenType::RPARENT));
    return node;
}

ParseTreeNode* Parser::parseParameterGroup() {
    ParseTreeNode* node = new ParseTreeNode("<parameter-group>");
    node->addChild(parseIdentifierList());
    node->addChild(match(TokenType::COLON));

    if (peek().type == TokenType::ARRAYSY) {
        node->addChild(parseArrayType());
    } else {
        node->addChild(match(TokenType::IDENT));
    }
    return node;
}