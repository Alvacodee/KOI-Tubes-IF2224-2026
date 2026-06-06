#include "semantic.hpp"
#include <sstream>
#include <algorithm>

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

ASTNode* SemanticAnalyzer::visitBlock(ParseTreeNode* n, bool doOpen) {
    ASTNode* ast = makeAST(AST_BLOCK);

    int bIdx = doOpen ? symtab.openBlock() : symtab.curBlock;
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

    if (doOpen) symtab.closeBlock();
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

ASTNode* SemanticAnalyzer::visitCompoundStatement(ParseTreeNode* n) {
    ASTNode* ast = makeAST(AST_COMPOUND);
    for (auto* c : n->children) {
        if (isNT(c, "<statement-list>"))
            ast->add(visitStatementList(c));
    }
    return ast;
}

ASTNode* SemanticAnalyzer::visitStatementList(ParseTreeNode* n) {
    ASTNode* ast = makeAST(AST_STMT_LIST);
    for (auto* c : n->children) {
        if (isTok(c, "semicolon")) continue;
        ASTNode* s = visitStatement(c);
        if (s) ast->add(s);
    }
    return ast;
}

ASTNode* SemanticAnalyzer::visitStatement(ParseTreeNode* n) {
    if (!n) return makeAST(AST_EMPTY);

    if (isNT(n, "<assignment-statement>")) return visitAssignStatement(n);
    if (isNT(n, "<if-statement>"))         return visitIfStatement(n);
    if (isNT(n, "<while-statement>"))      return visitWhileStatement(n);
    if (isNT(n, "<for-statement>"))        return visitForStatement(n);
    if (isNT(n, "<repeat-statement>"))     return visitRepeatStatement(n);
    if (isNT(n, "<case-statement>"))       return visitCaseStatement(n);
    if (isNT(n, "<procedure/function-call>")) return visitProcCall(n);
    if (isNT(n, "<compound-statement>"))   return visitCompoundStatement(n);
    if (isNT(n, "<empty-statement>"))      return makeAST(AST_EMPTY);

    if (isNT(n, "<statement>") && !n->children.empty()) {
        return visitStatement(n->children[0]);
    }

    return makeAST(AST_EMPTY);
}

ASTNode* SemanticAnalyzer::visitAssignStatement(ParseTreeNode* n) {
    ASTNode* ast = makeAST(AST_ASSIGN);
    ASTNode* targetAST = nullptr;
    ASTNode* valueAST = nullptr;

    for (auto* c : n->children) {
        if (isNT(c, "<variable>") || isNT(c, "<component-variable>")) {
            targetAST = visitVariable(c);
            if (targetAST) ast->add(targetAST);
        } else if (isNT(c, "<expression>")) {
            valueAST = visitExpression(c);
            if (valueAST) ast->add(valueAST);
        }
    }

    if (targetAST && valueAST) {
        bool compatible = false;
        if (targetAST->typeCode != T_NONE && valueAST->typeCode != T_NONE) {
            if (targetAST->typeCode == valueAST->typeCode) {
                compatible = true;
            } else if (targetAST->typeCode == T_REAL && valueAST->typeCode == T_INTEGER) {
                compatible = true;
            }
        }
        if (!compatible) {
            addError("Type mismatch in assignment: cannot assign '" +
                typeName(valueAST->typeCode) + "' to '" +
                typeName(targetAST->typeCode) + "'");
        }
        ast->typeCode = targetAST->typeCode;
    }
    return ast;
}

ASTNode* SemanticAnalyzer::visitIfStatement(ParseTreeNode* n) {
    ASTNode* ast = makeAST(AST_IF);
    for (auto* c : n->children) {
        if (isTok(c, "ifsy") || isTok(c, "thensy") || isTok(c, "elsesy")) continue;

        if (isNT(c, "<expression>")) {
            ASTNode* condAST = visitExpression(c);
            if (condAST && condAST->typeCode != T_NONE && condAST->typeCode != T_BOOLEAN) {
                addError("Type mismatch in IF condition: expected Boolean");
            }
            if (condAST) ast->add(condAST);
        } else {
            ASTNode* stmtAST = visitStatement(c);
            if (stmtAST) ast->add(stmtAST);
        }
    }
    return ast;
}

ASTNode* SemanticAnalyzer::visitWhileStatement(ParseTreeNode* n) {
    ASTNode* ast = makeAST(AST_WHILE);
    for (auto* c : n->children) {
        if (isTok(c, "dosy") || isTok(c, "semicolon")) continue;

        if (isNT(c, "<expression>")) {
            ASTNode* condAST = visitExpression(c);
            if (condAST && condAST->typeCode != T_NONE && condAST->typeCode != T_BOOLEAN) {
                addError("Type mismatch in WHILE condition: expected Boolean");
            }
            if (condAST) ast->add(condAST);
        } else if (isNT(c, "<compound-statement>")) {
            ASTNode* bodyAST = visitCompoundStatement(c);
            if (bodyAST) ast->add(bodyAST);
        }
    }
    return ast;
}

ASTNode* SemanticAnalyzer::visitForStatement(ParseTreeNode* n) {
    ASTNode* ast = makeAST(AST_FOR);

    for (auto* c : n->children) {
        if (isTok(c, "tosy")) ast->op = "to";
        else if (isTok(c, "downtosy")) ast->op = "downto";

        if (isTok(c, "becomes") || isTok(c, "tosy") ||
            isTok(c, "downtosy") || isTok(c, "dosy") || isTok(c, "semicolon")) continue;

        if (isTok(c, "ident")) {
            std::string varName = tokVal(c);
            ASTNode* v = makeAST(AST_VAR, varName);
            int idx = symtab.lookup(varName);

            if (idx >= 0) {
                v->tabIndex = idx;
                v->level = symtab.tab[idx].lev;
                v->typeCode = symtab.tab[idx].type;
                if (symtab.tab[idx].type != T_INTEGER) {
                    addError("FOR counter variable must be Integer: " + varName);
                }
            } else {
                addError("Undeclared FOR counter variable: " + varName);
            }
            ast->add(v);
        } else if (isNT(c, "<expression>")) {
            ASTNode* exprAST = visitExpression(c);
            if (exprAST && exprAST->typeCode != T_NONE && exprAST->typeCode != T_INTEGER) {
                addError("FOR range expression must evaluate to Integer");
            }
            if (exprAST) ast->add(exprAST);
        } else if (isNT(c, "<compound-statement>")) {
            ASTNode* bodyAST = visitCompoundStatement(c);
            if (bodyAST) ast->add(bodyAST);
        }
    }
    return ast;
}

ASTNode* SemanticAnalyzer::visitRepeatStatement(ParseTreeNode* n) {
    ASTNode* ast = makeAST(AST_REPEAT);
    for (auto* c : n->children) {
        if (isTok(c, "untilsy")) continue;

        if (isNT(c, "<statement-list>")) {
            ASTNode* stmtListAST = visitStatementList(c);
            if (stmtListAST) ast->add(stmtListAST);
        } else if (isNT(c, "<expression>")) {
            ASTNode* condAST = visitExpression(c);
            if (condAST && condAST->typeCode != T_NONE && condAST->typeCode != T_BOOLEAN) {
                addError("Type mismatch in REPEAT-UNTIL condition: expected Boolean");
            }
            if (condAST) ast->add(condAST);
        }
    }
    return ast;
}

ASTNode* SemanticAnalyzer::visitCaseStatement(ParseTreeNode* n) {
    ASTNode* ast = makeAST(AST_CASE);
    ASTNode* condAST = nullptr;

    for (auto* c : n->children) {
        if (isTok(c, "ofsy") || isTok(c, "endsy")) continue;

        if (isNT(c, "<expression>")) {
            condAST = visitExpression(c);
            if (condAST) ast->add(condAST);
        } else if (isNT(c, "<case-block>")) {
            ASTNode* cb = makeAST(AST_CASE_BLOCK);
            for (auto* cc : c->children) {
                if (isTok(cc, "colon") || isTok(cc, "comma") || isTok(cc, "semicolon")) continue;

                if (isNT(cc, "<constant>")) {
                    ASTNode* cst = makeAST(AST_INT_LIT, std::to_string(evalConstant(cc)));
                    cst->typeCode = typeOfConstant(cc);

                    if (condAST && condAST->typeCode != T_NONE && cst->typeCode != T_NONE) {
                        if (condAST->typeCode != cst->typeCode) {
                            addError("Case label constant type mismatch with case expression");
                        }
                    }
                    cb->add(cst);
                } else {
                    ASTNode* stmtAST = visitStatement(cc);
                    if (stmtAST) cb->add(stmtAST);
                }
            }
            ast->add(cb);
        }
    }
    return ast;
}

ASTNode* SemanticAnalyzer::visitProcCall(ParseTreeNode* n) {
    ASTNode* ast = makeAST(AST_PROC_CALL);
    for (auto* c : n->children) {
        if (isTok(c, "ident")) {
            ast->value = tokVal(c);
            int idx = symtab.lookup(tokVal(c));
            if (idx >= 0) {
                ast->tabIndex = idx;
                ast->typeCode = symtab.tab[idx].type;
            }
        } else if (isNT(c, "<parameter-list>")) {
            for (auto* p : c->children) {
                if (isTok(p, "comma")) continue;
                if (isNT(p, "<expression>")) ast->add(visitExpression(p));
            }
        }
    }
    return ast;
}

ASTNode* SemanticAnalyzer::visitExpression(ParseTreeNode* n) {
    if (!n) return makeAST(AST_INT_LIT, "0");
    if (n->children.size() == 1 && isNT(n->children[0], "<simple-expression>"))
        return visitSimpleExpression(n->children[0]);

    ASTNode* ast = makeAST(AST_BINOP);
    ASTNode* left = nullptr;
    ASTNode* right = nullptr;
    std::string relop;

    for (auto* c : n->children) {
        if (isNT(c, "<simple-expression>")) {
            if (!left) left = visitSimpleExpression(c);
            else right = visitSimpleExpression(c);
        } else if (isNT(c, "<relational-operator>")) {
            for (auto* op : c->children) relop = tokTy(op);
        }
    }

    if (left && right) {
        if (left->typeCode == T_NONE || right->typeCode == T_NONE) {
        } else if (left->typeCode != right->typeCode) {
            if (!((left->typeCode == T_INTEGER && right->typeCode == T_REAL) ||
                  (left->typeCode == T_REAL && right->typeCode == T_INTEGER))) {
                addError("Type mismatch in relational operator '" + relop +
                         "': cannot compare " + typeName(left->typeCode) +
                         " and " + typeName(right->typeCode));
            }
        }
        ast->typeCode = T_BOOLEAN;
        ast->add(left);
        ast->add(right);
        ast->op = relop;
    }
    return ast;
}

ASTNode* SemanticAnalyzer::visitSimpleExpression(ParseTreeNode* n) {
    if (n->children.empty()) return makeAST(AST_INT_LIT, "0");
    if (n->children.size() == 1 && isNT(n->children[0], "<term>"))
        return visitTerm(n->children[0]);

    ASTNode* ast = makeAST(AST_BINOP);
    std::string curOp;
    ASTNode* left = nullptr;
    for (auto* c : n->children) {
        if (isTok(c, "plus") || isTok(c, "minus") || isTok(c, "orsy")) {
            curOp = tokTy(c); continue;
        }
        if (isNT(c, "<additive-operator>")) {
            for (auto* op : c->children) curOp = tokTy(op);
            continue;
        }
        if (isNT(c, "<term>")) {
            ASTNode* t = visitTerm(c);
            if (!left) { 
                if (curOp == "minus" || curOp == "plus") {
                    ASTNode* unop = makeAST(AST_UNOP);
                    unop->op = curOp == "minus" ? "-" : "+";
                    unop->add(t);
                    unop->typeCode = t->typeCode;
                    left = unop;
                } else {
                    left = t; 
                }
            }
            else {
                ast->op = curOp;
                ast->add(left);
                ast->add(t);
                ast->typeCode = (left->typeCode == T_REAL || t->typeCode == T_REAL) ? T_REAL : (curOp == "orsy") ? T_BOOLEAN : T_INTEGER;
                left = ast;
                ast = makeAST(AST_BINOP);
            }
        }
    }
    if (left && ast->children.empty()) return left;
    return ast;
}

ASTNode* SemanticAnalyzer::visitTerm(ParseTreeNode* n) {
    if (n->children.size() == 1 && isNT(n->children[0], "<factor>"))
        return visitFactor(n->children[0]);

    ASTNode* ast = makeAST(AST_BINOP);
    std::string curOp;
    ASTNode* left = nullptr;
    for (auto* c : n->children) {
        if (isNT(c, "<multiplicative-operator>")) {
            for (auto* op : c->children) curOp = tokTy(op);
            continue;
        }
        if (isTok(c, "times") || isTok(c, "rdiv") || isTok(c, "idiv") ||
            isTok(c, "imod") || isTok(c, "andsy")) {
            curOp = tokTy(c); continue;
        }
        if (isNT(c, "<factor>")) {
            ASTNode* f = visitFactor(c);
            if (!left) { left = f; }
            else {
                ast->op = curOp;
                ast->add(left);
                ast->add(f);
                ast->typeCode = (left->typeCode == T_REAL || f->typeCode == T_REAL) ? T_REAL : (curOp == "andsy") ? T_BOOLEAN : T_INTEGER;
                left = ast;
                ast = makeAST(AST_BINOP);
            }
        }
    }
    if (left && ast->children.empty()) return left;
    return ast;
}

ASTNode* SemanticAnalyzer::visitFactor(ParseTreeNode* n) {
    for (auto* c : n->children) {
        if (isTok(c, "intcon")) {
            ASTNode* a = makeAST(AST_INT_LIT, tokVal(c));
            a->typeCode = T_INTEGER;
            return a;
        }
        if (isTok(c, "realcon")) {
            ASTNode* a = makeAST(AST_REAL_LIT, tokVal(c));
            a->typeCode = T_REAL;
            return a;
        }
        if (isTok(c, "charcon")) {
            ASTNode* a = makeAST(AST_CHAR_LIT, tokVal(c));
            a->typeCode = T_CHAR;
            return a;
        }
        if (isTok(c, "string")) {
            ASTNode* a = makeAST(AST_STR_LIT, tokVal(c));
            a->typeCode = T_STRING;
            return a;
        }
        if (isTok(c, "ident")) {
            ASTNode* a = makeAST(AST_VAR, tokVal(c));
            int idx = symtab.lookup(tokVal(c));
            if (idx >= 0) { a->tabIndex = idx; a->typeCode = symtab.tab[idx].type; }
            else addError("Identifier tidak dideklarasikan: " + tokVal(c));
            return a;
        }
        if(isTok(c, "notsy")){
            ASTNode* a = makeAST(AST_UNOP);
            a->op = "not";
            a->typeCode = T_BOOLEAN;
            bool foundNot = false;
            for (auto* inner : n->children){
                if (isTok(inner, "notsy")){
                    foundNot = true;
                    continue;
                }
                if (foundNot && isNT(inner, "<factor>")){
                    a->add(visitFactor(inner));
                    return a;
                }
            }
            return a;
        }
        if (isNT(c, "<expression>"))           return visitExpression(c);
        if (isNT(c, "<variable>"))             return visitVariable(c);
        if (isNT(c, "<procedure/function-call>")) return visitProcCall(c);
    }
    return makeAST(AST_INT_LIT, "0");
}

ASTNode* SemanticAnalyzer::visitVariable(ParseTreeNode* n) {
    if (!n) return makeAST(AST_EMPTY);

    bool isArrayAccess = false;
    if (isNT(n, "<component-variable>") || isNT(n, "<indexed-variable>")) {
        isArrayAccess = true;
    } else {
        for (auto* c : n->children) {
            if (isNT(c, "<component-variable>") || isNT(c, "<indexed-variable>")) {
                isArrayAccess = true;
                break;
            }
        }
    }

    if (isArrayAccess) {
        ASTNode* ast = makeAST(AST_ARRAY_ACCESS);

        ParseTreeNode* idNode = nullptr;
        ParseTreeNode* idxNode = nullptr;

        std::vector<ParseTreeNode*> stack = {n};
        while (!stack.empty()) {
            ParseTreeNode* curr = stack.back();
            stack.pop_back();

            if (isTok(curr, "ident") && !idNode) {
                idNode = curr;
            }
            else if (isNT(curr, "<index-list>") && !idxNode) {
                idxNode = curr;
            }

            for (int i = (int)curr->children.size() - 1; i >= 0; i--) {
                stack.push_back(curr->children[i]);
            }
        }

        if (idNode) {
            std::string arrName = tokVal(idNode);
            ast->value = arrName;
            int idx = symtab.lookup(arrName);

            if (idx >= 0) {
                ast->tabIndex = idx;
                ast->level = symtab.tab[idx].lev;
                int typeCode = symtab.tab[idx].type;
                int atabRef = symtab.tab[idx].ref;

                if (typeCode != T_ARRAY) {
                    addError("Variabel '" + arrName + "' bukan array");
                } else {
                    int realAtabIdx = atabRef;
                    if (realAtabIdx == (int)symtab.atab.size()) realAtabIdx -= 1;

                    if (realAtabIdx >= 0 && realAtabIdx < (int)symtab.atab.size()) {
                        ast->typeCode = symtab.atab[realAtabIdx].etyp;
                    }
                }
            } else {
                addError("Variabel tidak dideklarasikan: " + arrName);
            }
        }

        if (idxNode) {
            ParseTreeNode* targetIdxTok = nullptr;
            std::vector<ParseTreeNode*> q = {idxNode};
            while(!q.empty()) {
                auto* curr = q.back(); q.pop_back();
                if (isTok(curr, "intcon") || isTok(curr, "ident")) {
                    targetIdxTok = curr;
                    break;
                }
                for (int i = (int)curr->children.size() - 1; i >= 0; i--) {
                    q.push_back(curr->children[i]);
                }
            }

            if (targetIdxTok) {
                ASTNode* indexAst = nullptr;
                if (isTok(targetIdxTok, "intcon")) {
                    indexAst = makeAST(AST_INT_LIT, tokVal(targetIdxTok));
                    indexAst->typeCode = T_INTEGER;
                } else if (isTok(targetIdxTok, "ident")) {
                    indexAst = makeAST(AST_VAR, tokVal(targetIdxTok));
                    int iRef = symtab.lookup(tokVal(targetIdxTok));
                    if (iRef >= 0) {
                        indexAst->tabIndex = iRef;
                        indexAst->level = symtab.tab[iRef].lev;
                        indexAst->typeCode = symtab.tab[iRef].type;
                    }
                }

                if (indexAst) {
                    ast->add(indexAst);
                    if (indexAst->typeCode != T_NONE && indexAst->typeCode != T_INTEGER) {
                        addError("Array index must be of Integer type");
                    }
                }
            }
        }
        return ast;
    }

    ASTNode* ast = makeAST(AST_VAR);
    ParseTreeNode* idNode = nullptr;

    for (auto* c : n->children) {
        if (isTok(c, "ident")) {
            idNode = c;
            break;
        }
    }

    if (idNode) {
        ast->value = tokVal(idNode);
        int idx = symtab.lookup(ast->value);
        if (idx >= 0) {
            ast->tabIndex = idx;
            ast->level = symtab.tab[idx].lev;
            ast->typeCode = symtab.tab[idx].type;
        } else {
            addError("Variabel tidak dideklarasikan: " + ast->value);
        }
    }

    return ast;
}

void SemanticAnalyzer::printResults(std::ostream& out) const {
    symtab.printTab(out);
    symtab.printBtab(out);
    symtab.printAtab(out);
}
