#include <iostream>
#include <fstream>
#include <string>
#include <iostream>
#include <string>
#include "token.hpp"
#include "literal.hpp"
#include "identifier.hpp"
#include "operator.hpp"
#include "lexer.hpp"

using namespace std;

int main() {
    cout << "Masukkan alamat file.txt: ";
    string filename;
    cin >> filename;

    ifstream inputFile(filename);
    if (!inputFile.is_open()) {
        cerr << "Error: Tidak dapat membuka file " << filename << endl;
        return 1;
    }

    cout << "Berhasil membuka file: " << filename << endl;
    cout << "--- Memulai proses baca karakter ---" << endl << endl;

    Lexer lexer(inputFile);

    while (true) {
        Token t = lexer.getNextToken();

        if (t.type == TokenType::ERROR_TOK && t.value == "EOF") {
            break;
        }

        if (tokenHasValue(t.type)) {
            cout << tokenTypeName(t.type) << " (" << t.value << ")" << endl;
        } else {
            cout << tokenTypeName(t.type) << endl;
        }
    }

    cout << "\n--- Selesai ---" << endl;

    inputFile.close();
    return 0;
}