#pragma once
#include <string>
#include "token.hpp"

bool isIdentChar(char c);

TokenType classifyIdent(const std::string& w);