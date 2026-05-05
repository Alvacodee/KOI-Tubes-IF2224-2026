#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "token.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "node.hpp"

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

    // Tentukan nama file output parse tree
    string outFilename = filename;
    size_t dotPos = outFilename.find_last_of('.');
    if (dotPos != string::npos)
        outFilename = outFilename.substr(0, dotPos) + "_parsetree" + outFilename.substr(dotPos);
    else
        outFilename += "_parsetree.txt";

    // Fase 1: Lexical Analysis
    Lexer lexer(inputFile);
    vector<Token> tokens;

    while (true) {
        Token t = lexer.getNextToken();
        if (t.type == TokenType::ERROR_TOK && t.value == "EOF") break;
        // Skip komentar — tidak perlu masuk ke parser
        if (t.type == TokenType::COMMENT) continue;
        tokens.push_back(t);
    }
    inputFile.close();

    cout << "\nLexical analysis selesai: " << tokens.size() << " token.\n";

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
    }

    outputFile.close();
    return 0;
}
