#include "parser.hpp"
#include "node.hpp"
#include <sstream>

// =============================================================================
//   - parseExpression, parseSimpleExpression, parseTerm, parseFactor
//   - parseProcFuncCall, parseParameterList
//   - parseVariable, parseComponentVariable, parseIndexList
//   - Token navigation helpers
//   - printTree (output parse tree ke terminal/file)
// =============================================================================

// Token navigation
Parser::Parser(const std::vector<Token>& toks) : tokens(toks), pos(0) {}

Token& Parser::current() {
    return tokens[pos];
}

Token& Parser::peek(int offset) {
    size_t idx = pos + offset;
    if (idx >= tokens.size()) return tokens.back();
    return tokens[idx];
}

Token Parser::consume() {
    Token t = tokens[pos];
    if (pos + 1 < tokens.size()) pos++;
    return t;
}

// Consume token dan validasi tipenya. Lempar SyntaxError jika tidak cocok.
Token Parser::expect(TokenType type) {
    if (current().type != type) {
        std::string msg = "Syntax error baris " + std::to_string(current().line)
                        + ": expected " + tokenTypeName(type)
                        + ", got " + tokenTypeName(current().type);
        if (!current().value.empty()) msg += "(" + current().value + ")";
        throw SyntaxError(msg, current().line);
    }
    return consume();
}

bool Parser::check(TokenType type) const {
    return tokens[pos].type == type;
}

bool Parser::atEnd() const {
    return pos >= tokens.size()
        || tokens[pos].type == TokenType::ERROR_TOK;
}

// parseExpression
// aturan: simple-expression (relational-operator simple-expression)?
std::shared_ptr<ParseNode> Parser::parseExpression() {
    auto node = makeNode("<expression>");
    node->addChild(parseSimpleExpression());

    // Relational operator: == <> > >= < <=
    if (check(TokenType::EQL)  || check(TokenType::NEQ) ||
        check(TokenType::GTR)  || check(TokenType::GEQ) ||
        check(TokenType::LSS)  || check(TokenType::LEQ)) {

        auto opNode = makeNode("<relational-operator>");
        Token op = consume();
        opNode->addChild(makeLeaf(tokenTypeName(op.type)));
        node->addChild(opNode);
        node->addChild(parseSimpleExpression());
    }

    return node;
}

// parseSimpleExpression
// Aturan: (plus | minus)? term (additive-operator term)*
std::shared_ptr<ParseNode> Parser::parseSimpleExpression() {
    auto node = makeNode("<simple-expression>");

    // unary plus/minus opsional di awal
    if (check(TokenType::PLUS) || check(TokenType::MINUS)) {
        Token sign = consume();
        node->addChild(makeLeaf(tokenTypeName(sign.type)));
    }

    node->addChild(parseTerm());

    // additive operators: + - or
    while (check(TokenType::PLUS) || check(TokenType::MINUS) || check(TokenType::ORSY)) {
        auto opNode = makeNode("<additive-operator>");
        Token op = consume();
        opNode->addChild(makeLeaf(tokenTypeName(op.type)));
        node->addChild(opNode);
        node->addChild(parseTerm());
    }

    return node;
}

// parseTerm
// aturan: factor (multiplicative-operator factor)*
std::shared_ptr<ParseNode> Parser::parseTerm() {
    auto node = makeNode("<term>");
    node->addChild(parseFactor());

    // multiplicative operators: * / div mod and
    while (check(TokenType::TIMES) || check(TokenType::RDIV) ||
           check(TokenType::IDIV)  || check(TokenType::IMOD) ||
           check(TokenType::ANDSY)) {

        auto opNode = makeNode("<multiplicative-operator>");
        Token op = consume();
        opNode->addChild(makeLeaf(tokenTypeName(op.type)));
        node->addChild(opNode);
        node->addChild(parseFactor());
    }

    return node;
}

// ---------------------------------------------------------------------------
// parseFactor
// aturan: intcon | realcon | charcon | string | (lparent expression rparent)
//         | (notsy factor) | procedure/function-call | variable
//
// ambiguitas ident: bisa jadi variable biasa atau procedure/function-call.
// bedanya dilihat dari token berikutnya — kalau ada '(' berarti function call.
// ---------------------------------------------------------------------------
std::shared_ptr<ParseNode> Parser::parseFactor() {
    auto node = makeNode("<factor>");

    if (check(TokenType::INTCON)) {
        Token t = consume();
        node->addChild(makeLeaf("intcon", t.value));

    } else if (check(TokenType::REALCON)) {
        Token t = consume();
        node->addChild(makeLeaf("realcon", t.value));

    } else if (check(TokenType::CHARCON)) {
        Token t = consume();
        node->addChild(makeLeaf("charcon", t.value));

    } else if (check(TokenType::STRING_TOK)) {
        Token t = consume();
        node->addChild(makeLeaf("string", t.value));

    } else if (check(TokenType::LPARENT)) {
        // ekspresi dalam kurung: ( expression )
        consume(); // (
        node->addChild(makeLeaf("lparent"));
        node->addChild(parseExpression());
        expect(TokenType::RPARENT);
        node->addChild(makeLeaf("rparent"));

    } else if (check(TokenType::NOTSY)) {
        // NOT factor (unary logical negation)
        consume();
        node->addChild(makeLeaf("notsy"));
        node->addChild(parseFactor());

    } else if (check(TokenType::IDENT)) {
        // cek token berikutnya: kalau '(' → procedure/function-call
        // kalau '[' atau '.' -> component variable (array/record access)
        // selain itu -> identifier biasa (variable)
        if (peek().type == TokenType::LPARENT) {
            node->addChild(parseProcFuncCall());
        } else {
            node->addChild(parseVariable());
        }

    } else {
        std::string msg = "Syntax error baris " + std::to_string(current().line)
                        + ": unexpected token " + tokenTypeName(current().type)
                        + " in factor";
        throw SyntaxError(msg, current().line);
    }

    return node;
}

