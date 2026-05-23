#include "semantic.hpp"
#include <sstream>
#include <algorithm>
// #include <> //tambahin include nya disini wan

SemanticAnalyzer::SemanticAnalyzer() {}

ASTNode* SemanticAnalyzer::analyze(ParseTreeNode* pt) {
    if (!pt) return nullptr;
    return visitProgram(pt);
}

std::string SemanticAnalyzer::tokVal(ParseTreeNode* n) const {
    auto& s = n->name;
    size_t lp = s.find('(');
    size_t rp = s.rfind(')');
    if (lp != std::string::npos && rp != std::string::npos && rp > lp)
        return s.substr(lp + 1, rp - lp - 1);
    return "";
}

std::string SemanticAnalyzer::tokTy(ParseTreeNode* n) const {
    auto& s = n->name;
    size_t lp = s.find('(');
    if (lp != std::string::npos) return s.substr(0, lp);
    return s;
}

bool SemanticAnalyzer::isNT(ParseTreeNode* n, const std::string& name) const {
    return n && n->name == name;
}

bool SemanticAnalyzer::isTok(ParseTreeNode* n, const std::string& type) const {
    return n && tokTy(n) == type;
}

ParseTreeNode* SemanticAnalyzer::findChild(ParseTreeNode* n, const std::string& name) const {
    for (auto* c : n->children)
        if (c->name == name || tokTy(c) == name) return c;
    return nullptr;
}

void SemanticAnalyzer::addError(const std::string& msg) {
    errors.push_back("Semantic Error: " + msg);
    std::cerr << "Semantic Error: " << msg << "\n";
}


ASTNode* SemanticAnalyzer::visitProgram(ParseTreeNode* n) {
    ASTNode* ast = makeAST(AST_PROGRAM);

    ParseTreeNode* hdr = n->children[0];
    std::string progName;
    for (auto* c : hdr->children)
        if (isTok(c, "ident")) { progName = tokVal(c); break; }

    ast->value = progName;
    ast->level = 0;

    int progIdx = symtab.enter(progName, OBJ_PROGRAM, T_NONE, 0, 1, 0);
    ast->tabIndex = progIdx;

    ParseTreeNode* blk = n->children[1];
    ASTNode* blockAst = visitBlock(blk);
    ast->add(blockAst);

    return ast;
}

ASTNode* SemanticAnalyzer::visitBlock(ParseTreeNode* n) {
    ASTNode* ast = makeAST(AST_BLOCK);

    int bIdx = symtab.openBlock();
    ast->tabIndex = bIdx;
    ast->level = symtab.curLevel;

    for (auto* c : n->children) {
        if (isNT(c, "<declaration-part>")) {
            ASTNode* declAst = visitDeclarationPart(c);
            ast->add(declAst);
        } else if (isNT(c, "<compound-statement>")) {
            ASTNode* stmtAst = visitCompoundStatement(c);
            ast->add(stmtAst);
        }
    }

    symtab.closeBlock();
    return ast;
}

ASTNode* SemanticAnalyzer::visitDeclarationPart(ParseTreeNode* n) {
    ASTNode* ast = makeAST(AST_DECL_PART);

    for (auto* c : n->children) {
        if (isNT(c, "<const-declaration>")) {
            ast->add(visitConstDeclaration(c));
        } else if (isNT(c, "<type-declaration>")) {
            ast->add(visitTypeDeclaration(c));
        } else if (isNT(c, "<var-declaration>")) {
            ast->add(visitVarDeclaration(c));
        } else if (isNT(c, "<subprogram-declaration>")) {
            ast->add(visitSubprogramDeclaration(c));
        }
    }
    return ast;
}

// Output hasil
void SemanticAnalyzer::printResults(std::ostream& out) const {
    symtab.printTab(out);
    symtab.printBtab(out);
    symtab.printAtab(out);
}
