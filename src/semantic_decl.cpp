#include "semantic.hpp"
#include <sstream>
#include <stdexcept>

ASTNode* SemanticAnalyzer::visitConstDeclaration(ParseTreeNode* n) {
    ASTNode* ast = makeAST(AST_CONST_DECL);

    int i = 1;
    while (i < (int)n->children.size()) {
        ParseTreeNode* identNode = nullptr;
        ParseTreeNode* constNode = nullptr;

        while (i < (int)n->children.size()) {
            auto* c = n->children[i];
            if (isTok(c, "ident"))        { identNode = c; i++; }
            else if (isTok(c, "eql"))     { i++; }
            else if (isNT(c, "<constant>")){ constNode = c; i++; }
            else if (isTok(c, "semicolon")){ i++; break; }
            else i++;
        }

        if (!identNode || !constNode) continue;

        std::string name = tokVal(identNode);

        int existing = symtab.lookup(name);
        if (existing >= (int)symtab.FIRST_USER && symtab.tab[existing].lev == symtab.curLevel) {
            addError("Redeklakarasi identifier: " + name);
            continue;
        }

        int tc  = typeOfConstant(constNode);
        int val = evalConstant(constNode);

        int idx = symtab.enter(name, OBJ_CONST, tc, 0, 1, val);

        ASTNode* cAst = makeAST(AST_CONST_DECL, name);
        cAst->typeCode = tc;
        cAst->tabIndex = idx;
        cAst->level    = symtab.curLevel;
        ast->add(cAst);
    }
    return ast;
}


ASTNode* SemanticAnalyzer::visitTypeDeclaration(ParseTreeNode* n) {
    ASTNode* ast = makeAST(AST_TYPE_DECL);

    int i = 1;
    while (i < (int)n->children.size()) {
        ParseTreeNode* identNode = nullptr;
        ParseTreeNode* typeNode  = nullptr;

        while (i < (int)n->children.size()) {
            auto* c = n->children[i];
            if (isTok(c, "ident"))        { identNode = c; i++; }
            else if (isTok(c, "eql"))     { i++; }
            else if (isNT(c, "<type>"))   { typeNode = c; i++; }
            else if (isTok(c, "semicolon")){ i++; break; }
            else i++;
        }

        if (!identNode || !typeNode) continue;

        std::string name = tokVal(identNode);

        int existing = symtab.lookup(name);
        if (existing >= (int)symtab.FIRST_USER && symtab.tab[existing].lev == symtab.curLevel) {
            addError("Redeklakarasi tipe: " + name);
            continue;
        }

        auto [tc, ref] = visitType(typeNode);
        int idx = symtab.enter(name, OBJ_TYPE, tc, ref, 1, 0);

        ASTNode* tAst = makeAST(AST_TYPE_DECL, name);
        tAst->typeCode = tc;
        tAst->typeRef  = ref;
        tAst->tabIndex = idx;
        tAst->level    = symtab.curLevel;
        ast->add(tAst);
    }
    return ast;
}

ASTNode* SemanticAnalyzer::visitVarDeclaration(ParseTreeNode* n) {
    ASTNode* ast = makeAST(AST_VAR_DECL);

    int i = 1;
    while (i < (int)n->children.size()) {
        ParseTreeNode* idListNode = nullptr;
        ParseTreeNode* typeNode   = nullptr;

        while (i < (int)n->children.size()) {
            auto* c = n->children[i];
            if (isNT(c, "<identifier-list>")){ idListNode = c; i++; }
            else if (isTok(c, "colon"))      { i++; }
            else if (isNT(c, "<type>"))      { typeNode = c; i++; }
            else if (isTok(c, "semicolon"))  { i++; break; }
            else i++;
        }

        if (!idListNode || !typeNode) continue;

        auto [tc, ref] = visitType(typeNode);
        int sz = symtab.typeSize(tc, ref);

        for (auto* idChild : idListNode->children) {
            if (!isTok(idChild, "ident")) continue;
            std::string name = tokVal(idChild);

            int existing = symtab.lookup(name);
            if (existing >= (int)symtab.FIRST_USER && symtab.tab[existing].lev == symtab.curLevel) {
                addError("Redeklakarasi variabel: " + name);
                continue;
            }

            int offset = symtab.btab[symtab.curBlock].vsze;
            int idx = symtab.enter(name, OBJ_VARIABLE, tc, ref, 1, offset);
            symtab.btab[symtab.curBlock].vsze += sz;

            ASTNode* vAst = makeAST(AST_VAR_DECL, name);
            vAst->typeCode = tc;
            vAst->typeRef  = ref;
            vAst->tabIndex = idx;
            vAst->level    = symtab.curLevel;
            ast->add(vAst);
        }
    }
    return ast;
}