// parseProcFuncCall
// Aturan: ident (lparent parameter-list? rparent)?
std::shared_ptr<ParseNode> Parser::parseProcFuncCall() {
    auto node = makeNode("<procedure/function-call>");

    Token name = expect(TokenType::IDENT);
    node->addChild(makeLeaf("ident", name.value));

    // parameter list opsional
    if (check(TokenType::LPARENT)) {
        consume();
        node->addChild(makeLeaf("lparent"));

        // kalau langsung ')' maka parameter list kosong
        if (!check(TokenType::RPARENT)) {
            node->addChild(parseParameterList());
        }

        expect(TokenType::RPARENT);
        node->addChild(makeLeaf("rparent"));
    }

    return node;
}

// parseParameterList
// aturan: expression (comma expression)*
std::shared_ptr<ParseNode> Parser::parseParameterList() {
    auto node = makeNode("<parameter-list>");
    node->addChild(parseExpression());

    while (check(TokenType::COMMA)) {
        consume();
        node->addChild(makeLeaf("comma"));
        node->addChild(parseExpression());
    }

    return node;
}

// parseVariable
// aturan: ident | component-variable
// component-variable: variable[index-list] atau variable.ident
std::shared_ptr<ParseNode> Parser::parseVariable() {
    auto node = makeNode("<variable>");

    Token name = expect(TokenType::IDENT);
    auto identLeaf = makeLeaf("ident", name.value);

    // cek apakah ini component variable (array access atau record field)
    if (check(TokenType::LBRACK) || check(TokenType::PERIOD)) {
        // Bungkus ident sebagai base variable dulu
        auto baseVar = makeNode("<variable>");
        baseVar->addChild(identLeaf);
        node->addChild(parseComponentVariable(baseVar));
    } else {
        node->addChild(identLeaf);
    }

    return node;
}

// parseComponentVariable
// aturan: variable ([index-list] | .ident) — bisa berantai (someArr[1][2])
std::shared_ptr<ParseNode> Parser::parseComponentVariable(std::shared_ptr<ParseNode> varNode) {
    auto node = makeNode("<component-variable>");
    node->addChild(varNode);

    if (check(TokenType::LBRACK)) {
        consume();
        node->addChild(makeLeaf("lbrack"));
        node->addChild(parseIndexList());
        expect(TokenType::RBRACK);
        node->addChild(makeLeaf("rbrack"));

        // Berantai: someArr[1][2] atau someArr[1].field
        if (check(TokenType::LBRACK) || check(TokenType::PERIOD))
            return parseComponentVariable(node);

    } else if (check(TokenType::PERIOD)) {
        consume();
        node->addChild(makeLeaf("period"));
        Token field = expect(TokenType::IDENT);
        node->addChild(makeLeaf("ident", field.value));

        // berantai: someRecord.field.subField
        if (check(TokenType::LBRACK) || check(TokenType::PERIOD))
            return parseComponentVariable(node);
    }

    return node;
}

// parseIndexList
// aturan: (intcon | charcon | ident) (comma index-list)*
std::shared_ptr<ParseNode> Parser::parseIndexList() {
    auto node = makeNode("<index-list>");

    auto parseOneIndex = [&]() -> std::shared_ptr<ParseNode> {
        if (check(TokenType::INTCON)) {
            Token t = consume();
            return makeLeaf("intcon", t.value);
        } else if (check(TokenType::CHARCON)) {
            Token t = consume();
            return makeLeaf("charcon", t.value);
        } else if (check(TokenType::IDENT)) {
            Token t = consume();
            return makeLeaf("ident", t.value);
        } else {
            std::string msg = "Syntax error baris " + std::to_string(current().line)
                            + ": expected index (intcon/charcon/ident)";
            throw SyntaxError(msg, current().line);
        }
    };

    node->addChild(parseOneIndex());
    while (check(TokenType::COMMA)) {
        consume();
        node->addChild(makeLeaf("comma"));
        node->addChild(parseOneIndex());
    }

    return node;
}

// ---------------------------------------------------------------------------
// printTree — print parse tree ke ostream dengan format ASCII tree
//
// contoh output:
// <program>
// ├── <program-header>
// │   ├── programsy
// │   └── ident(Hello)
// └── period
// ---------------------------------------------------------------------------
void printTree(const std::shared_ptr<ParseNode>& node, std::ostream& out,
               const std::string& prefix, bool isLast) {
    if (!node) return;

    out << prefix;
    out << (isLast ? "└── " : "├── ");
    out << node->label << "\n";

    std::string childPrefix = prefix + (isLast ? "    " : "│   ");
    for (size_t i = 0; i < node->children.size(); i++) {
        bool last = (i == node->children.size() - 1);
        printTree(node->children[i], out, childPrefix, last);
    }
}
