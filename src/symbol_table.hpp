#pragma once
#include "ast_node.hpp"
#include <string>
#include <vector>
#include <algorithm>
#include <iostream>

// =============================================
// Entry tabel identifier (tab)
// =============================================
struct TabEntry {
    std::string id;  // nama identifier (lowercase untuk case-insensitive)
    int link;        // indeks ke identifier sebelumnya dalam scope
    int obj;         // kelas objek (OBJ_*)
    int type;        // kode tipe (T_*)
    int ref;         // ref ke atab/btab untuk tipe komposit
    int nrm;         // 1=normal variable, 0=var parameter
    int lev;         // lexical level
    int adr;         // alamat / offset / nilai konstanta

    TabEntry(const std::string& id_, int link_, int obj_, int type_,
             int ref_, int nrm_, int lev_, int adr_)
        : id(id_), link(link_), obj(obj_), type(type_),
          ref(ref_), nrm(nrm_), lev(lev_), adr(adr_) {}
};

// =============================================
// Entry tabel blok (btab)
// =============================================
struct BtabEntry {
    int last;   // indeks terakhir di tab untuk blok ini
    int lpar;   // indeks parameter terakhir
    int psze;   // ukuran parameter block
    int vsze;   // ukuran variabel lokal

    BtabEntry() : last(0), lpar(0), psze(0), vsze(0) {}
    BtabEntry(int last_, int lpar_, int psze_, int vsze_)
        : last(last_), lpar(lpar_), psze(psze_), vsze(vsze_) {}
};

// =============================================
// Entry tabel array (atab)
// =============================================
struct AtabEntry {
    int xtyp;  // tipe indeks
    int etyp;  // tipe elemen
    int eref;  // ref ke atab/btab jika elemen komposit
    int low;   // batas bawah
    int high;  // batas atas
    int elsz;  // ukuran satu elemen
    int size;  // total ukuran

    AtabEntry(int xtyp_, int etyp_, int eref_, int low_, int high_, int elsz_)
        : xtyp(xtyp_), etyp(etyp_), eref(eref_),
          low(low_), high(high_), elsz(elsz_) {
        size = (high_ - low_ + 1) * elsz_;
    }
};

// =============================================
// Symbol Table
// =============================================
class SymbolTable {
public:
    std::vector<TabEntry>  tab;
    std::vector<BtabEntry> btab;
    std::vector<AtabEntry> atab;

    int curLevel;   // lexical level sekarang
    int curBlock;   // indeks btab dari blok aktif
    std::vector<int> display; // display[level] = indeks btab

    // Indeks pertama entry user (setelah semua predefined)
    static const int FIRST_USER = 34;

    SymbolTable();

    // Inisialisasi reserved words & predefined identifiers
    void initPredefined();

    // Masukkan identifier baru, kembalikan indeksnya
    int enter(const std::string& id, int obj, int type,
              int ref, int nrm, int adr);

    // Lookup dari scope terdalam ke terluar (case-insensitive)
    // Return indeks di tab, -1 jika tidak ditemukan
    int lookup(const std::string& id) const;

    // Buka blok baru (prosedur/fungsi/record), kembalikan indeks btab
    int openBlock();

    // Tutup blok aktif
    void closeBlock();

    // Tambah entry array, kembalikan indeks atab
    int newArray(int xtyp, int etyp, int eref, int low, int high, int elsz);

    // Ukuran tipe dalam unit memori
    int typeSize(int typeCode, int ref) const;

    // Print tables
    void printTab(std::ostream& out) const;
    void printBtab(std::ostream& out) const;
    void printAtab(std::ostream& out) const;

private:
    std::string toLower(const std::string& s) const;
};
