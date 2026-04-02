#pragma once
#include <string>
#include "token.hpp"

// Mengecek apakah karakter c boleh jadi bagian dari identifier (huruf atau digit)
bool isIdentChar(char c);

// Mengklasifikasikan sebuah kata sebagai keyword atau identifier biasa.
// Parameter w harus sudah dalam bentuk lowercase sebelum dipanggil.
TokenType classifyIdent(const std::string& w);