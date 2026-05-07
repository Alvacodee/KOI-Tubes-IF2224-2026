#include <iostream>
#include <fstream>
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

    string outFilename = filename;
    size_t dotPos = outFilename.find_last_of('.');
    if (dotPos != string::npos) {
        outFilename = outFilename.substr(0, dotPos) + "_solusi" + outFilename.substr(dotPos);
    } else {
        outFilename += "_solusi.txt";
    }

    ofstream outputFile(outFilename);
    if (!outputFile.is_open()) {
        cerr << "Error: Tidak dapat membuat file solusi " << outFilename << endl;
        return 1;
    }

    cout << "\nBerhasil membuka file: " << filename << endl;

    cout << "--- Isi File Input ---" << endl;
    outputFile << "--- Isi File Input ---" << endl;
    
    string line;
    while (getline(inputFile, line)) {
        cout << line << endl;
        outputFile << line << endl;
    }

    inputFile.clear();
    inputFile.seekg(0);

    cout << "\n--- Memulai proses baca karakter ---" << endl;
    outputFile << "\n--- Hasil Lexical Analysis ---" << endl;

    Lexer lexer(inputFile);

    while (true) {
        Token t = lexer.getNextToken();

        if (t.type == TokenType::ERROR_TOK && t.value == "EOF") {
            break;
        }

        if (tokenHasValue(t.type)) {
            cout << tokenTypeName(t.type) << "(" << t.value << ")" << endl;
            outputFile << tokenTypeName(t.type) << "(" << t.value << ")" << endl;
        } else {
            cout << tokenTypeName(t.type) << endl;
            outputFile << tokenTypeName(t.type) << endl;
        }
    }

    cout << "\n--- Selesai ---" << endl;
    cout << "Output berhasil disimpan di: " << outFilename << endl;

    inputFile.close();
    outputFile.close();
    
    return 0;
}