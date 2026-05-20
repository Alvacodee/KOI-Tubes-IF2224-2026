#include "semantic.hpp"
#include <sstream>
#include <stdexcept>

// ===================================================================
// CONST DECLARATION
// <const-declaration>: constsy (ident eql <constant> semicolon)+
// ===================================================================
ASTNode* SemanticAnalyzer::visitConstDeclaration(ParseTreeNode* n) {
    ASTNode* ast = makeAST(AST_CONST_DECL);

    // Skip child[0] = constsy, lalu iterasi grup (ident eql constant semi)
    int i = 1;
    while (i < (int)n->children.size()) {
        ParseTreeNode* identNode = nullptr;
        ParseTreeNode* constNode = nullptr;

        // Kumpulkan ident dan <constant> dari grup ini
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

        // Cek redeklakarasi di scope yang sama
        int existing = symtab.lookup(name);
        if (existing >= (int)symtab.FIRST_USER && symtab.tab[existing].lev == symtab.curLevel) {
            addError("Redeklakarasi identifier: " + name);
            continue;
        }

        int tc  = typeOfConstant(constNode);
        int val = evalConstant(constNode);

        int idx = symtab.enter(name, OBJ_CONST, tc, 0, 1, val);

        // Buat node AST
        ASTNode* cAst = makeAST(AST_CONST_DECL, name);
        cAst->typeCode = tc;
        cAst->tabIndex = idx;
        cAst->level    = symtab.curLevel;
        ast->add(cAst);
    }
    return ast;
}

