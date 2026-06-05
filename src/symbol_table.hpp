#pragma once
#include "ast_node.hpp"
#include <string>
#include <vector>
#include <algorithm>
#include <iostream>

struct TabEntry {
    std::string id;
    int link;
    int obj;
    int type;
    int ref;
    int nrm;
    int lev;
    int adr;

    TabEntry(const std::string& id_, int link_, int obj_, int type_,
             int ref_, int nrm_, int lev_, int adr_)
        : id(id_), link(link_), obj(obj_), type(type_),
          ref(ref_), nrm(nrm_), lev(lev_), adr(adr_) {}
};

struct BtabEntry {
    int last;
    int lpar;
    int psze;
    int vsze;

    BtabEntry() : last(0), lpar(0), psze(0), vsze(0) {}
    BtabEntry(int last_, int lpar_, int psze_, int vsze_)
        : last(last_), lpar(lpar_), psze(psze_), vsze(vsze_) {}
};

struct AtabEntry {
    int xtyp;
    int etyp;
    int eref;
    int low;
    int high;
    int elsz;
    int size;

    AtabEntry(int xtyp_, int etyp_, int eref_, int low_, int high_, int elsz_)
        : xtyp(xtyp_), etyp(etyp_), eref(eref_),
          low(low_), high(high_), elsz(elsz_) {
        size = (high_ - low_ + 1) * elsz_;
    }
};

class SymbolTable {
public:
    std::vector<TabEntry>  tab;
    std::vector<BtabEntry> btab;
    std::vector<AtabEntry> atab;

    int curLevel;
    int curBlock;
    std::vector<int> display;

    static const int FIRST_USER = 38;

    SymbolTable();

    void initPredefined();

    int enter(const std::string& id, int obj, int type,
              int ref, int nrm, int adr);

    int lookup(const std::string& id) const;

    int openBlock();

    void closeBlock();

    int newArray(int xtyp, int etyp, int eref, int low, int high, int elsz);

    int typeSize(int typeCode, int ref) const;

    void printTab(std::ostream& out) const;
    void printBtab(std::ostream& out) const;
    void printAtab(std::ostream& out) const;

private:
    std::string toLower(const std::string& s) const;
};
