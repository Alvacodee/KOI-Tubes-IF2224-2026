#include "semantic.hpp"
#include <sstream>

static int arithmeticResultType(int lt, int rt) {
    if (lt == T_REAL || rt == T_REAL) return T_REAL;
    if (lt == T_INTEGER && rt == T_INTEGER) return T_INTEGER;
    return T_NONE;
}

static bool isOrdinal(int t) {
    return t == T_INTEGER || t == T_BOOLEAN || t == T_CHAR;
}

static bool assignCompatible(int t1, int t2) {
    if (t1 == t2) return true;
    if (t1 == T_REAL && t2 == T_INTEGER) return true;
    if (t1 == T_STRING && t2 == T_STRING) return true;
    // rule ii: compatible ordinal
    if (isOrdinal(t1) && isOrdinal(t2)) return true;
    return false;
}

static bool relCompatible(int t1, int t2) {
    if (t1 == t2) return true;
    if ((t1 == T_INTEGER || t1 == T_REAL) &&
        (t2 == T_INTEGER || t2 == T_REAL)) return true;
    return false;
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

    if (isNT(n, "<assignment-statement>"))     return visitAssignStatement(n);
    if (isNT(n, "<if-statement>"))             return visitIfStatement(n);
    if (isNT(n, "<while-statement>"))          return visitWhileStatement(n);
    if (isNT(n, "<for-statement>"))            return visitForStatement(n);
    if (isNT(n, "<repeat-statement>"))         return visitRepeatStatement(n);
    if (isNT(n, "<case-statement>"))           return visitCaseStatement(n);
    if (isNT(n, "<procedure/function-call>"))  return visitProcCall(n);
    if (isNT(n, "<compound-statement>"))       return visitCompoundStatement(n);
    if (isNT(n, "<empty-statement>"))          return makeAST(AST_EMPTY);
    // statement kosong (node langsung berisi semicolon)
    if (isTok(n, "semicolon"))                 return makeAST(AST_EMPTY);

    return makeAST(AST_EMPTY);
}

ASTNode* SemanticAnalyzer::visitAssignStatement(ParseTreeNode* n) {
    ASTNode* ast = makeAST(AST_ASSIGN);

    ASTNode* varNode  = nullptr;
    ASTNode* exprNode = nullptr;

    for (auto* c : n->children) {
        if (isTok(c, "becomes")) continue;

        if (isNT(c, "<variable>") || isNT(c, "<component-variable>")) {
            varNode = visitVariable(c);
            ast->add(varNode);
        } else if (isTok(c, "ident") && !varNode) {
            // ident langsung (tanpa node <variable>) — pola lama di parse tree
            varNode = makeAST(AST_VAR, tokVal(c));
            int idx = symtab.lookup(tokVal(c));
            if (idx >= 0) {
                varNode->tabIndex = idx;
                varNode->typeCode = symtab.tab[idx].type;
                varNode->typeRef  = symtab.tab[idx].ref;
                varNode->level    = symtab.tab[idx].lev;
            } else {
                addError("Variabel tidak dideklarasikan: " + tokVal(c));
            }
            ast->add(varNode);
        } else if (isNT(c, "<expression>")) {
            exprNode = visitExpression(c);
            ast->add(exprNode);
        }
    }

    // Type checking assignment
    if (varNode && exprNode) {
        int varType  = varNode->typeCode;
        int exprType = exprNode->typeCode;

        if (varType != T_NONE && exprType != T_NONE) {
            if (!assignCompatible(varType, exprType)) {
                addError("Assignment tidak kompatibel: tidak bisa assign '"
                         + typeName(exprType) + "' ke '"
                         + typeName(varType) + "'");
            }
        }
    }

    ast->typeCode = T_NONE;
    return ast;
}

ASTNode* SemanticAnalyzer::visitIfStatement(ParseTreeNode* n) {
    ASTNode* ast = makeAST(AST_IF);

    for (auto* c : n->children) {
        if (isTok(c, "ifsy") || isTok(c, "thensy") ||
            isTok(c, "elsesy")) continue;

        if (isNT(c, "<expression>")) {
            ASTNode* cond = visitExpression(c);
            if (cond->typeCode != T_BOOLEAN && cond->typeCode != T_NONE)
                addError("Kondisi if harus bertipe Boolean, ditemukan: "
                         + typeName(cond->typeCode));
            ast->add(cond);
        } else {
            ast->add(visitStatement(c));
        }
    }
    return ast;
}

