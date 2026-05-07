#include "identifier.hpp"
#include <cctype>
#include <algorithm>

bool isIdentChar(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9');
}

TokenType classifyIdent(const std::string& w) {
    std::string lower = w;
    std::transform(lower.begin(), lower.end(), lower.begin(),
                   [](unsigned char c){ return std::tolower(c); });

    if (lower == "not")       return TokenType::NOTSY;
    if (lower == "and")       return TokenType::ANDSY;
    if (lower == "or")        return TokenType::ORSY;

    if (lower == "div")       return TokenType::IDIV;
    if (lower == "mod")       return TokenType::IMOD;

    if (lower == "const")     return TokenType::CONSTSY;
    if (lower == "type")      return TokenType::TYPESY;
    if (lower == "var")       return TokenType::VARSY;
    if (lower == "function")  return TokenType::FUNCTIONSY;
    if (lower == "procedure") return TokenType::PROCEDURESY;
    if (lower == "array")     return TokenType::ARRAYSY;
    if (lower == "record")    return TokenType::RECORDSY;
    if (lower == "program")   return TokenType::PROGRAMSY;

    if (lower == "begin")     return TokenType::BEGINSY;
    if (lower == "end")       return TokenType::ENDSY;
    if (lower == "if")        return TokenType::IFSY;
    if (lower == "then")      return TokenType::THENSY;
    if (lower == "else")      return TokenType::ELSESY;
    if (lower == "case")      return TokenType::CASESY;
    if (lower == "of")        return TokenType::OFSY;
    if (lower == "while")     return TokenType::WHILESY;
    if (lower == "do")        return TokenType::DOSY;
    if (lower == "for")       return TokenType::FORSY;
    if (lower == "to")        return TokenType::TOSY;
    if (lower == "downto")    return TokenType::DOWNTOSY;
    if (lower == "repeat")    return TokenType::REPEATSY;
    if (lower == "until")     return TokenType::UNTILSY;

    return TokenType::IDENT;
}
