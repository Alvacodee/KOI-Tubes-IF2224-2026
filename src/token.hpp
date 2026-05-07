#pragma once
#include <string>

enum class TokenType {
    // Literal
    INTCON, REALCON, CHARCON, STRING_TOK,

    // Operator aritmatika
    PLUS, MINUS, TIMES, IDIV, RDIV, IMOD,

    // Operator logika
    NOTSY, ANDSY, ORSY,

    // Operator perbandingan
    EQL, NEQ, GTR, GEQ, LSS, LEQ,

    // Tanda baca 
    LPARENT, RPARENT, LBRACK, RBRACK,
    COMMA, SEMICOLON, PERIOD, COLON, BECOMES,

    // Keyword deklarasi
    CONSTSY, TYPESY, VARSY, FUNCTIONSY, PROCEDURESY,
    ARRAYSY, RECORDSY, PROGRAMSY,

    // Keyword control flow
    BEGINSY, ENDSY, IFSY, THENSY, ELSESY,
    CASESY, OFSY, WHILESY, DOSY,
    FORSY, TOSY, DOWNTOSY, REPEATSY, UNTILSY,

    // Identifier 
    IDENT,

    // Komentar
    COMMENT,

    // Token tidak dikenal
    ERROR_TOK,

    // End of file
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