ASTNode* SemanticAnalyzer::visitWhileStatement(ParseTreeNode* n) {
    ASTNode* ast = makeAST(AST_WHILE);

    for (auto* c : n->children) {
        if (isTok(c, "whilesy") || isTok(c, "dosy") ||
            isTok(c, "semicolon")) continue;

        if (isNT(c, "<expression>")) {
            ASTNode* cond = visitExpression(c);
            if (cond->typeCode != T_BOOLEAN && cond->typeCode != T_NONE)
                addError("Kondisi while harus bertipe Boolean, ditemukan: "
                         + typeName(cond->typeCode));
            ast->add(cond);
        } else if (isNT(c, "<compound-statement>")) {
            ast->add(visitCompoundStatement(c));
        }
    }
    return ast;
}

ASTNode* SemanticAnalyzer::visitForStatement(ParseTreeNode* n) {
    ASTNode* ast = makeAST(AST_FOR);

    bool pastBecomes  = false;
    bool pastDir      = false;  // setelah to/downto
    int  exprCount    = 0;
    int  counterType  = T_INTEGER;

    for (auto* c : n->children) {
        if (isTok(c, "forsy") || isTok(c, "dosy") ||
            isTok(c, "semicolon")) continue;

        if (isTok(c, "becomes")) { pastBecomes = true; continue; }
        if (isTok(c, "tosy") || isTok(c, "downtosy")) {
            ast->op = isTok(c, "tosy") ? "to" : "downto";
            pastDir = true;
            continue;
        }

        if (isTok(c, "ident") && !pastBecomes) {
            // Counter variable
            std::string cname = tokVal(c);
            ASTNode* cv = makeAST(AST_VAR, cname);
            int idx = symtab.lookup(cname);
            if (idx >= 0) {
                cv->tabIndex  = idx;
                cv->typeCode  = symtab.tab[idx].type;
                cv->level     = symtab.tab[idx].lev;
                counterType   = cv->typeCode;
                if (!isOrdinal(counterType))
                    addError("Variabel counter for harus bertipe ordinal: " + cname);
            } else {
                addError("Variabel counter for tidak dideklarasikan: " + cname);
            }
            ast->add(cv);
            continue;
        }

        if (isNT(c, "<expression>")) {
            ASTNode* e = visitExpression(c);
            if (!isOrdinal(e->typeCode) && e->typeCode != T_NONE)
                addError("Batas for harus bertipe ordinal, ditemukan: "
                         + typeName(e->typeCode));
            if (exprCount == 0 || pastDir)
                ast->add(e);
            exprCount++;
            continue;
        }

        if (isNT(c, "<compound-statement>")) {
            ast->add(visitCompoundStatement(c));
        }
    }

    ast->typeCode = T_NONE;
    return ast;
}

ASTNode* SemanticAnalyzer::visitRepeatStatement(ParseTreeNode* n) {
    ASTNode* ast = makeAST(AST_REPEAT);

    for (auto* c : n->children) {
        if (isTok(c, "repeatsy") || isTok(c, "untilsy")) continue;

        if (isNT(c, "<statement-list>")) {
            ast->add(visitStatementList(c));
        } else if (isNT(c, "<expression>")) {
            ASTNode* cond = visitExpression(c);
            if (cond->typeCode != T_BOOLEAN && cond->typeCode != T_NONE)
                addError("Kondisi repeat-until harus bertipe Boolean, ditemukan: "
                         + typeName(cond->typeCode));
            ast->add(cond);
        }
    }
    return ast;
}

