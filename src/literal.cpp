#include "literal.hpp"

Token handleNumber(std::istream& inputFile, char firstChar, int currentLine) {
    std::string value = "";
    value += firstChar;
    
    char nextChar;
    bool isReal = false;

    while (inputFile.get(nextChar)) {
        if (nextChar >= '0' && nextChar <= '9') {
            value += nextChar;
        } else if (nextChar == '.') {
            char afterDot = inputFile.peek();
            if (afterDot >= '0' && afterDot <= '9') {
                isReal = true;
                value += nextChar; 
            } else {
                inputFile.unget(); 
                break;
            }
        } else {
            inputFile.unget();
            break;
        }
    }

    TokenType type = isReal ? TokenType::REALCON : TokenType::INTCON;
    return Token(type, value, currentLine);
}

Token handleStringOrChar(std::istream& inputFile, int currentLine) {
    std::string value = "'"; 
    char nextChar;

    while (inputFile.get(nextChar)) {
        value += nextChar;
        if (nextChar == '\'') {
            break;
        }
    }
    TokenType type = (value.length() == 3) ? TokenType::CHARCON : TokenType::STRING_TOK;
    return Token(type, value, currentLine);
}