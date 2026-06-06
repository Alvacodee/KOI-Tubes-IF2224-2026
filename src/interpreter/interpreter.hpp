#pragma once
#include "intermediate_code.hpp"
#include <vector>
#include <string>
#include <iostream>
#include <stdexcept>
#include <climits>

using StackVal = long long;

struct RuntimeError : public std::runtime_error {
    explicit RuntimeError(const std::string& msg)
        : std::runtime_error("Runtime Error: " + msg) {}
};

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

class Interpreter {
public:

    static const int MAX_STACK_DEPTH = 1000;

    static const int MAX_STACK_SIZE  = 100000;

    static const long long INT_MAX_VAL =  2147483647LL;
    static const long long INT_MIN_VAL = -2147483648LL;

    explicit Interpreter(const std::vector<Instruction>& instructions);

    void run();

    void enableVulnDetection(bool enable) { vulnDetect = enable; }

    std::ostream* output = &std::cout;

private:

    const std::vector<Instruction>& prog;

    std::vector<StackVal> stack;
    int sp = -1;
    int ip =  0;
    int bp =  0;

    bool vulnDetect = true;

    const Instruction& fetch();

    bool execute(const Instruction& instr);

    void push(StackVal val);

    StackVal pop();

    StackVal load(int addr) const;

    void store(int addr, StackVal val);

    int find_base(int level) const;

    void execINT(int m);
    void execLIT(int v);
    void execLOD(int level, int a);
    void execSTO(int level, int a);
    void execLDA(int level, int a);
    void execLDI();
    void execSTI();
    void execOPR(int o);
    void execJMP(int l);
    void execJPC(int l);
    void execCAL(int level, int l);
    bool execRET();

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

    void throwVuln(VulnType type, const std::string& detail = "") const;

    void checkAddr(int addr) const;

    void checkIP(int target) const;

    void checkNumRange(long long result) const;
};