ASTNode* SemanticAnalyzer::visitSubprogramDeclaration(ParseTreeNode* n) {
    for (auto* c : n->children) {
        if (isNT(c, "<procedure-declaration>")) return visitProcedureDeclaration(c);
        if (isNT(c, "<function-declaration>"))  return visitFunctionDeclaration(c);
    }
    return makeAST(AST_PROC_DECL);
}

ASTNode* SemanticAnalyzer::visitProcedureDeclaration(ParseTreeNode* n) {
    ASTNode* ast = makeAST(AST_PROC_DECL);

    std::string name;
    for (auto* c : n->children)
        if (isTok(c, "ident")) { name = tokVal(c); break; }

    int existing = symtab.lookup(name);
    if (existing >= (int)symtab.FIRST_USER && symtab.tab[existing].lev == symtab.curLevel)
        addError("Redeklakarasi prosedur: " + name);

    int bIdx = (int)symtab.btab.size();
    int idx = symtab.enter(name, OBJ_PROC, T_NONE, bIdx, 1, 0);
    ast->value    = name;
    ast->tabIndex = idx;
    ast->level    = symtab.curLevel;

    symtab.openBlock();

    int lastParam = 0, psze = 0;
    for (auto* c : n->children) {
        if (isNT(c, "<formal-parameter-list>"))
            ast->add(visitFormalParameterList(c, lastParam, psze));
    }
    symtab.btab[symtab.curBlock].lpar = lastParam;
    symtab.btab[symtab.curBlock].psze = psze;

    for (auto* c : n->children) {
        if (isNT(c, "<block>")) ast->add(visitBlock(c));
    }

    symtab.closeBlock();
    return ast;
}

ASTNode* SemanticAnalyzer::visitFunctionDeclaration(ParseTreeNode* n) {
    ASTNode* ast = makeAST(AST_FUNC_DECL);

    std::string name;
    std::string retTypeName;

    bool foundName = false;
    for (auto* c : n->children) {
        if (isTok(c, "functionsy")) continue;
        if (isTok(c, "ident") && !foundName) { name = tokVal(c); foundName = true; }
        else if (isTok(c, "ident"))           { retTypeName = tokVal(c); }
        else if (isTok(c, "colon"))           continue;
    }

    int retType = T_NONE;
    int retIdx = symtab.lookup(retTypeName);
    if (retIdx >= 0 && symtab.tab[retIdx].obj == OBJ_TYPE)
        retType = symtab.tab[retIdx].type;
    else if (!retTypeName.empty())
        addError("Tipe return tidak dikenal: " + retTypeName);

    int existing = symtab.lookup(name);
    if (existing >= (int)symtab.FIRST_USER && symtab.tab[existing].lev == symtab.curLevel)
        addError("Redeklakarasi fungsi: " + name);

    int bIdx = (int)symtab.btab.size();
    int idx = symtab.enter(name, OBJ_FUNC, retType, bIdx, 1, 0);
    ast->value    = name;
    ast->typeCode = retType;
    ast->tabIndex = idx;
    ast->level    = symtab.curLevel;

    symtab.openBlock();

    int lastParam = 0, psze = 0;
    for (auto* c : n->children) {
        if (isNT(c, "<formal-parameter-list>"))
            ast->add(visitFormalParameterList(c, lastParam, psze));
    }
    symtab.btab[symtab.curBlock].lpar = lastParam;
    symtab.btab[symtab.curBlock].psze = psze;

    for (auto* c : n->children) {
        if (isNT(c, "<block>")) ast->add(visitBlock(c));
    }

    symtab.closeBlock();
    return ast;
}

