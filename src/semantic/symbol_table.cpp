#include "symbol_table.hpp"
#include <iomanip>
#include <iostream>

std::string SymbolTable::toLower(const std::string& s) const {
    std::string r = s;
    std::transform(r.begin(), r.end(), r.begin(), ::tolower);
    return r;
}

SymbolTable::SymbolTable() : curLevel(0), curBlock(0) {
    btab.push_back(BtabEntry(0, 0, 0, 0));
    display.push_back(0);

    initPredefined();
}

void SymbolTable::initPredefined() {
    auto add = [&](const std::string& id, int obj, int type, int ref, int nrm, int adr) {
        tab.push_back(TabEntry(toLower(id), 0, obj, type, ref, nrm, 0, adr));
    };

    add("and",       OBJ_KEYWORD, T_NONE, 0, 1, 0);
    add("array",     OBJ_KEYWORD, T_NONE, 0, 1, 0);
    add("begin",     OBJ_KEYWORD, T_NONE, 0, 1, 0);
    add("case",      OBJ_KEYWORD, T_NONE, 0, 1, 0);
    add("const",     OBJ_KEYWORD, T_NONE, 0, 1, 0);
    add("div",       OBJ_KEYWORD, T_NONE, 0, 1, 0);
    add("downto",    OBJ_KEYWORD, T_NONE, 0, 1, 0);
    add("do",        OBJ_KEYWORD, T_NONE, 0, 1, 0);
    add("else",      OBJ_KEYWORD, T_NONE, 0, 1, 0);
    add("end",       OBJ_KEYWORD, T_NONE, 0, 1, 0);
    add("for",       OBJ_KEYWORD, T_NONE, 0, 1, 0);
    add("function",  OBJ_KEYWORD, T_NONE, 0, 1, 0);
    add("if",        OBJ_KEYWORD, T_NONE, 0, 1, 0);
    add("mod",       OBJ_KEYWORD, T_NONE, 0, 1, 0);
    add("not",       OBJ_KEYWORD, T_NONE, 0, 1, 0);
    add("of",        OBJ_KEYWORD, T_NONE, 0, 1, 0);
    add("or",        OBJ_KEYWORD, T_NONE, 0, 1, 0);
    add("procedure", OBJ_KEYWORD, T_NONE, 0, 1, 0);
    add("program",   OBJ_KEYWORD, T_NONE, 0, 1, 0);
    add("record",    OBJ_KEYWORD, T_NONE, 0, 1, 0);
    add("repeat",    OBJ_KEYWORD, T_NONE, 0, 1, 0);
    add("integer",   OBJ_TYPE, T_INTEGER, 0, 1, 0);
    add("real",      OBJ_TYPE, T_REAL,    0, 1, 0);
    add("boolean",   OBJ_TYPE, T_BOOLEAN, 0, 1, 0);
    add("char",      OBJ_TYPE, T_CHAR,    0, 1, 0);
    add("string",    OBJ_TYPE, T_STRING,  0, 1, 0);
    add("then",      OBJ_KEYWORD, T_NONE, 0, 1, 0);
    add("to",        OBJ_KEYWORD, T_NONE, 0, 1, 0);
    add("type",      OBJ_KEYWORD, T_NONE, 0, 1, 0);
    add("until",     OBJ_KEYWORD, T_NONE, 0, 1, 0);
    add("var",       OBJ_KEYWORD, T_NONE, 0, 1, 0);
    add("while",     OBJ_KEYWORD, T_NONE, 0, 1, 0);
    add("false", OBJ_CONST, T_BOOLEAN, 0, 1, 0);
    add("true",  OBJ_CONST, T_BOOLEAN, 0, 1, 1);
    add("writeln", OBJ_PROC, T_NONE, 0, 1, 0);
    add("write", OBJ_PROC, T_NONE, 0, 1, 0);
    add("readln", OBJ_PROC, T_NONE, 0, 1, 0);
    add("read", OBJ_PROC, T_NONE, 0, 1, 0);

    while ((int)tab.size() < FIRST_USER) {
        add("__reserved__", OBJ_KEYWORD, T_NONE, 0, 1, 0);
    }

    btab[0].last = (int)tab.size() - 1;
}

int SymbolTable::enter(const std::string& id, int obj, int type,
                       int ref, int nrm, int adr) {
    std::string lower = toLower(id);
    int idx = (int)tab.size();
    int prevLast = btab[curBlock].last;
    tab.push_back(TabEntry(lower, prevLast, obj, type, ref, nrm, curLevel, adr));
    btab[curBlock].last = idx;
    return idx;
}

