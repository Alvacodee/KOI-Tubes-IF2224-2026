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
    while (!eofReached && (currentChar == ' ' || currentChar == '\t' ||
                           currentChar == '\r' || currentChar == '\n')) {
        if (currentChar == '\n') currentLine++;
        readNextChar();
    }
}

Token Lexer::getNextToken() {
    skipWhitespace();

    if (eofReached) {
        return Token(TokenType::ERROR_TOK, "EOF", currentLine);
    }

    if (currentChar == '{') {
        std::string content;
        bool closed = false;
        while (readNextChar(), !eofReached) {
            if (currentChar == '\n') currentLine++;
            if (currentChar == '}') { closed = true; readNextChar(); break; }
            content += currentChar;
        }
        if (!closed) std::cerr << "Error: komentar tidak tertutup\n";
        return Token(TokenType::COMMENT, content, currentLine);
    }

    if (currentChar == '(') {
        readNextChar();
        if (currentChar == '*') {

            std::string content;
            bool closed = false;
            while (!eofReached) {
                readNextChar();
                if (eofReached) break;
                if (currentChar == '\n') currentLine++;
                if (currentChar == '*') {
                    readNextChar();
                    if (currentChar == ')') { closed = true; readNextChar(); break; }
                    content += '*';
                }
                if (!eofReached) content += currentChar;
            }
            if (!closed) std::cerr << "Error: komentar tidak tertutup\n";
            return Token(TokenType::COMMENT, content, currentLine);
        }

        return Token(TokenType::LPARENT, "(", currentLine);
    }

    if (currentChar >= '0' && currentChar <= '9') {
        std::string numBuf;
        int dotCount = 0;
        while (!eofReached && ((currentChar >= '0' && currentChar <= '9') || currentChar == '.')) {
            if (currentChar == '.') {

                int next = inputStream.peek();
                if (next == '.') break;
                dotCount++;
                if (dotCount > 1) break;
                numBuf += '.';
                readNextChar();

                if (!(currentChar >= '0' && currentChar <= '9')) {
                    numBuf.pop_back();
                    dotCount--;
                    break;
                }
                continue;
            }
            numBuf += currentChar;
            readNextChar();

            if (currentChar == 'e' || currentChar == 'E') {
                while (!eofReached && !isspace(currentChar) && !isOperatorChar(currentChar)) {
                    numBuf += currentChar;
                    readNextChar();
                }
                return Token(TokenType::ERROR_TOK, numBuf, currentLine);
            }
        }
        TokenType ty = (dotCount == 1) ? TokenType::REALCON : TokenType::INTCON;
        return Token(ty, numBuf, currentLine);
    }

    if ((currentChar >= 'a' && currentChar <= 'z') || (currentChar >= 'A' && currentChar <= 'Z')) {
        std::string identBuf;
        while (!eofReached && isIdentChar(currentChar)) {
            identBuf += currentChar;
            readNextChar();
        }
        return Token(classifyIdent(identBuf), identBuf, currentLine);
    }

    if (currentChar == '\'') {
        std::string strBuf = "'";
        while (readNextChar(), !eofReached && currentChar != '\'') {
            strBuf += currentChar;
        }
        strBuf += '\'';
        readNextChar();

        TokenType ty = (strBuf.size() == 3) ? TokenType::CHARCON : TokenType::STRING_TOK;
        return Token(ty, strBuf, currentLine);
    }

    if (isOperatorChar(currentChar)) {
        std::string opBuf;
        opBuf += currentChar;
        char first = currentChar;
        readNextChar();

        if (first == ':' && currentChar == '=') {
            opBuf += currentChar; readNextChar();
        } else if (first == '=' && currentChar == '=') {

            opBuf += currentChar; readNextChar();
        } else if (first == '<' && (currentChar == '=' || currentChar == '>')) {
            opBuf += currentChar; readNextChar();
        } else if (first == '>' && currentChar == '=') {
            opBuf += currentChar; readNextChar();
        }

        return Token(classifyOperator(opBuf), opBuf, currentLine);
    }

    std::string unknown(1, currentChar);
    readNextChar();
    return Token(TokenType::ERROR_TOK, unknown, currentLine);
}

bool Lexer::isEOF() const {
    return eofReached;
}
