#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "token.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "semantic.hpp"
#include "parse_tree_reader.hpp"

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

    // Buat nama file output (hapus .txt, tambah _solusi.txt)
    string base = filename;
    size_t dotPos = base.find_last_of('.');
    if (dotPos != string::npos) base = base.substr(0, dotPos);

    string outLexer   = base + "_solusi.txt";
    string outParser  = base + "_parse_tree.txt";
    string outSemantic= base + "_semantic.txt";

    ofstream outputFile(outLexer);
    if (!outputFile.is_open()) {
        cerr << "Error: Tidak dapat membuat file output " << outLexer << endl;
        return 1;
    }

    // ---- Lexical Analysis ----
    cout << "\n--- Hasil Lexical Analysis ---" << endl;
    outputFile << "--- Hasil Lexical Analysis ---" << endl;

    Lexer lexer(inputFile);
    vector<Token> tokenList;

    while (true) {
        Token t = lexer.getNextToken();
        if (t.type == TokenType::ERROR_TOK && t.value == "EOF") break;

        if (t.type == TokenType::COMMENT) continue; // M3: comment tidak dijadikan node

        string display;
        if (tokenHasValue(t.type))
            display = tokenTypeName(t.type) + "(" + t.value + ")";
        else
            display = tokenTypeName(t.type);

        cout     << display << endl;
        outputFile << display << endl;
        tokenList.push_back(t);
    }
    outputFile.close();

    cout << "\n--- Selesai Lexical Analysis ---" << endl;
    cout << "Output lexer: " << outLexer << endl;

    // ---- Syntax Analysis ----
    cout << "\n--- Memulai Syntax Analysis ---" << endl;
    Parser parser(tokenList);
    ParseTreeNode* parseTree = nullptr;

    try {
        parseTree = parser.parse();
        cout << "\n--- Parse Tree ---" << endl;
        parseTree->print(cout);
        parseTree->printToFile(outParser);
        cout << "Parse tree disimpan: " << outParser << endl;
    } catch (const SyntaxError& e) {
        cerr << "\n" << e.what() << endl;
        return 1;
    }

    // ---- Semantic Analysis ----
    cout << "\n--- Memulai Semantic Analysis ---" << endl;

    SemanticAnalyzer sem;
    ASTNode* ast = nullptr;

    try {
        ast = sem.analyze(parseTree);
    } catch (const SemanticError& e) {
        cerr << e.what() << endl;
        // tidak return, tetap cetak hasil parsial
    } catch (const exception& e) {
        cerr << "Error semantic: " << e.what() << endl;
    }

    // Cetak Decorated AST
    if (ast) {
        cout << "\n--- Decorated AST ---" << endl;
        // cetak root secara manual
        cout << "ProgramNode('" << ast->value << "')" << endl;
        string pfx = "";
        for (int i = 0; i < (int)ast->children.size(); i++)
            if (ast->children[i])
                ast->children[i]->print(cout, pfx, i == (int)ast->children.size()-1);
    }

    // Cetak symbol tables ke terminal dan file
    cout << endl;
    sem.printResults(cout);

    ofstream semFile(outSemantic);
    if (semFile.is_open()) {
        if (ast) {
            semFile << "--- Decorated AST ---\n";
            semFile << "ProgramNode('" << ast->value << "')\n";
            string pfx = "";
            for (int i = 0; i < (int)ast->children.size(); i++)
                if (ast->children[i])
                    ast->children[i]->print(semFile, pfx, i == (int)ast->children.size()-1);
            semFile << "\n";
        }
        sem.printResults(semFile);
        semFile.close();
        cout << "\nHasil semantic disimpan: " << outSemantic << endl;
    }

    // Cetak error jika ada
    if (!sem.errors.empty()) {
        cout << "\n--- Semantic Errors (" << sem.errors.size() << ") ---" << endl;
        for (auto& e : sem.errors) cout << e << endl;
    }

    // Bersihkan memori
    delete parseTree;
    delete ast;

    cout << "\n--- Selesai ---" << endl;
    return 0;
}