int SymbolTable::lookup(const std::string& id) const {
    std::string lower = toLower(id);

    for (int lv = curLevel; lv >= 0; lv--) {
        int bIdx = display[lv];
        int i = btab[bIdx].last;
        while (i > 0) {
            if (tab[i].id == lower) return i;
            i = tab[i].link;
        }
        if (i == 0 && (int)tab.size() > 0 && tab[0].id == lower) return 0;
    }

    for (int i = 0; i < FIRST_USER && i < (int)tab.size(); i++) {
        if (tab[i].id == lower) return i;
    }

    return -1;
}

int SymbolTable::openBlock() {
    int bIdx = (int)btab.size();
    btab.push_back(BtabEntry(0, 0, 0, 0));
    curLevel++;
    curBlock = bIdx;
    if ((int)display.size() <= curLevel)
        display.push_back(bIdx);
    else
        display[curLevel] = bIdx;
    return bIdx;
}

void SymbolTable::closeBlock() {
    if (curLevel > 0) {
        curLevel--;
        curBlock = display[curLevel];
    }
}

int SymbolTable::newArray(int xtyp, int etyp, int eref, int low, int high, int elsz) {
    int idx = (int)atab.size() + 1;
    atab.push_back(AtabEntry(xtyp, etyp, eref, low, high, elsz));
    return idx;
}

int SymbolTable::typeSize(int typeCode, int ref) const {
    switch (typeCode) {
        case T_INTEGER: return 1;
        case T_REAL:    return 1;
        case T_BOOLEAN: return 1;
        case T_CHAR:    return 1;
        case T_STRING:  return 1;
        case T_ARRAY:
            if (ref > 0 && ref <= (int)atab.size())
                return atab[ref-1].size;
            return 1;
        case T_RECORD:
            if (ref > 0 && ref <= (int)btab.size())
                return btab[ref-1].vsze;
            return 1;
        default: return 1;
    }
}

void SymbolTable::printTab(std::ostream& out) const {
    out << "\n--- Symbol Table (tab) ---\n";
    out << std::left
        << std::setw(5)  << "idx"
        << std::setw(20) << "identifier"
        << std::setw(12) << "obj"
        << std::setw(8)  << "type"
        << std::setw(6)  << "ref"
        << std::setw(6)  << "nrm"
        << std::setw(6)  << "lev"
        << std::setw(8)  << "adr"
        << std::setw(8)  << "link"
        << "\n";
    out << std::string(79, '-') << "\n";

    for (int i = FIRST_USER; i < (int)tab.size(); i++) {
        const auto& e = tab[i];
        out << std::left
            << std::setw(5)  << i
            << std::setw(20) << e.id
            << std::setw(12) << objName(e.obj)
            << std::setw(8)  << typeName(e.type)
            << std::setw(6)  << e.ref
            << std::setw(6)  << e.nrm
            << std::setw(6)  << e.lev
            << std::setw(8)  << e.adr
            << std::setw(8)  << e.link
            << "\n";
    }
}

void SymbolTable::printBtab(std::ostream& out) const {
    out << "\n--- Block Table (btab) ---\n";
    out << std::left
        << std::setw(6)  << "idx"
        << std::setw(8)  << "last"
        << std::setw(8)  << "lpar"
        << std::setw(8)  << "psze"
        << std::setw(8)  << "vsze"
        << "\n";
    out << std::string(40, '-') << "\n";
    for (int i = 0; i < (int)btab.size(); i++) {
        const auto& b = btab[i];
        out << std::left
            << std::setw(6) << i
            << std::setw(8) << b.last
            << std::setw(8) << b.lpar
            << std::setw(8) << b.psze
            << std::setw(8) << b.vsze
            << "\n";
    }
}

void SymbolTable::printAtab(std::ostream& out) const {
    out << "\n--- Array Table (atab) ---\n";
    if (atab.empty()) {
        out << "(kosong)\n";
        return;
    }
    out << std::left
        << std::setw(6)  << "idx"
        << std::setw(8)  << "xtyp"
        << std::setw(8)  << "etyp"
        << std::setw(8)  << "eref"
        << std::setw(8)  << "low"
        << std::setw(8)  << "high"
        << std::setw(8)  << "elsz"
        << std::setw(8)  << "size"
        << "\n";
    out << std::string(62, '-') << "\n";
    for (int i = 0; i < (int)atab.size(); i++) {
        const auto& a = atab[i];
        out << std::left
            << std::setw(6)  << (i+1)
            << std::setw(8)  << typeName(a.xtyp)
            << std::setw(8)  << typeName(a.etyp)
            << std::setw(8)  << a.eref
            << std::setw(8)  << a.low
            << std::setw(8)  << a.high
            << std::setw(8)  << a.elsz
            << std::setw(8)  << a.size
            << "\n";
    }
}