// ===================================================================
// TYPE DECLARATION
// <type-declaration>: typesy (ident eql <type> semicolon)+
// ===================================================================
ASTNode* SemanticAnalyzer::visitTypeDeclaration(ParseTreeNode* n) {
    ASTNode* ast = makeAST(AST_TYPE_DECL);

    int i = 1; // skip typesy
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

        // Cek redeklakarasi
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

// ===================================================================
// VAR DECLARATION
// <var-declaration>: varsy (<identifier-list> colon <type> semicolon)+
// ===================================================================
ASTNode* SemanticAnalyzer::visitVarDeclaration(ParseTreeNode* n) {
    ASTNode* ast = makeAST(AST_VAR_DECL);

    int i = 1; // skip varsy
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

        // Iterasi setiap identifier dalam daftar
        for (auto* idChild : idListNode->children) {
            if (!isTok(idChild, "ident")) continue;
            std::string name = tokVal(idChild);

            // Cek redeklakarasi di scope yang sama
            int existing = symtab.lookup(name);
            if (existing >= (int)symtab.FIRST_USER && symtab.tab[existing].lev == symtab.curLevel) {
                addError("Redeklakarasi variabel: " + name);
                continue;
            }

            // adr = offset dalam stack frame (vsze sekarang)
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

// ===================================================================
// SUBPROGRAM DECLARATION: dispatch ke procedure / function
// ===================================================================
ASTNode* SemanticAnalyzer::visitSubprogramDeclaration(ParseTreeNode* n) {
    for (auto* c : n->children) {
        if (isNT(c, "<procedure-declaration>")) return visitProcedureDeclaration(c);
        if (isNT(c, "<function-declaration>"))  return visitFunctionDeclaration(c);
    }
    return makeAST(AST_PROC_DECL);
}

// ===================================================================
// PROCEDURE DECLARATION
// proceduresy ident (param-list)? semicolon block semicolon
// ===================================================================
ASTNode* SemanticAnalyzer::visitProcedureDeclaration(ParseTreeNode* n) {
    ASTNode* ast = makeAST(AST_PROC_DECL);

    std::string name;
    for (auto* c : n->children)
        if (isTok(c, "ident")) { name = tokVal(c); break; }

    // Cek redeklakarasi
    int existing = symtab.lookup(name);
    if (existing >= (int)symtab.FIRST_USER && symtab.tab[existing].lev == symtab.curLevel)
        addError("Redeklakarasi prosedur: " + name);

    // Tentukan indeks btab untuk blok prosedur ini
    int bIdx = (int)symtab.btab.size();
    int idx = symtab.enter(name, OBJ_PROC, T_NONE, bIdx, 1, 0);
    ast->value    = name;
    ast->tabIndex = idx;
    ast->level    = symtab.curLevel;

    // Buka blok untuk prosedur
    symtab.openBlock();

    // Parameter formal
    int lastParam = 0, psze = 0;
    for (auto* c : n->children) {
        if (isNT(c, "<formal-parameter-list>"))
            ast->add(visitFormalParameterList(c, lastParam, psze));
    }
    symtab.btab[symtab.curBlock].lpar = lastParam;
    symtab.btab[symtab.curBlock].psze = psze;

    // Block prosedur
    for (auto* c : n->children) {
        if (isNT(c, "<block>")) ast->add(visitBlock(c));
    }

    symtab.closeBlock();
    return ast;
}

// ===================================================================
// FUNCTION DECLARATION
// functionsy ident (param-list)? colon ident semicolon block semicolon
// ===================================================================
ASTNode* SemanticAnalyzer::visitFunctionDeclaration(ParseTreeNode* n) {
    ASTNode* ast = makeAST(AST_FUNC_DECL);

    std::string name;
    std::string retTypeName;

    // Ambil nama dan tipe return
    // children: functionsy, ident(name), [param-list], colon, ident(retType), semi, block, semi
    bool foundName = false;
    for (auto* c : n->children) {
        if (isTok(c, "functionsy")) continue;
        if (isTok(c, "ident") && !foundName) { name = tokVal(c); foundName = true; }
        else if (isTok(c, "ident"))           { retTypeName = tokVal(c); }
        else if (isTok(c, "colon"))           continue;
    }

    // Resolve return type
    int retType = T_NONE;
    int retIdx = symtab.lookup(retTypeName);
    if (retIdx >= 0 && symtab.tab[retIdx].obj == OBJ_TYPE)
        retType = symtab.tab[retIdx].type;
    else if (!retTypeName.empty())
        addError("Tipe return tidak dikenal: " + retTypeName);

    // Cek redeklakarasi
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

// ===================================================================
// FORMAL PARAMETER LIST
// lparent (param-group (semicolon param-group)*)? rparent
// ===================================================================
ASTNode* SemanticAnalyzer::visitFormalParameterList(ParseTreeNode* n,
                                                     int& lastParam, int& psze) {
    ASTNode* ast = makeAST(AST_PARAM_LIST);

    for (auto* c : n->children) {
        if (!isNT(c, "<parameter-group>")) continue;

        // <parameter-group>: <identifier-list> colon (ident | <array-type>)
        ParseTreeNode* idList = nullptr;
        int tc = T_NONE, ref = 0;

        for (auto* pc : c->children) {
            if (isNT(pc, "<identifier-list>"))  idList = pc;
            else if (isTok(pc, "ident")) {
                // tipe parameter
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

// ===================================================================
// RESOLVE TYPE dari node <type>
// Kembalikan (typeCode, ref)
// ===================================================================
std::pair<int,int> SemanticAnalyzer::visitType(ParseTreeNode* n) {
    if (!n || n->children.empty()) return {T_NONE, 0};

    ParseTreeNode* child = n->children[0];

    // Tipe bernama: ident(integer), ident(MyRecord), dsb
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
        // Named type: kembalikan (type, ref ke definisi type di tab)
        // Untuk named record/array, ref = e.ref (atab/btab)
        return {e.type, e.ref};
    }

    // Array type
    if (isNT(child, "<array-type>")) return visitArrayType(child);

    // Enumerated: (A, B, C) -> buat tipe integer (simplified)
    if (isNT(child, "<enumerated>")) {
        // Setiap ident dalam enumerated = konstanta integer
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

    // Subrange: <range> -> <constant> period period <constant>
    if (isNT(child, "<range>")) {
        int tc = T_INTEGER;  // subrange default integer
        if (!child->children.empty())
            tc = typeOfConstant(child->children[0]);  // tipe dari batas bawah
        // Cek batas bawah <= batas atas
        if (child->children.size() >= 4) {
            int lo = evalConstant(child->children[0]);
            int hi = evalConstant(child->children[3]);
            if (lo > hi) addError("Batas bawah range lebih besar dari batas atas");
            if (tc == T_REAL) addError("Subrange tidak boleh bertipe Real");
        }
        return {tc, 0};
    }

    // Record type
    if (isNT(child, "<record-type>")) {
        int bIdx = symtab.openBlock();
        int prevBlock = symtab.curBlock - 1; // blok sebelumnya
        (void)prevBlock;

        // Proses field-list
        ParseTreeNode* fl = nullptr;
        for (auto* rc : child->children) {
            if (isNT(rc, "<field-list>")) { fl = rc; break; }
        }
        if (fl) {
            for (auto* fp : fl->children) {
                if (!isNT(fp, "<field-part>")) continue;
                // <field-part>: <identifier-list> colon <type>
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

// ===================================================================
// ARRAY TYPE
// arraysy lbrack (range|ident) rbrack ofsy <type>
// ===================================================================
std::pair<int,int> SemanticAnalyzer::visitArrayType(ParseTreeNode* n) {
    int xtyp = T_INTEGER;
    int low  = 0, high = 0;
    int etyp = T_NONE, eref = 0;

    for (auto* c : n->children) {
        if (isTok(c, "arraysy") || isTok(c, "lbrack") ||
            isTok(c, "rbrack") || isTok(c, "ofsy")) continue;

        if (isNT(c, "<range>")) {
            // <range>: <constant> period period <constant>
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
            // Index berupa named type (e.g., Boolean)
            int tIdx = symtab.lookup(tokVal(c));
            if (tIdx >= 0) xtyp = symtab.tab[tIdx].type;
            low = 0; high = 1; // default untuk boolean-indexed
        } else if (isNT(c, "<type>")) {
            auto [tc, ref] = visitType(c);
            etyp = tc; eref = ref;
        }
    }

    int elsz = symtab.typeSize(etyp, eref);
    int aIdx = symtab.newArray(xtyp, etyp, eref, low, high, elsz);
    return {T_ARRAY, aIdx};
}

// ===================================================================
// Evaluasi nilai <constant> menjadi integer
// (untuk konstanta integer; untuk char ambil ASCII; real jadi integer)
// ===================================================================
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

// ===================================================================
// Tipe dari <constant>
// ===================================================================
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