ASTNode* SemanticAnalyzer::visitCaseStatement(ParseTreeNode* n) {
    ASTNode* ast = makeAST(AST_CASE);

    for (auto* c : n->children) {
        if (isTok(c, "casesy") || isTok(c, "ofsy") ||
            isTok(c, "endsy")) continue;

        if (isNT(c, "<expression>")) {
            ASTNode* sel = visitExpression(c);
            if (!isOrdinal(sel->typeCode) && sel->typeCode != T_NONE)
                addError("Selector case harus bertipe ordinal, ditemukan: "
                         + typeName(sel->typeCode));
            ast->add(sel);
        } else if (isNT(c, "<case-block>")) {
            ASTNode* cb = makeAST(AST_CASE_BLOCK);
            for (auto* cc : c->children) {
                if (isTok(cc, "colon") || isTok(cc, "comma") ||
                    isTok(cc, "semicolon")) continue;
                if (isNT(cc, "<constant>")) {
                    ASTNode* cst = makeAST(AST_INT_LIT,
                                          std::to_string(evalConstant(cc)));
                    cst->typeCode = typeOfConstant(cc);
                    cb->add(cst);
                } else {
                    cb->add(visitStatement(cc));
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
        if (isTok(c, "lparent") || isTok(c, "rparent") ||
            isTok(c, "semicolon")) continue;

        if (isTok(c, "ident")) {
            ast->value = tokVal(c);
            int idx = symtab.lookup(tokVal(c));
            if (idx >= 0) {
                int obj = symtab.tab[idx].obj;
                if (obj != OBJ_PROC && obj != OBJ_FUNC)
                    addError("'" + tokVal(c) + "' bukan prosedur atau fungsi");
                ast->tabIndex = idx;
                ast->typeCode = symtab.tab[idx].type;
            }
            // predefined seperti writeln/readln tidak throw error
        } else if (isNT(c, "<parameter-list>")) {
            for (auto* p : c->children) {
                if (isTok(p, "comma")) continue;
                if (isNT(p, "<expression>"))
                    ast->add(visitExpression(p));
            }
        }
    }
    return ast;
}

ASTNode* SemanticAnalyzer::visitExpression(ParseTreeNode* n) {
    if (!n) {
        ASTNode* a = makeAST(AST_INT_LIT, "0");
        a->typeCode = T_INTEGER;
        return a;
    }

    // Kumpulkan simple-expression dan relational-operator
    std::vector<ASTNode*> simples;
    std::string relOp;

    for (auto* c : n->children) {
        if (isNT(c, "<simple-expression>")) {
            simples.push_back(visitSimpleExpression(c));
        } else if (isNT(c, "<relational-operator>")) {
            for (auto* op : c->children) relOp = tokTy(op);
        }
        // token relational langsung (tanpa wrapper node)
        else if (isTok(c, "eql") || isTok(c, "neq") ||
                 isTok(c, "lss") || isTok(c, "leq") ||
                 isTok(c, "gtr") || isTok(c, "geq")) {
            relOp = tokTy(c);
        }
    }

    // Satu simple-expression saja → teruskan
    if (simples.size() == 1 && relOp.empty()) return simples[0];

    // Ada relational operator
    ASTNode* ast = makeAST(AST_BINOP);
    ast->op = relOp;

    if (simples.size() >= 1) ast->add(simples[0]);
    if (simples.size() >= 2) ast->add(simples[1]);

    // Cek compatibility
    if (simples.size() >= 2) {
        int lt = simples[0]->typeCode;
        int rt = simples[1]->typeCode;
        if (lt != T_NONE && rt != T_NONE && !relCompatible(lt, rt))
            addError("Tipe tidak kompatibel untuk operasi relasional: '"
                     + typeName(lt) + "' dan '" + typeName(rt) + "'");
    }

    ast->typeCode = T_BOOLEAN;
    return ast;
}

ASTNode* SemanticAnalyzer::visitSimpleExpression(ParseTreeNode* n) {
    if (!n || n->children.empty()) {
        ASTNode* a = makeAST(AST_INT_LIT, "0");
        a->typeCode = T_INTEGER;
        return a;
    }

    int unarySign = 0; // 0=none, 1=plus, -1=minus
    std::vector<std::pair<std::string, ASTNode*>> terms; // (op, node)
    std::string curOp;

    for (auto* c : n->children) {
        // Unary sign di depan
        if (isTok(c, "plus") && terms.empty())  { unarySign =  1; continue; }
        if (isTok(c, "minus") && terms.empty()) { unarySign = -1; continue; }

        // Additive operator
        if (isTok(c, "plus") || isTok(c, "minus") || isTok(c, "orsy")) {
            curOp = tokTy(c); continue;
        }
        if (isNT(c, "<additive-operator>")) {
            for (auto* op : c->children) curOp = tokTy(op);
            continue;
        }

        if (isNT(c, "<term>")) {
            ASTNode* t = visitTerm(c);
            terms.push_back({curOp, t});
            curOp = "";
        }
    }

    if (terms.empty()) {
        ASTNode* a = makeAST(AST_INT_LIT, "0");
        a->typeCode = T_INTEGER;
        return a;
    }

    // Terapkan unary sign
    ASTNode* result = terms[0].second;
    if (unarySign == -1) {
        ASTNode* unop = makeAST(AST_UNOP);
        unop->op = "minus";
        unop->typeCode = result->typeCode;
        unop->add(result);
        result = unop;
    }

    // Fold operator kiri ke kanan
    for (int i = 1; i < (int)terms.size(); i++) {
        std::string op = terms[i].first;
        ASTNode* right = terms[i].second;

        ASTNode* binop = makeAST(AST_BINOP);
        binop->op = op;
        binop->add(result);
        binop->add(right);

        int lt = result->typeCode;
        int rt = right->typeCode;

        if (op == "orsy") {
            // boolean or boolean → boolean
            if ((lt != T_BOOLEAN && lt != T_NONE) ||
                (rt != T_BOOLEAN && rt != T_NONE))
                addError("Operand 'or' harus bertipe Boolean");
            binop->typeCode = T_BOOLEAN;
        } else {
            // plus / minus aritmatika
            if (lt != T_NONE && rt != T_NONE) {
                int res = arithmeticResultType(lt, rt);
                if (res == T_NONE)
                    addError("Tipe tidak kompatibel untuk operator '"
                             + op + "': '"
                             + typeName(lt) + "' dan '" + typeName(rt) + "'");
                binop->typeCode = res;
            } else {
                binop->typeCode = (lt != T_NONE) ? lt : rt;
            }
        }
        result = binop;
    }

    return result;
}

ASTNode* SemanticAnalyzer::visitTerm(ParseTreeNode* n) {
    if (!n || n->children.empty()) {
        ASTNode* a = makeAST(AST_INT_LIT, "0");
        a->typeCode = T_INTEGER;
        return a;
    }

    std::vector<std::pair<std::string, ASTNode*>> factors;
    std::string curOp;

    for (auto* c : n->children) {
        if (isNT(c, "<multiplicative-operator>")) {
            for (auto* op : c->children) curOp = tokTy(op);
            continue;
        }
        if (isTok(c, "times") || isTok(c, "rdiv") || isTok(c, "idiv") ||
            isTok(c, "imod")  || isTok(c, "andsy")) {
            curOp = tokTy(c); continue;
        }
        if (isNT(c, "<factor>")) {
            factors.push_back({curOp, visitFactor(c)});
            curOp = "";
        }
    }

    if (factors.empty()) {
        ASTNode* a = makeAST(AST_INT_LIT, "0");
        a->typeCode = T_INTEGER;
        return a;
    }

    ASTNode* result = factors[0].second;

    for (int i = 1; i < (int)factors.size(); i++) {
        std::string op  = factors[i].first;
        ASTNode* right  = factors[i].second;
        int lt = result->typeCode;
        int rt = right->typeCode;

        ASTNode* binop = makeAST(AST_BINOP);
        binop->op = op;
        binop->add(result);
        binop->add(right);

        if (op == "andsy") {
            if ((lt != T_BOOLEAN && lt != T_NONE) ||
                (rt != T_BOOLEAN && rt != T_NONE))
                addError("Operand 'and' harus bertipe Boolean");
            binop->typeCode = T_BOOLEAN;

        } else if (op == "rdiv") {
            // '/' selalu menghasilkan Real
            if ((lt != T_INTEGER && lt != T_REAL && lt != T_NONE) ||
                (rt != T_INTEGER && rt != T_REAL && rt != T_NONE))
                addError("Operand '/' harus bertipe Integer atau Real");
            binop->typeCode = T_REAL;

        } else if (op == "idiv" || op == "imod") {
            // div dan mod hanya untuk Integer
            if ((lt != T_INTEGER && lt != T_NONE) ||
                (rt != T_INTEGER && rt != T_NONE))
                addError("Operand '" + op + "' harus bertipe Integer");
            binop->typeCode = T_INTEGER;

        } else {
            // times (*)
            if (lt != T_NONE && rt != T_NONE) {
                int res = arithmeticResultType(lt, rt);
                if (res == T_NONE)
                    addError("Tipe tidak kompatibel untuk operator '*': '"
                             + typeName(lt) + "' dan '" + typeName(rt) + "'");
                binop->typeCode = res;
            } else {
                binop->typeCode = (lt != T_NONE) ? lt : rt;
            }
        }
        result = binop;
    }

    return result;
}

ASTNode* SemanticAnalyzer::visitFactor(ParseTreeNode* n) {
    if (!n) {
        ASTNode* a = makeAST(AST_INT_LIT, "0");
        a->typeCode = T_INTEGER;
        return a;
    }

    for (auto* c : n->children) {

        // Literal integer
        if (isTok(c, "intcon")) {
            ASTNode* a = makeAST(AST_INT_LIT, tokVal(c));
            a->typeCode = T_INTEGER;
            return a;
        }

        // Literal real
        if (isTok(c, "realcon")) {
            ASTNode* a = makeAST(AST_REAL_LIT, tokVal(c));
            a->typeCode = T_REAL;
            return a;
        }

        // Literal char
        if (isTok(c, "charcon")) {
            ASTNode* a = makeAST(AST_CHAR_LIT, tokVal(c));
            a->typeCode = T_CHAR;
            return a;
        }

        // Literal string
        if (isTok(c, "string")) {
            ASTNode* a = makeAST(AST_STR_LIT, tokVal(c));
            a->typeCode = T_STRING;
            return a;
        }

        // NOT factor
        if (isTok(c, "notsy")) {
            // Cari factor-nya (child berikutnya atau child terakhir)
            ParseTreeNode* factNode = nullptr;
            for (auto* fc : n->children)
                if (isNT(fc, "<factor>")) { factNode = fc; break; }

            ASTNode* inner = factNode ? visitFactor(factNode)
                                      : makeAST(AST_BOOL_LIT, "false");
            if (inner->typeCode != T_BOOLEAN && inner->typeCode != T_NONE)
                addError("Operand 'not' harus bertipe Boolean, ditemukan: "
                         + typeName(inner->typeCode));

            ASTNode* a = makeAST(AST_UNOP);
            a->op = "not";
            a->typeCode = T_BOOLEAN;
            a->add(inner);
            return a;
        }

        // Ekspresi dalam kurung
        if (isTok(c, "lparent")) {
            for (auto* fc : n->children)
                if (isNT(fc, "<expression>")) return visitExpression(fc);
        }

        // Procedure/function call (sebagai ekspresi, artinya fungsi)
        if (isNT(c, "<procedure/function-call>")) {
            ASTNode* call = visitProcCall(c);
            // Tipe hasil = return type fungsi
            return call;
        }

        // Variable (termasuk array element dan record field)
        if (isNT(c, "<variable>") || isNT(c, "<component-variable>")) {
            return visitVariable(c);
        }

        // Identifier langsung di dalam factor
        if (isTok(c, "ident")) {
            std::string name = tokVal(c);

            // Cek apakah ini proc/func call dengan lookahead
            bool isCall = false;
            for (size_t i = 0; i < n->children.size(); i++) {
                if (n->children[i] == c &&
                    i + 1 < n->children.size() &&
                    isTok(n->children[i+1], "lparent")) {
                    isCall = true; break;
                }
            }

            if (isCall) {
                // Proses sebagai proc/func call
                ASTNode* call = visitProcCall(n);
                return call;
            }

            // Variabel biasa
            int idx = symtab.lookup(name);
            ASTNode* a = makeAST(AST_VAR, name);
            if (idx >= 0) {
                a->tabIndex = idx;
                a->typeCode = symtab.tab[idx].type;
                a->typeRef  = symtab.tab[idx].ref;
                a->level    = symtab.tab[idx].lev;
            } else {
                addError("Identifier tidak dideklarasikan: " + name);
            }
            return a;
        }
    }

    // Fallback
    ASTNode* a = makeAST(AST_INT_LIT, "0");
    a->typeCode = T_INTEGER;
    return a;
}

ASTNode* SemanticAnalyzer::visitVariable(ParseTreeNode* n) {
    if (!n) return makeAST(AST_VAR);

    // ident langsung
    if (isTok(n, "ident")) {
        std::string name = tokVal(n);
        ASTNode* a = makeAST(AST_VAR, name);
        int idx = symtab.lookup(name);
        if (idx >= 0) {
            a->tabIndex = idx;
            a->typeCode = symtab.tab[idx].type;
            a->typeRef  = symtab.tab[idx].ref;
            a->level    = symtab.tab[idx].lev;
        } else {
            addError("Variabel tidak dideklarasikan: " + name);
        }
        return a;
    }

    // <variable> atau <component-variable>
    ASTNode* base = nullptr;
    std::string baseName;

    for (size_t i = 0; i < n->children.size(); i++) {
        auto* c = n->children[i];

        if (isTok(c, "ident") && !base) {
            // Identifier dasar
            baseName = tokVal(c);
            base = makeAST(AST_VAR, baseName);
            int idx = symtab.lookup(baseName);
            if (idx >= 0) {
                base->tabIndex = idx;
                base->typeCode = symtab.tab[idx].type;
                base->typeRef  = symtab.tab[idx].ref;
                base->level    = symtab.tab[idx].lev;
            } else {
                addError("Variabel tidak dideklarasikan: " + baseName);
            }

        } else if (isTok(c, "lbrack")) {
            // Array element access: var[index]
            ASTNode* acc = makeAST(AST_ARRAY_ACCESS);

            // Tipe elemen array
            if (base && base->typeCode == T_ARRAY) {
                int aref = base->typeRef;
                if (aref > 0 && aref <= (int)symtab.atab.size()) {
                    acc->typeCode = symtab.atab[aref - 1].etyp;
                    acc->typeRef  = symtab.atab[aref - 1].eref;
                }
            }
            acc->add(base);

            // Kumpulkan indeks
            for (size_t j = i + 1; j < n->children.size(); j++) {
                auto* ic = n->children[j];
                if (isTok(ic, "rbrack")) { i = j; break; }
                if (isTok(ic, "comma")) continue;
                if (isNT(ic, "<index-list>")) {
                    for (auto* idx_c : ic->children) {
                        if (isTok(idx_c, "comma")) continue;
                        if (isNT(idx_c, "<expression>"))
                            acc->add(visitExpression(idx_c));
                        else if (isTok(idx_c, "intcon") ||
                                 isTok(idx_c, "charcon") ||
                                 isTok(idx_c, "ident")) {
                            ASTNode* idxNode = visitFactor(n); // fallback
                            (void)idxNode;
                        }
                    }
                    i = j;
                    break;
                }
                if (isNT(ic, "<expression>")) acc->add(visitExpression(ic));
            }
            base = acc;

        } else if (isTok(c, "period") && base) {
            // Field access: record.field
            if (i + 1 < n->children.size() && isTok(n->children[i+1], "ident")) {
                std::string fname = tokVal(n->children[i+1]);
                ASTNode* acc = makeAST(AST_FIELD_ACCESS, fname);

                // Cari tipe field dari btab
                if (base->typeCode == T_RECORD && base->typeRef > 0) {
                    int bref = base->typeRef;
                    // Cari field di blok record
                    if (bref < (int)symtab.btab.size()) {
                        int fi = symtab.btab[bref].last;
                        while (fi > 0) {
                            if (symtab.tab[fi].id == fname) {
                                acc->typeCode  = symtab.tab[fi].type;
                                acc->typeRef   = symtab.tab[fi].ref;
                                acc->tabIndex  = fi;
                                break;
                            }
                            fi = symtab.tab[fi].link;
                        }
                    }
                }
                acc->add(base);
                base = acc;
                i++; // lewati ident field
            }

        } else if (isNT(c, "<variable>") || isNT(c, "<component-variable>")) {
            base = visitVariable(c);
        }
    }

    return base ? base : makeAST(AST_VAR);
}