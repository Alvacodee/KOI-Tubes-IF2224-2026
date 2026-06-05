#pragma once
#include <string>

enum class TokenType {

    INTCON, REALCON, CHARCON, STRING_TOK,

    PLUS, MINUS, TIMES, IDIV, RDIV, IMOD,

    NOTSY, ANDSY, ORSY,

    EQL, NEQ, GTR, GEQ, LSS, LEQ,

    LPARENT, RPARENT, LBRACK, RBRACK,
    COMMA, SEMICOLON, PERIOD, COLON, BECOMES,

    CONSTSY, TYPESY, VARSY, FUNCTIONSY, PROCEDURESY,
    ARRAYSY, RECORDSY, PROGRAMSY,

    BEGINSY, ENDSY, IFSY, THENSY, ELSESY,
    CASESY, OFSY, WHILESY, DOSY,
    FORSY, TOSY, DOWNTOSY, REPEATSY, UNTILSY,

    IDENT,

    COMMENT,

    ERROR_TOK,

    EOF_TOK
};

struct Token {
    TokenType   type;
    std::string value;
    int         line;
    int         col;

    Token(TokenType t, std::string v, int l, int c = 0)
        : type(t), value(std::move(v)), line(l), col(c) {}
    Token(TokenType t, int l, int c = 0)
        : type(t), value(""), line(l), col(c) {}
};

std::string tokenTypeName(TokenType type);

bool tokenHasValue(TokenType type);