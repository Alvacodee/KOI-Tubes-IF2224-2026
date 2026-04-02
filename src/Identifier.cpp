#include "Identifier.h"
#include <cctype>

// Identifier di Arion: diawali huruf, diikuti huruf/angka bebas, case-insensitive.
// Contoh valid: x, PI, MyInt, h4g, identIniSangatPanjang24tetapiTetapValid

bool isIdentChar(char c) {
    return isalpha(static_cast<unsigned char>(c)) || isdigit(static_cast<unsigned char>(c));
}

// Semua perbandingan dilakukan dalam lowercase karena Arion bersifat case-insensitive.
// Caller wajib melowercasekan buffer sebelum memanggil fungsi ini.
TokenType classifyIdent(const std::string& w) {
    // Logical operators
    if (w == "not")       return TokenType::NOTSY;
    if (w == "and")       return TokenType::ANDSY;
    if (w == "or")        return TokenType::ORSY;

    // Arithmetic operators
    if (w == "div")       return TokenType::IDIV;
    if (w == "mod")       return TokenType::IMOD;

    // Declaration keywords
    if (w == "const")     return TokenType::CONSTSY;
    if (w == "type")      return TokenType::TYPESY;
    if (w == "var")       return TokenType::VARSY;
    if (w == "function")  return TokenType::FUNCTIONSY;
    if (w == "procedure") return TokenType::PROCEDURESY;
    if (w == "array")     return TokenType::ARRAYSY;
    if (w == "record")    return TokenType::RECORDSY;
    if (w == "program")   return TokenType::PROGRAMSY;

    // Control flow keywords
    if (w == "begin")     return TokenType::BEGINSY;
    if (w == "end")       return TokenType::ENDSY;
    if (w == "if")        return TokenType::IFSY;
    if (w == "then")      return TokenType::THENSY;
    if (w == "else")      return TokenType::ELSESY;
    if (w == "case")      return TokenType::CASESY;
    if (w == "of")        return TokenType::OFSY;
    if (w == "while")     return TokenType::WHILESY;
    if (w == "do")        return TokenType::DOSY;
    if (w == "for")       return TokenType::FORSY;
    if (w == "to")        return TokenType::TOSY;
    if (w == "downto")    return TokenType::DOWNTOSY;
    if (w == "repeat")    return TokenType::REPEATSY;
    if (w == "until")     return TokenType::UNTILSY;

    return TokenType::IDENT;
}