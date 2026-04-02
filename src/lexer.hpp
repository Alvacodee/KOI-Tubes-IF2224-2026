#pragma once
#include <iostream>
#include <string>
#include "token.hpp"
#include "literal.hpp"
#include "identifier.hpp"
#include "operator.hpp"

class Lexer {
public:
    explicit Lexer(std::istream& input);

    Token getNextToken();

    bool isEOF() const;

private:
    std::istream& inputStream;
    int currentLine;
    char currentChar;
    bool eofReached;

    void readNextChar();

    void skipWhitespace();
};