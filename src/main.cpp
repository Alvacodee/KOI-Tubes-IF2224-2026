#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "token.hpp"
#include "lexer.hpp"
#include "parser.hpp"
<<<<<<< HEAD
#include "node.hpp"
=======
>>>>>>> 542f638e28e03e5d007ff24ad92b7087216241e8

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

<<<<<<< HEAD
    // Tentukan nama file output parse tree
    string outFilename = filename;
    size_t dotPos = outFilename.find_last_of('.');
    if (dotPos != string::npos)
        outFilename = outFilename.substr(0, dotPos) + "_parsetree" + outFilename.substr(dotPos);
    else
        outFilename += "_parsetree.txt";
=======
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
>>>>>>> 542f638e28e03e5d007ff24ad92b7087216241e8

    // Fase 1: Lexical Analysis
    Lexer lexer(inputFile);
<<<<<<< HEAD
    vector<Token> tokens;
=======
    vector<Token> tokenList;
>>>>>>> 542f638e28e03e5d007ff24ad92b7087216241e8

    while (true) {
        Token t = lexer.getNextToken();
        if (t.type == TokenType::ERROR_TOK && t.value == "EOF") break;
        // Skip komentar — tidak perlu masuk ke parser
        if (t.type == TokenType::COMMENT) continue;
        tokens.push_back(t);
    }
    inputFile.close();

    cout << "\nLexical analysis selesai: " << tokens.size() << " token.\n";

<<<<<<< HEAD
    // Fase 2: Syntax Analysis (Parse)
    ofstream outputFile(outFilename);
    if (!outputFile.is_open()) {
        cerr << "Error: Tidak dapat membuat file output " << outFilename << endl;
        return 1;
    }

    try {
        Parser parser(tokens);
        auto tree = parser.parse();

        // Print ke terminal
        cout << "\n--- Parse Tree ---\n";
        cout << tree->label << "\n";
        for (size_t i = 0; i < tree->children.size(); i++) {
            bool last = (i == tree->children.size() - 1);
            printTree(tree->children[i], cout, "", last);
        }

        // Print ke file
        outputFile << tree->label << "\n";
        for (size_t i = 0; i < tree->children.size(); i++) {
            bool last = (i == tree->children.size() - 1);
            printTree(tree->children[i], outputFile, "", last);
        }

        cout << "\nOutput disimpan di: " << outFilename << "\n";

    } catch (const SyntaxError& e) {
        cerr << e.what() << endl;
        outputFile << "SYNTAX ERROR: " << e.what() << "\n";
        return 1;
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
=======
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
>>>>>>> 542f638e28e03e5d007ff24ad92b7087216241e8
    }

    outputFile.close();
<<<<<<< HEAD
=======

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

>>>>>>> 542f638e28e03e5d007ff24ad92b7087216241e8
    return 0;
}
