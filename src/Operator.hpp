#ifndef OPERATOR_HPP
#define OPERATOR_HPP

#include <string>
#include "token.hpp"

bool isOperatorChar(char c);

TokenType classifyOperator(const std::string& op);

#endif