ASTNode* SemanticAnalyzer::visitFormalParameterList(ParseTreeNode* n,
                                                     int& lastParam, int& psze) {
    ASTNode* ast = makeAST(AST_PARAM_LIST);

    for (auto* c : n->children) {
        if (!isNT(c, "<parameter-group>")) continue;

        ParseTreeNode* idList = nullptr;
        int tc = T_NONE, ref = 0;

        for (auto* pc : c->children) {
            if (isNT(pc, "<identifier-list>"))  idList = pc;
            else if (isTok(pc, "ident")) {
                int tIdx = symtab.lookup(tokVal(pc));
                if (tIdx >= 0 && symtab.tab[tIdx].obj == OBJ_TYPE)
                    tc = symtab.tab[tIdx].type;
                else addError("Tipe parameter tidak dikenal: " + tokVal(pc));
            } else if (isNT(pc, "<array-type>")) {
                auto [atc, aref] = visitArrayType(pc);
                tc = atc; ref = aref;
            }
        }

        if (!idList) continue;

        ASTNode* grpAst = makeAST(AST_PARAM_GROUP);
        for (auto* idChild : idList->children) {
            if (!isTok(idChild, "ident")) continue;
            std::string pname = tokVal(idChild);
            int offset = psze;
            int sz = symtab.typeSize(tc, ref);
            int pidx = symtab.enter(pname, OBJ_VARIABLE, tc, ref, 1, offset);
            psze += sz;
            lastParam = pidx;

            ASTNode* pAst = makeAST(AST_VAR_DECL, pname);
            pAst->typeCode = tc; pAst->typeRef = ref;
            pAst->tabIndex = pidx; pAst->level = symtab.curLevel;
            grpAst->add(pAst);
        }
        ast->add(grpAst);
    }
    return ast;
}

std::pair<int,int> SemanticAnalyzer::visitType(ParseTreeNode* n) {
    if (!n || n->children.empty()) return {T_NONE, 0};

    ParseTreeNode* child = n->children[0];

    if (isTok(child, "ident")) {
        std::string tname = tokVal(child);
        int idx = symtab.lookup(tname);
        if (idx < 0) {
            addError("Tipe tidak dikenal: " + tname);
            return {T_NONE, 0};
        }
        auto& e = symtab.tab[idx];
        if (e.obj != OBJ_TYPE) {
            addError(tname + " bukan tipe data");
            return {T_NONE, 0};
        }
        return {e.type, e.ref};
    }

    if (isNT(child, "<array-type>")) return visitArrayType(child);

    if (isNT(child, "<enumerated>")) {
        int i = 0;
        for (auto* ec : child->children) {
            if (!isTok(ec, "ident")) continue;
            std::string eName = tokVal(ec);
            int existing = symtab.lookup(eName);
            if (existing >= (int)symtab.FIRST_USER && symtab.tab[existing].lev == symtab.curLevel) {
                addError("Redeklakarasi enum: " + eName);
            } else {
                symtab.enter(eName, OBJ_CONST, T_INTEGER, 0, 1, i);
            }
            i++;
        }
        return {T_INTEGER, 0};
    }

    if (isNT(child, "<range>")) {
        int tc = T_INTEGER;
        if (!child->children.empty())
            tc = typeOfConstant(child->children[0]);
        if (child->children.size() >= 4) {
            int lo = evalConstant(child->children[0]);
            int hi = evalConstant(child->children[3]);
            if (lo > hi) addError("Batas bawah range lebih besar dari batas atas");
            if (tc == T_REAL) addError("Subrange tidak boleh bertipe Real");
        }
        return {tc, 0};
    }

    if (isNT(child, "<record-type>")) {
        int bIdx = symtab.openBlock();
        int prevBlock = symtab.curBlock - 1;
        (void)prevBlock;

        ParseTreeNode* fl = nullptr;
        for (auto* rc : child->children) {
            if (isNT(rc, "<field-list>")) { fl = rc; break; }
        }
        if (fl) {
            for (auto* fp : fl->children) {
                if (!isNT(fp, "<field-part>")) continue;
                ParseTreeNode* fidList = nullptr;
                ParseTreeNode* ftype  = nullptr;
                for (auto* fpc : fp->children) {
                    if (isNT(fpc, "<identifier-list>")) fidList = fpc;
                    else if (isNT(fpc, "<type>"))       ftype  = fpc;
                }
                if (!fidList || !ftype) continue;
                auto [ftc, fref] = visitType(ftype);
                int fsz = symtab.typeSize(ftc, fref);
                for (auto* fid : fidList->children) {
                    if (!isTok(fid, "ident")) continue;
                    int offset = symtab.btab[symtab.curBlock].vsze;
                    symtab.enter(tokVal(fid), OBJ_VARIABLE, ftc, fref, 1, offset);
                    symtab.btab[symtab.curBlock].vsze += fsz;
                }
            }
        }

        symtab.closeBlock();
        return {T_RECORD, bIdx};
    }

    return {T_NONE, 0};
}

