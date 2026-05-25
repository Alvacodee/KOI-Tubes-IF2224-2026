#include "interpreter.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>

// Constructor
Interpreter::Interpreter(const std::vector<Instruction>& instructions)
    : prog(instructions)
{
    stack.resize(MAX_STACK_SIZE, 0);
}

// Entry point eksekusi
void Interpreter::run() {
    pc = 0;
    sp = -1;
    frameDepth = 0;
    bpStack.clear();
    pendingRetAddr = 0;
    pendingDynLink = -1;

    if (vulnDetect) {
        for (int i = 0; i < (int)prog.size(); i++) {
            const auto& instr = prog[i];
            if (instr.op == OP_JMP || instr.op == OP_JPC || instr.op == OP_CAL) {
                if (instr.arg < 0 || instr.arg >= (int)prog.size()) {
                    throwVuln(VULN_INVALID_JUMP,
                        "instruksi di baris " + std::to_string(i) +
                        " melompat ke baris " + std::to_string(instr.arg) +
                        " yang tidak ada");
                }
            }
        }
    }

    while (pc >= 0 && pc < (int)prog.size()) {
        const Instruction& instr = fetch();
        bool cont = execute(instr);
        if (!cont) break;
    }
}

// Fetch
const Instruction& Interpreter::fetch() {
    if (vulnDetect) checkPC(pc);
    return prog[pc++];
}

// Execute
bool Interpreter::execute(const Instruction& instr) {
    switch (instr.op) {
        case OP_INT: execINT(instr.arg); return true;
        case OP_LIT: execLIT(instr.arg); return true;
        case OP_LOD: execLOD(instr.arg); return true;
        case OP_STO: execSTO(instr.arg); return true;
        case OP_OPR: execOPR(instr.arg); return true;
        case OP_JMP: execJMP(instr.arg); return true;
        case OP_JPC: execJPC(instr.arg); return true;
        case OP_CAL: execCAL(instr.arg); return true;
        case OP_RET: return !execRET();
        default:
            throw RuntimeError("opcode tidak dikenal: " + std::to_string(instr.op));
    }
}

// Operasi stack dasar
void Interpreter::push(StackVal val) {
    sp++;
    if (vulnDetect && sp >= MAX_STACK_SIZE) {
        throwVuln(VULN_STACK_OVERFLOW,
            "stack penuh (sp=" + std::to_string(sp) + ")");
    }
    stack[sp] = val;
}

StackVal Interpreter::pop() {
    if (vulnDetect && sp < 0) {
        throwVuln(VULN_STACK_UNDERFLOW, "pop dari stack kosong");
    }
    if (sp < 0) throw RuntimeError("stack underflow");
    return stack[sp--];
}

StackVal Interpreter::load(int addr) const {
    if (vulnDetect) checkAddr(addr);
    return stack[addr];
}

void Interpreter::store(int addr, StackVal val) {
    if (vulnDetect) checkAddr(addr);
    stack[addr] = val;
}


void Interpreter::execINT(int m) {
    frameDepth++;
    if (vulnDetect && frameDepth > MAX_STACK_DEPTH) {
        throwVuln(VULN_STACK_OVERFLOW,
            "kedalaman frame melebihi " + std::to_string(MAX_STACK_DEPTH));
    }

    int base = sp + 1;  // indeks slot pertama frame baru

    if (vulnDetect && base + m - 1 >= MAX_STACK_SIZE) {
        throwVuln(VULN_STACK_OVERFLOW, "kehabisan ruang stack");
    }

    bpStack.push_back(base);

    for (int i = 0; i < m; i++) {
        stack[base + i] = 0;
    }
    sp = base + m - 1;

    if (bpStack.size() > 1) {
        stack[base + 0] = 0;
        stack[base + 1] = pendingDynLink;
        stack[base + 2] = pendingRetAddr;
    }
}

void Interpreter::execLIT(int v) {
    push(static_cast<StackVal>(v));
}

void Interpreter::execLOD(int a) {
    push(load(a));
}

void Interpreter::execSTO(int a) {
    StackVal val = pop();
    store(a, val);
}

void Interpreter::execOPR(int o) {
    switch (static_cast<OprType>(o)) {
        case OPR_NEG:   oprNEG();   break;
        case OPR_ADD:   oprADD();   break;
        case OPR_SUB:   oprSUB();   break;
        case OPR_MUL:   oprMUL();   break;
        case OPR_DIV:   oprDIV();   break;
        case OPR_MOD:   oprMOD();   break;
        case OPR_EQL:   oprEQL();   break;
        case OPR_NEQ:   oprNEQ();   break;
        case OPR_LSS:   oprLSS();   break;
        case OPR_GEQ:   oprGEQ();   break;
        case OPR_GTR:   oprGTR();   break;
        case OPR_LEQ:   oprLEQ();   break;
        case OPR_WRT:   oprWRT();   break;
        case OPR_WRTLN: oprWRTLN(); break;
        case OPR_NOT:   oprNOT();   break;
        case OPR_AND:   oprAND();   break;
        case OPR_OR:    oprOR();    break;
        default:
            throw RuntimeError("operasi OPR tidak dikenal: " + std::to_string(o));
    }
}

void Interpreter::execJMP(int l) {
    if (vulnDetect) checkPC(l);
    pc = l;
}

void Interpreter::execJPC(int l) {
    StackVal cond = pop();
    if (cond == 0) {
        if (vulnDetect) checkPC(l);
        pc = l;
    }
}

