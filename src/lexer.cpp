#include "lexer.hpp"
#include <iostream>

Lexer::Lexer(std::istream& input) : inputStream(input), currentLine(1), eofReached(false) {
    if (!inputStream.get(currentChar)) {
        eofReached = true;
        currentChar = EOF;
    }
}

void Lexer::readNextChar() {
    if (!inputStream.get(currentChar)) {
        eofReached = true;
        currentChar = EOF;
    }
}

void Lexer::skipWhitespace() {
    while (!eofReached && (currentChar == ' ' || currentChar == '\t' || currentChar == '\r' || currentChar == '\n')) {
        if (currentChar == '\n') currentLine++;
        readNextChar();
    }
}

Token Lexer::getNextToken() {
    skipWhitespace();

    if (eofReached) {
        return Token(TokenType::ERROR_TOK, "EOF", currentLine);
    }

    // Komentar
    if (currentChar == '{' || (currentChar == '(')) {
        bool isCommentCandidate = false;
        if (currentChar == '{') isCommentCandidate = true;
        else {
            readNextChar();
            if (currentChar == '*') isCommentCandidate = true;
            else {
                return Token(TokenType::LPARENT, "(", currentLine);
            }
        }

        if (isCommentCandidate) {
            std::string content = "";
            bool closed = false;
            while (readNextChar(), !eofReached) {
                if (currentChar == '\n') currentLine++;
                
                if (currentChar == '}') {
                    closed = true;
                    readNextChar();
                    break;
                }
                if (currentChar == '*') {
                    readNextChar();
                    if (currentChar == ')') {
                        closed = true;
                        readNextChar();
                        break;
                    }
                    content += '*';
                }
                if (!eofReached) content += currentChar;
            }

            if (!closed) {
                std::cerr << "Error: Komentar tidak tertutup pada baris " << currentLine-1 << std::endl;
                return getNextToken();
            }
            return Token(TokenType::COMMENT, content, currentLine);
        }
    }


    bool isNegative = false;
    if (currentChar == '-') {
        readNextChar();
        if (currentChar >= '0' && currentChar <= '9') isNegative = true;
        else {
            return Token(TokenType::MINUS, "-", currentLine);
        }
    }

    if (currentChar >= '0' && currentChar <= '9') {
        std::string numBuffer = isNegative ? "-" : "";
        int dotCount = 0;

        while (!eofReached && ((currentChar >= '0' && currentChar <= '9') || currentChar == '.')) {
            if (currentChar == '.') {
                dotCount++;
                if (dotCount > 1) break; 
            }
            numBuffer += currentChar;
            readNextChar();
            
            if (currentChar == 'e' || currentChar == 'E') {
                while (!eofReached && !isspace(currentChar) && !isOperatorChar(currentChar)) {
                    numBuffer += currentChar;
                    readNextChar();
                }
                return Token(TokenType::ERROR_TOK, numBuffer, currentLine);
            }
        }

        TokenType type = (dotCount == 1) ? TokenType::REALCON : TokenType::INTCON;
        return Token(type, numBuffer, currentLine);
    }

    if ((currentChar >= 'a' && currentChar <= 'z') || (currentChar >= 'A' && currentChar <= 'Z')) {
        std::string identBuffer = "";
        while (!eofReached && isIdentChar(currentChar)) {
            identBuffer += currentChar;
            readNextChar();
        }
        return Token(classifyIdent(identBuffer), identBuffer, currentLine);
    }

    if (currentChar == '\'') {
        std::string strBuffer = "'";
        while (readNextChar(), !eofReached && currentChar != '\'') {
            strBuffer += currentChar;
        }
        strBuffer += currentChar; 
        readNextChar();
        TokenType type = (strBuffer.length() == 3) ? TokenType::CHARCON : TokenType::STRING_TOK;
        return Token(type, strBuffer, currentLine);
    }

    if (isOperatorChar(currentChar)) {
        std::string opBuffer = "";
        opBuffer += currentChar;
        char first = currentChar;
        readNextChar();

        if (first == ':' && currentChar == '=') {
            opBuffer += currentChar;
            readNextChar();
        } else if (first == '<' && (currentChar == '=' || currentChar == '>')) {
            opBuffer += currentChar;
            readNextChar();
        } else if (first == '>' && currentChar == '=') {
            opBuffer += currentChar;
            readNextChar();
        }
        
        return Token(classifyOperator(opBuffer), opBuffer, currentLine);
    }

    std::string unknown(1, currentChar);
    readNextChar();
    return Token(TokenType::ERROR_TOK, unknown, currentLine);
}

bool Lexer::isEOF() const {
    return eofReached;
}