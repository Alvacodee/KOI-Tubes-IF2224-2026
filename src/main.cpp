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

void printDecoratedAST(ostream& out, const ASTNode* ast) {
    if (!ast) return;
    out << "ProgramNode('" << ast->value << "')" << endl;
    string pfx = "";
    for (int i = 0; i < (int)ast->children.size(); i++)
        if (ast->children[i])
            ast->children[i]->print(out, pfx, i == (int)ast->children.size()-1);
}

void runSemanticAndPrint(ParseTreeNode* parseTree, const string& outSemantic) {
    SemanticAnalyzer sem;
    ASTNode* ast = nullptr;

    try {
        ast = sem.analyze(parseTree);
    } catch (const SemanticError& e) {
        cerr << e.what() << endl;
    } catch (const exception& e) {
        cerr << "Error semantic: " << e.what() << endl;
    }

    if (!sem.errors.empty()) {
        cout << "\n[!] Proses Semantic Analysis GAGAL. Ditemukan " 
             << sem.errors.size() << " kesalahan semantik:" << endl;
        for (auto& e : sem.errors) cout << "  - " << e << endl;
    } else if (ast) {
        cout << "\n--- Decorated AST ---" << endl;
        printDecoratedAST(cout, ast);
    }

    cout << endl;
    sem.printResults(cout);

    ofstream semFile(outSemantic);
    if (semFile.is_open()) {
        if (!sem.errors.empty()) {
            semFile << "--- Semantic Errors (" << sem.errors.size() << ") ---\n";
            for (auto& e : sem.errors) {
                semFile << "  - " << e << "\n";
            }
            semFile << "\n";
        } else if (ast) {
            semFile << "--- Decorated AST ---\n";
            printDecoratedAST(semFile, ast);
            semFile << "\n";
        }
        
        sem.printResults(semFile);
        semFile.close();
        cout << "Output: " << outSemantic << endl;
    }

    delete ast;
}

void modeSourceCode(const string& filename) {
    ifstream inputFile(filename);
    if (!inputFile.is_open()) {
        cerr << "Error: Tidak dapat membuka file " << filename << endl;
        return;
    }

    string base = filename;
    size_t dotPos = base.find_last_of('.');
    if (dotPos != string::npos) base = base.substr(0, dotPos);

    string outLexer    = base + "_solusi.txt";
    string outParser   = base + "_parse_tree.txt";
    string outSemantic = base + "_semantic.txt";

    ofstream outputFile(outLexer);
    if (!outputFile.is_open()) {
        cerr << "Error: Tidak dapat membuat file output " << outLexer << endl;
        return;
    }

    cout << "--- Lexical Analysis ---" << endl;
    outputFile << "--- Hasil Lexical Analysis ---" << endl;

    Lexer lexer(inputFile);
    vector<Token> tokenList;

    while (true) {
        Token t = lexer.getNextToken();
        if (t.type == TokenType::ERROR_TOK && t.value == "EOF") break;
        if (t.type == TokenType::COMMENT) continue;

        string display;
        if (tokenHasValue(t.type))
            display = tokenTypeName(t.type) + "(" + t.value + ")";
        else
            display = tokenTypeName(t.type);

        cout       << display << endl;
        outputFile << display << endl;
        tokenList.push_back(t);
    }
    outputFile.close();

    cout << "\n--- Parse Tree ---" << endl;
    Parser parser(tokenList);
    ParseTreeNode* parseTree = nullptr;

    try {
        parseTree = parser.parse();
        parseTree->print(cout);
        parseTree->printToFile(outParser);
    } catch (const SyntaxError& e) {
        cerr << e.what() << endl;
        return;
    }

    runSemanticAndPrint(parseTree, outSemantic);
    delete parseTree;
}

void modeParseTreeFile(const string& filename) {
    ParseTreeNode* parseTree = ParseTreeReader::readFromFile(filename);
    if (!parseTree) {
        cerr << "Error: Gagal membaca parse tree dari file." << endl;
        return;
    }

    cout << "--- Parse Tree (dari file) ---" << endl;
    parseTree->print(cout);

    string base = filename;
    size_t dotPos = base.find_last_of('.');
    if (dotPos != string::npos) base = base.substr(0, dotPos);
    string outSemantic = base + "_semantic.txt";

    runSemanticAndPrint(parseTree, outSemantic);
    delete parseTree;
}

int main(int argc, char* argv[]) {
    if (argc >= 3) {
        string mode = argv[1];
        string file = argv[2];
        if (mode == "-s") modeSourceCode(file);
        else if (mode == "-p") modeParseTreeFile(file);
        else cerr << "Mode: -s (source) atau -p (parse tree)" << endl;
        return 0;
    }

    cout << "Pilih mode:" << endl;
    cout << "  1. Source code  (Lexer -> Parser -> Semantic)" << endl;
    cout << "  2. Parse tree   (ParseTreeReader -> Semantic)" << endl;
    cout << "Pilihan (1/2): ";

    string choice;
    cin >> choice;

    cout << "Masukkan alamat file: ";
    string filename;
    cin >> filename;

    if (choice == "1") modeSourceCode(filename);
    else if (choice == "2") modeParseTreeFile(filename);
    else cerr << "Pilihan tidak valid." << endl;

    return 0;
}