void Interpreter::execCAL(int l) {
    if (vulnDetect) checkPC(l);

    pendingRetAddr = pc;
    pendingDynLink = sp;

    pc = l;
}

bool Interpreter::execRET() {
    frameDepth--;

    if (bpStack.empty()) {
        sp = -1;
        return true;
    }

    int base = bpStack.back();
    bpStack.pop_back();

    if (bpStack.empty()) {
        sp = -1;
        return true;
    }

    StackVal dynLink = stack[base + 1];
    StackVal retAddr = stack[base + 2];

    sp = (int)dynLink;
    pc = (int)retAddr;

    return false;
}

// Operasi OPR — aritmatika & logika

void Interpreter::oprNEG() {
    StackVal a = pop();
    push(-a);
}

void Interpreter::oprADD() {
    StackVal b = pop();
    StackVal a = pop();
    long long result = (long long)a + (long long)b;
    if (vulnDetect) checkNumRange(result);
    push(result);
}

void Interpreter::oprSUB() {
    StackVal b = pop();
    StackVal a = pop();
    long long result = (long long)a - (long long)b;
    if (vulnDetect) checkNumRange(result);
    push(result);
}

void Interpreter::oprMUL() {
    StackVal b = pop();
    StackVal a = pop();
    long long result = (long long)a * (long long)b;
    if (vulnDetect) checkNumRange(result);
    push(result);
}

void Interpreter::oprDIV() {
    StackVal b = pop();
    StackVal a = pop();
    if (b == 0) throw RuntimeError("pembagian dengan nol");
    push(a / b);
}

void Interpreter::oprMOD() {
    StackVal b = pop();
    StackVal a = pop();
    if (b == 0) throw RuntimeError("modulus dengan nol");
    push(a % b);
}

void Interpreter::oprEQL() {
    StackVal b = pop();
    StackVal a = pop();
    push(a == b ? 1 : 0);
}

void Interpreter::oprNEQ() {
    StackVal b = pop();
    StackVal a = pop();
    push(a != b ? 1 : 0);
}

void Interpreter::oprLSS() {
    StackVal b = pop();
    StackVal a = pop();
    push(a < b ? 1 : 0);
}

void Interpreter::oprGEQ() {
    StackVal b = pop();
    StackVal a = pop();
    push(a >= b ? 1 : 0);
}

void Interpreter::oprGTR() {
    StackVal b = pop();
    StackVal a = pop();
    push(a > b ? 1 : 0);
}

void Interpreter::oprLEQ() {
    StackVal b = pop();
    StackVal a = pop();
    push(a <= b ? 1 : 0);
}

void Interpreter::oprWRT() {
    StackVal val = pop();
    *output << val;
}

void Interpreter::oprWRTLN() {
    StackVal val = pop();
    *output << val << "\n";
}

void Interpreter::oprNOT() {
    StackVal a = pop();
    push(a == 0 ? 1 : 0);
}

void Interpreter::oprAND() {
    StackVal b = pop();
    StackVal a = pop();
    push((a != 0 && b != 0) ? 1 : 0);
}

void Interpreter::oprOR() {
    StackVal b = pop();
    StackVal a = pop();
    push((a != 0 || b != 0) ? 1 : 0);
}

// Helper validasi (bonus vulnerability detection)

void Interpreter::throwVuln(VulnType type, const std::string& detail) const {
    std::string msg;
    switch (type) {
        case VULN_STACK_OVERFLOW:   msg = "Stack Overflow";          break;
        case VULN_STACK_UNDERFLOW:  msg = "Stack Underflow";         break;
        case VULN_STACK_SMASHING:   msg = "Stack Smashing";          break;
        case VULN_STACK_CORRUPTION: msg = "Stack Corruption";        break;
        case VULN_OUT_OF_BOUNDS:    msg = "Out-of-Bounds Access";    break;
        case VULN_INVALID_JUMP:     msg = "Invalid Jump Target";     break;
        case VULN_NUM_OVERFLOW:     msg = "Numerical Overflow";      break;
        case VULN_NUM_UNDERFLOW:    msg = "Numerical Underflow";     break;
    }
    if (!detail.empty()) msg += ": " + detail;
    throw RuntimeError("[VULNERABILITY] " + msg);
}

void Interpreter::checkAddr(int addr) const {
    if (addr < 0 || addr > sp) {
        throwVuln(VULN_OUT_OF_BOUNDS,
            "alamat " + std::to_string(addr) +
            " di luar jangkauan stack (sp=" + std::to_string(sp) + ")");
    }
}

void Interpreter::checkPC(int target) const {
    if (target < 0 || target >= (int)prog.size()) {
        throwVuln(VULN_INVALID_JUMP,
            "baris " + std::to_string(target) + " tidak ada (total instruksi: " +
            std::to_string(prog.size()) + ")");
    }
}

void Interpreter::checkNumRange(long long result) const {
    if (result > INT_MAX_VAL) {
        throwVuln(VULN_NUM_OVERFLOW,
            "hasil = " + std::to_string(result) +
            " melebihi INT_MAX (" + std::to_string(INT_MAX_VAL) + ")");
    }
    if (result < INT_MIN_VAL) {
        throwVuln(VULN_NUM_UNDERFLOW,
            "hasil = " + std::to_string(result) +
            " kurang dari INT_MIN (" + std::to_string(INT_MIN_VAL) + ")");
    }
}
