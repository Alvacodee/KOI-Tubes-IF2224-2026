#include "operator.hpp"

bool isOperatorChar(char c){
    if (c == '+' || c == '-' || c == '*' || c == '/' ||
        c == '=' || c == '>' || c == '<' || c == ':' ||
        c == ';' || c == ',' || c == '.' || c == '(' ||
        c == ')' || c == '[' || c == ']' || c == '!'){
        return true;
    } else {
        return false;
    }
}

TokenType classifyOperator(const std::string& op){
    if (op == "+") return TokenType::PLUS;
    if (op == "-") return TokenType::MINUS;
    if (op == "*") return TokenType::TIMES;
    if (op == "/") return TokenType::RDIV;

    if (op == "==" || op == "=") return TokenType::EQL;
    if (op == "<>" || op == "!=") return TokenType::NEQ;
    if (op == ">") return TokenType::GTR;
    if (op == ">=") return TokenType::GEQ;
    if (op == "<") return TokenType::LSS;
    if (op == "<=") return TokenType::LEQ;
    if (op == ":=") return TokenType::BECOMES;

    if (op == "(") return TokenType::LPARENT;
    if (op == ")") return TokenType::RPARENT;
    if (op == "[") return TokenType::LBRACK;
    if (op == "]") return TokenType::RBRACK;
    if (op == ",") return TokenType::COMMA;
    if (op == ";") return TokenType::SEMICOLON;
    if (op == ".") return TokenType::PERIOD;
    if (op == ":") return TokenType::COLON;

    return TokenType::ERROR_TOK;
}