#include <iostream>
#include <fstream>
#include <string>
#include <cstring>

#include "parse_tree_reader.hpp"
#include "parse_tree_node.hpp"
#include "ast_node.hpp"
#include "semantic.hpp"
#include "symbol_table.hpp"

ASTNode* buildMockAST() {
    ASTNode* program = new ASTNode(AST_PROGRAM, "Hello");
    program->tabIndex = 34;
    program->level = 0;

    ASTNode* declPart = new ASTNode(AST_DECL_PART);

    ASTNode* varA = new ASTNode(AST_VAR_DECL, "a");
    varA->tabIndex = 35;
    varA->typeCode = T_INTEGER;
    varA->level = 1;

    ASTNode* varB = new ASTNode(AST_VAR_DECL, "b");
    varB->tabIndex = 36;
    varB->typeCode = T_INTEGER;
    varB->level = 1;

    declPart->add(varA);
    declPart->add(varB);

    ASTNode* block = new ASTNode(AST_BLOCK);
    block->tabIndex = 1;
    block->level = 1;

    block->add(declPart);

    ASTNode* compound = new ASTNode(AST_COMPOUND);
    ASTNode* stmtList = new ASTNode(AST_STMT_LIST);

    ASTNode* assign1 = new ASTNode(AST_ASSIGN);
    assign1->typeCode = T_NONE;

    ASTNode* targetA = new ASTNode(AST_VAR, "a");
    targetA->tabIndex = 35;
    targetA->typeCode = T_INTEGER;

    ASTNode* val5 = new ASTNode(AST_INT_LIT, "5");
    val5->typeCode = T_INTEGER;

    assign1->add(targetA);
    assign1->add(val5);

    ASTNode* assign2 = new ASTNode(AST_ASSIGN);
    assign2->typeCode = T_NONE;

    ASTNode* targetB = new ASTNode(AST_VAR, "b");
    targetB->tabIndex = 36;
    targetB->typeCode = T_INTEGER;

    ASTNode* binOp = new ASTNode(AST_BINOP, "", "plus");
    binOp->typeCode = T_INTEGER;

    ASTNode* refA = new ASTNode(AST_VAR, "a");
    refA->tabIndex = 35;
    refA->typeCode = T_INTEGER;

    ASTNode* val10 = new ASTNode(AST_INT_LIT, "10");
    val10->typeCode = T_INTEGER;

    binOp->add(refA);
    binOp->add(val10);

    assign2->add(targetB);
    assign2->add(binOp);

    ASTNode* procCall = new ASTNode(AST_PROC_CALL, "writeln");

    ASTNode* strArg = new ASTNode(AST_STR_LIT, "'Result = '");
    strArg->typeCode = T_STRING;

    ASTNode* varBArg = new ASTNode(AST_VAR, "b");
    varBArg->tabIndex = 36;
    varBArg->typeCode = T_INTEGER;

    procCall->add(strArg);
    procCall->add(varBArg);

    stmtList->add(assign1);
    stmtList->add(assign2);
    stmtList->add(procCall);

    compound->add(stmtList);
    block->add(compound);

    program->add(block);

    return program;
}

void printDecoratedAST(std::ostream& out, const ASTNode* ast) {
    if (!ast) {
        out << "(AST kosong)" << std::endl;
        return;
    }

    out << "ProgramNode(name: '" << ast->value << "')" << std::endl;

    std::string prefix = "";
    for (int i = 0; i < (int)ast->children.size(); i++) {
        if (ast->children[i]) {
            ast->children[i]->print(out, prefix,
                                     i == (int)ast->children.size() - 1);
        }
    }
}

void runSemanticFromFile(const std::string& filepath) {
    std::cout << "=== Arion Compiler - Milestone 3: Semantic Analysis ===" << std::endl;
    std::cout << "Membaca parse tree dari: " << filepath << std::endl;
    std::cout << std::endl;

    std::cout << "--- Tahap 1: Membaca Parse Tree ---" << std::endl;
    ParseTreeNode* parseTree = ParseTreeReader::readFromFile(filepath);

    if (!parseTree) {
        std::cerr << "Error: Gagal membaca parse tree dari file." << std::endl;
        return;
    }

    std::cout << "Parse tree berhasil dimuat. Struktur:" << std::endl;
    parseTree->print(std::cout);
    std::cout << std::endl;

    std::cout << "--- Tahap 2: Semantic Analysis ---" << std::endl;
    SemanticAnalyzer sem;
    ASTNode* ast = nullptr;

    try {
        ast = sem.analyze(parseTree);
        std::cout << "Semantic analysis selesai." << std::endl;
    } catch (const SemanticError& e) {
        std::cerr << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error semantic: " << e.what() << std::endl;
    }

    if (ast) {
        std::cout << std::endl;
        std::cout << "--- Tahap 3: Decorated AST ---" << std::endl;
        printDecoratedAST(std::cout, ast);
    }

    std::cout << std::endl;
    sem.printResults(std::cout);

    if (!sem.errors.empty()) {
        std::cout << "\n--- Semantic Errors (" << sem.errors.size() << ") ---"
                  << std::endl;
        for (const auto& e : sem.errors) {
            std::cout << "  " << e << std::endl;
        }
    }

    std::string base = filepath;
    size_t dotPos = base.find_last_of('.');
    if (dotPos != std::string::npos) base = base.substr(0, dotPos);
    std::string outPath = base + "_semantic_output.txt";

    std::ofstream outFile(outPath);
    if (outFile.is_open()) {
        outFile << "--- Decorated AST ---\n";
        if (ast) printDecoratedAST(outFile, ast);
        outFile << "\n";
        sem.printResults(outFile);
        outFile.close();
        std::cout << "\nHasil disimpan ke: " << outPath << std::endl;
    }

    delete parseTree;
    delete ast;
}

void runMockDemo() {
    std::cout << "=== Arion Compiler - Demo Mode ===" << std::endl;
    std::cout << "Membangun AST buatan (mockup) untuk demonstrasi printer..."
              << std::endl;
    std::cout << std::endl;

    ASTNode* mockAST = buildMockAST();

    std::cout << "--- Decorated AST (Mockup) ---" << std::endl;
    printDecoratedAST(std::cout, mockAST);
    std::cout << std::endl;

    std::cout << "--- Raw AST Print (via ASTNode::printToFile format) ---" << std::endl;
    mockAST->printToFile("demo_ast_output.txt");
    std::cout << "Output disimpan ke: demo_ast_output.txt" << std::endl;

    delete mockAST;
}

int main(int argc, char* argv[]) {
    if (argc >= 2) {
        std::string arg = argv[1];

        if (arg == "--demo" || arg == "-d") {
            runMockDemo();
            return 0;
        }

        runSemanticFromFile(arg);
        return 0;
    }

    std::cout << "=== Arion Compiler - Milestone 3: Semantic Analysis ===" << std::endl;
    std::cout << std::endl;
    std::cout << "Pilih mode:" << std::endl;
    std::cout << "  1. Baca dari file parse tree" << std::endl;
    std::cout << "  2. Jalankan demo (AST mockup)" << std::endl;
    std::cout << "Pilihan (1/2): ";

    std::string choice;
    std::getline(std::cin, choice);

    if (choice == "2") {
        runMockDemo();
    } else {
        std::cout << "Masukkan path file parse tree: ";
        std::string filepath;
        std::getline(std::cin, filepath);
        runSemanticFromFile(filepath);
    }

    return 0;
}
