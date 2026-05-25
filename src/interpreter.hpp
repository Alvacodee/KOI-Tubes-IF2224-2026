#pragma once
#include "intermediate_code.hpp"
#include <vector>
#include <string>
#include <iostream>
#include <stdexcept>
#include <climits>

// Runtime value — stack machine menyimpan nilai sebagai int
// (integer, boolean, char semuanya integer; real sebagai bits)
using StackVal = long long;

// Error runtime interpreter
struct RuntimeError : public std::runtime_error {
    explicit RuntimeError(const std::string& msg)
        : std::runtime_error("Runtime Error: " + msg) {}
};

// enis vulnerability yang bisa dideteksi (BONUS)
enum VulnType {
    VULN_STACK_OVERFLOW,
    VULN_STACK_UNDERFLOW,
    VULN_STACK_SMASHING,
    VULN_STACK_CORRUPTION,
    VULN_OUT_OF_BOUNDS,
    VULN_INVALID_JUMP,
    VULN_NUM_OVERFLOW,
    VULN_NUM_UNDERFLOW,
};

// Interpreter (Stack Machine)
// Membaca daftar Instruction dan mengeksekusinya
class Interpreter {
public:
    // Batas maksimal kedalaman stack frame (untuk deteksi overflow)
    static const int MAX_STACK_DEPTH = 1000;

    // Batas memori total stack (jumlah slot StackVal)
    static const int MAX_STACK_SIZE  = 100000;

    // Batas nilai integer (untuk deteksi numerical overflow)
    static const long long INT_MAX_VAL =  2147483647LL;  // 2^31 - 1
    static const long long INT_MIN_VAL = -2147483648LL;

    // Constructor: menerima daftar instruksi TAC
    explicit Interpreter(const std::vector<Instruction>& instructions);

    // Jalankan program dari baris 0
    void run();

    // Aktifkan/nonaktifkan deteksi vulnerability (bonus)
    void enableVulnDetection(bool enable) { vulnDetect = enable; }

    // Output stream (default: cout)
    std::ostream* output = &std::cout;

private:
    // Data instruksi
    const std::vector<Instruction>& prog;

    // Memori stack
    std::vector<StackVal> stack;
    int sp = -1;
    int pc =  0;
    int frameDepth = 0;


    std::vector<int> bpStack;

    int pendingRetAddr = 0;
    int pendingDynLink = -1;

    bool vulnDetect = true;

    // Siklus eksekusi

    const Instruction& fetch();

    bool execute(const Instruction& instr);

    // Operasi stack

    void push(StackVal val);

    StackVal pop();

    StackVal load(int addr) const;

    void store(int addr, StackVal val);

    // Handler per instruksi

    void execINT(int m);
    void execLIT(int v);
    void execLOD(int a);
    void execSTO(int a);
    void execOPR(int o);
    void execJMP(int l);
    void execJPC(int l);
    void execCAL(int l);
    bool execRET();

    // Handler operasi OPR

    void oprNEG();
    void oprADD();
    void oprSUB();
    void oprMUL();
    void oprDIV();
    void oprMOD();
    void oprEQL();
    void oprNEQ();
    void oprLSS();
    void oprGEQ();
    void oprGTR();
    void oprLEQ();
    void oprWRT();
    void oprWRTLN();
    void oprNOT();
    void oprAND();
    void oprOR();

    // Helper validasi (bonus)

    // RuntimeError dengan pesan vulnerability
    void throwVuln(VulnType type, const std::string& detail = "") const;

    // Cek apakah addr valid di dalam stack
    void checkAddr(int addr) const;

    // Cek apakah pc valid
    void checkPC(int target) const;

    // Cek overflow/underflow untuk hasil operasi aritmatika
    void checkNumRange(long long result) const;
};
