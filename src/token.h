#pragma once
#include <string>

// Semua jenis token yang dikenali oleh lexer Arion
enum class TokenType {
    // Literal
    INTCON, REALCON, CHARCON, STRING_TOK,

    // Operator aritmatika
    PLUS, MINUS, TIMES, IDIV, RDIV, IMOD,

    // Operator logika
    NOTSY, ANDSY, ORSY,

    // Operator perbandingan
    EQL, NEQ, GTR, GEQ, LSS, LEQ,

    // Tanda baca dan delimiter
    LPARENT, RPARENT, LBRACK, RBRACK,
    COMMA, SEMICOLON, PERIOD, COLON, BECOMES,

    // Keyword deklarasi
    CONSTSY, TYPESY, VARSY, FUNCTIONSY, PROCEDURESY,
    ARRAYSY, RECORDSY, PROGRAMSY,

    // Keyword control flow
    BEGINSY, ENDSY, IFSY, THENSY, ELSESY,
    CASESY, OFSY, WHILESY, DOSY,
    FORSY, TOSY, DOWNTOSY, REPEATSY, UNTILSY,

    // Identifier (nama variabel, fungsi, prosedur, tipe, dll)
    IDENT,

    // Komentar
    COMMENT,

    // Token tidak dikenal
    ERROR_TOK
};

struct Token {
    TokenType   type;
    std::string value;  // diisi untuk IDENT, literal, COMMENT, ERROR_TOK
    int         line;

    Token(TokenType t, std::string v, int l) : type(t), value(std::move(v)), line(l) {}
    Token(TokenType t, int l)               : type(t), value(""), line(l) {}
};

// Mengubah TokenType menjadi string untuk keperluan output
std::string tokenTypeName(TokenType type);

// Mengecek apakah token perlu mencetak value-nya di output
bool tokenHasValue(TokenType type);