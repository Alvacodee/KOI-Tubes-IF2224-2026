#pragma once
#include <istream>
#include "token.hpp"

Token handleNumber(std::istream& inputFile, char firstChar, int currentLine);

Token handleStringOrChar(std::istream& inputFile, int currentLine);