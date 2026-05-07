#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "token.hpp"
#include "lexer.hpp"
#include "parser.hpp"

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

    string outLexer = filename;
    size_t dotPos = outLexer.find_last_of('.');
    if (dotPos != string::npos) {
        outLexer = outLexer.substr(0, dotPos) + "_solusi" + outLexer.substr(dotPos);
    } else {
        outLexer += "_solusi.txt";
    }

    ofstream outputFile(outLexer);
    if (!outputFile.is_open()) {
        cerr << "Error: Tidak dapat membuat file solusi " << outLexer << endl;
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
    vector<Token> tokenList;

    while (true) {
        Token t = lexer.getNextToken();
        if (t.type == TokenType::ERROR_TOK && t.value == "EOF") break;

        if (t.type == TokenType::COMMENT) {
            cout << tokenTypeName(t.type) << " (" << t.value << ")" << endl;
            outputFile << tokenTypeName(t.type) << " (" << t.value << ")" << endl;
            continue;
        }

        if (tokenHasValue(t.type)) {
            cout << tokenTypeName(t.type) << "(" << t.value << ")" << endl;
            outputFile << tokenTypeName(t.type) << "(" << t.value << ")" << endl;
        } else {
            cout << tokenTypeName(t.type) << endl;
            outputFile << tokenTypeName(t.type) << endl;
        }

        tokenList.push_back(t);
    }

    outputFile.close();

    cout << "\n--- Selesai Lexical Analysis ---" << endl;
    cout << "Output lexer disimpan di: " << outLexer << endl;

    cout << "\n--- Memulai Syntax Analysis ---" << endl;

    Parser parser(tokenList);

    try {
        ParseTreeNode* tree = parser.parse();

        cout << "\n--- Parse Tree ---" << endl;
        tree->print(cout);

        string outParser = filename;
        dotPos = outParser.find_last_of('.');
        if (dotPos != string::npos) {
            outParser = outParser.substr(0, dotPos) + "_parse_tree" + outParser.substr(dotPos);
        } else {
            outParser += "_parse_tree.txt";
        }

        tree->printToFile(outParser);
        cout << "\nParse tree disimpan di: " << outParser << endl;

        delete tree;

    } catch (const SyntaxError& e) {
        cerr << "\n" << e.what() << endl;
        return 1;
    }

    cout << "\n--- Selesai ---" << endl;

    return 0;
}