std::pair<int,int> SemanticAnalyzer::visitArrayType(ParseTreeNode* n) {
    int xtyp = T_INTEGER;
    int low  = 0, high = 0;
    int etyp = T_NONE, eref = 0;

    for (auto* c : n->children) {
        if (isTok(c, "arraysy") || isTok(c, "lbrack") ||
            isTok(c, "rbrack") || isTok(c, "ofsy")) continue;

        if (isNT(c, "<range>")) {
            if (c->children.size() >= 4) {
                xtyp = typeOfConstant(c->children[0]);
                low  = evalConstant(c->children[0]);
                high = evalConstant(c->children[3]);
                if (xtyp == T_REAL)
                    addError("Index array tidak boleh bertipe Real");
                if (low > high)
                    addError("Batas bawah array lebih besar dari batas atas");
            }
        } else if (isTok(c, "ident")) {
            int tIdx = symtab.lookup(tokVal(c));
            if (tIdx >= 0) xtyp = symtab.tab[tIdx].type;
            low = 0; high = 1;
        } else if (isNT(c, "<type>")) {
            auto [tc, ref] = visitType(c);
            etyp = tc; eref = ref;
        }
    }

    int elsz = symtab.typeSize(etyp, eref);
    int aIdx = symtab.newArray(xtyp, etyp, eref, low, high, elsz);
    return {T_ARRAY, aIdx};
}

int SemanticAnalyzer::evalConstant(ParseTreeNode* n) {
    if (!n) return 0;

    int sign = 1;
    for (auto* c : n->children) {
        if (isTok(c, "plus"))  { sign =  1; continue; }
        if (isTok(c, "minus")) { sign = -1; continue; }

        if (isTok(c, "intcon"))  return sign * std::stoi(tokVal(c));
        if (isTok(c, "realcon")) return sign * (int)std::stod(tokVal(c));
        if (isTok(c, "charcon")) {
            std::string v = tokVal(c);
            // 'x' -> ambil karakter di index 1
            return (v.size() >= 2) ? (int)(unsigned char)v[1] : 0;
        }
        if (isTok(c, "string")) return 0;
        if (isTok(c, "ident")) {
            std::string nm = tokVal(c);
            int idx = symtab.lookup(nm);
            if (idx >= 0 && symtab.tab[idx].obj == OBJ_CONST)
                return sign * symtab.tab[idx].adr;
            return 0;
        }
    }
    return 0;
}

int SemanticAnalyzer::typeOfConstant(ParseTreeNode* n) {
    if (!n) return T_NONE;
    for (auto* c : n->children) {
        if (isTok(c, "plus") || isTok(c, "minus")) continue;
        if (isTok(c, "intcon"))  return T_INTEGER;
        if (isTok(c, "realcon")) return T_REAL;
        if (isTok(c, "charcon")) return T_CHAR;
        if (isTok(c, "string"))  return T_STRING;
        if (isTok(c, "ident")) {
            std::string nm = tokVal(c);
            int idx = symtab.lookup(nm);
            if (idx >= 0) return symtab.tab[idx].type;
        }
    }
    return T_NONE;
}
