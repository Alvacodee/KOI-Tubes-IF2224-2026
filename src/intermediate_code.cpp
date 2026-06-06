#include "intermediate_code.hpp"
#include <iomanip>
#include <sstream>

std::string Instruction::opName() const {
    switch (op) {
        case OP_INT: return "INT";
        case OP_LIT: return "LIT";
        case OP_LOD: return "LOD";
        case OP_STO: return "STO";
        case OP_OPR: return "OPR";
        case OP_JMP: return "JMP";
        case OP_JPC: return "JPC";
        case OP_CAL: return "CAL";
        case OP_RET: return "RET";
        default:     return "???";
    }
}

void Instruction::print(std::ostream& out, int index) const {
    out << index << " " << opName() << " " << lvl << " " << arg << "\n";
}

int IntermediateCodeGenerator::emit(OpcodeType op, int lvl, int arg) {
    int idx = (int)code.size();
    code.emplace_back(op, lvl, arg);
    return idx;
}

void IntermediateCodeGenerator::backpatch(int addr, int target) {
    code[addr].arg = target;
}

int IntermediateCodeGenerator::frameSize(int btabIdx) const {
    return 3 + symtab.btab[btabIdx].vsze;
}

int IntermediateCodeGenerator::varAddress(int tabIdx) const {
    return 3 + symtab.tab[tabIdx].adr;
}

void IntermediateCodeGenerator::print(std::ostream& out) const {
    for (int i = 0; i < (int)code.size(); i++) {
        code[i].print(out, i);
    }
}

void IntermediateCodeGenerator::printToFile(const std::string& path) const {
    std::ofstream f(path);
    if (f.is_open()) {
        print(f);
        f.close();
    }
}

void IntermediateCodeGenerator::generate(ASTNode* root) {
    if (!root) return;
    genProgram(root);
}

void IntermediateCodeGenerator::genProgram(ASTNode* node) {
    if (!node || node->children.empty()) return;
    ASTNode* blockNode = node->children[0];
    emit(OP_INT, 0, frameSize(blockNode->tabIndex));
    genBlock(blockNode);
    emit(OP_RET, 0, 0);
}

void IntermediateCodeGenerator::genBlock(ASTNode* node) {
    if (!node) return;
    currentLevel = node->level;
    int jmpIdx = emit(OP_JMP, 0, 0);
    
    for (auto* c : node->children) {
        if (c->kind == AST_DECL_PART) {
            genDeclPart(c);
        }
    }
    backpatch(jmpIdx, nextAddr());
    for (auto* c : node->children) {
        if (c->kind == AST_COMPOUND) {
            genCompound(c);
        }
    }
}

void IntermediateCodeGenerator::genDeclPart(ASTNode* node) {
    if (!node) return;
    for (auto* c : node->children) {
        if (c->kind == AST_PROC_DECL) genProcDecl(c);
        else if (c->kind == AST_FUNC_DECL) genFuncDecl(c);
    }
}

void IntermediateCodeGenerator::genProcDecl(ASTNode* node) {
    if (!node) return;
    procAddrs[node->tabIndex] = nextAddr();
    int btabIdx = symtab.tab[node->tabIndex].ref;
    emit(OP_INT, 0, frameSize(btabIdx));
    for (auto* c : node->children) {
        if (c->kind == AST_BLOCK) {
            genBlock(c);
        }
    }
    emit(OP_RET, 0, 0);
}

void IntermediateCodeGenerator::genFuncDecl(ASTNode* node) {
    if (!node) return;
    procAddrs[node->tabIndex] = nextAddr();
    int btabIdx = symtab.tab[node->tabIndex].ref;
    emit(OP_INT, 0, frameSize(btabIdx));
    for (auto* c : node->children) {
        if (c->kind == AST_BLOCK) {
            genBlock(c);
        }
    }
    emit(OP_RET, 0, 0);
}

void IntermediateCodeGenerator::genExpr(ASTNode* node) {
    if (!node) return;
    
    switch (node->kind) {
        case AST_INT_LIT:
            emit(OP_LIT, 0, std::stoi(node->value));
            break;
        case AST_BOOL_LIT:
            emit(OP_LIT, 0, (node->value == "true") ? 1 : 0);
            break;
        case AST_CHAR_LIT: {
            int ch = (node->value.size() >= 3) ? (int)(unsigned char)node->value[1] : 0;
            emit(OP_LIT, 0, ch);
            break;
        }
        case AST_REAL_LIT: {
            int val = (int)std::stod(node->value);
            emit(OP_LIT, 0, val);
            break;
        }
        case AST_STR_LIT:
            emit(OP_LIT, 0, 0);
            break;
        case AST_VAR: {
            int diff = currentLevel - node->level;
            int offs = varOffset(node->tabIndex);
            emit(OP_LOD, diff, offs);
            break;
        }
        case AST_BINOP: {
            genExpr(node->children[0]);
            genExpr(node->children[1]);
            int opr = mapOpToOpr(node->op);
            if (opr == 0) opr = OPR_ADD;  
            emit(OP_OPR, 0, opr);
            break;
        }
        case AST_UNOP: {
            if (!node->children.empty())
                genExpr(node->children[0]);
            if (node->op == "not")
                emit(OP_OPR, 0, OPR_NOT);
            else if (node->op == "-")
                emit(OP_OPR, 0, OPR_NEG);
            break;
        }
        case AST_ARRAY_ACCESS: {
            if (!node->children.empty() && node->children[0]->kind == AST_VAR) {
                ASTNode* varNode = node->children[0];
                int diff = currentLevel - varNode->level;
                int offs = varOffset(varNode->tabIndex);
                emit(OP_LOD, diff, offs);
            } else {
                emit(OP_LIT, 0, 0);
            }
            break;
        }
        case AST_PROC_CALL: {
            genProcCall(node);
            break;
        }
        default:
            emit(OP_LIT, 0, 0);
            break;
    }
}

void IntermediateCodeGenerator::genAssign(ASTNode* node) {
    if (!node || node->children.size() < 2) return;
    
    ASTNode* lhs = node->children[0];
    ASTNode* rhs = node->children[1];
    
    genExpr(rhs);
    if (lhs->kind == AST_VAR) {
        int diff = currentLevel - lhs->level;
        int offs = varOffset(lhs->tabIndex);
        emit(OP_STO, diff, offs);
    }
    else if (lhs->kind == AST_ARRAY_ACCESS) {
        // Array assignment: perlu perhitungan alamat, lewati sementara
        // (bisa diimplementasikan nanti jika diperlukan)
    }
}

void IntermediateCodeGenerator::genProcCall(ASTNode* node) {
    if (!node) return;
    std::string procName = node->value;
    
    if (procName == "write" || procName == "writeln") {
        for (auto* child : node->children) {
            genExpr(child);
        }

        int opr = (procName == "write") ? OPR_WRT : OPR_WRTLN;
        emit(OP_OPR, 0, opr);
    }
    else {
        int tabIdx = node->tabIndex;
        if (tabIdx >= 0) {
            int btabIdx = symtab.tab[tabIdx].ref; 
            int diff = currentLevel - symtab.tab[tabIdx].lev;
            
            for (auto* param : node->children) {
                genExpr(param);
            }
    
            auto it = procAddrs.find(tabIdx);
            int targetAddr = (it != procAddrs.end()) ? it->second : 0;
            emit(OP_CAL, diff, targetAddr);
        } else {
            // fallback: tidak dikenal
        }
    }
}

void IntermediateCodeGenerator::genVar(ASTNode* node) {

    (void)node;
}

void IntermediateCodeGenerator::genCompound(ASTNode* node) {
    if (!node) return;
    for (auto* child : node->children) {
        if (child->kind == AST_STMT_LIST) {
            genStmtList(child);
        }
    }
}

void IntermediateCodeGenerator::genStmtList(ASTNode* node) {
    if (!node) return;
    for (auto* stmt : node->children) {
        genStmt(stmt);
    }
}

void IntermediateCodeGenerator::genStmt(ASTNode* node) {
    if (!node) return;
    switch (node->kind) {
        case AST_ASSIGN:
            genAssign(node);
            break;
        case AST_PROC_CALL:
            genProcCall(node);
            break;
        case AST_COMPOUND:
            genCompound(node);
            break;
        case AST_IF:
            genIf(node);      
            break;
        case AST_WHILE:
            genWhile(node); 
            break;
        case AST_FOR:
            genFor(node);   
            break;
        case AST_REPEAT:
            genRepeat(node);  
            break;
        case AST_EMPTY:
            break;
        default:
            break;
    }
}

void IntermediateCodeGenerator::genIf(ASTNode* node) {

    (void)node;
}

void IntermediateCodeGenerator::genWhile(ASTNode* node) {

    (void)node;
}

void IntermediateCodeGenerator::genFor(ASTNode* node) {

    (void)node;
}

void IntermediateCodeGenerator::genRepeat(ASTNode* node) {

    (void)node;
}

//Fungi helper
// ========== Helper untuk mapping operator ke kode OPR ==========
int IntermediateCodeGenerator::mapOpToOpr(const std::string& op) const {
    if (op == "+")      return OPR_ADD;
    if (op == "-")      return OPR_SUB;
    if (op == "*")      return OPR_MUL;
    if (op == "/")      return OPR_DIV;
    if (op == "div")    return OPR_DIV;
    if (op == "mod")    return OPR_MOD;
    if (op == "and")    return OPR_AND;
    if (op == "or")     return OPR_OR;
    if (op == "not")    return OPR_NOT;
    if (op == "=")      return OPR_EQL;
    if (op == "<>")     return OPR_NEQ;
    if (op == "<")      return OPR_LSS;
    if (op == "<=")     return OPR_LEQ;
    if (op == ">")      return OPR_GTR;
    if (op == ">=")     return OPR_GEQ;
    return 0;  // fallback
}

int IntermediateCodeGenerator::varOffset(int tabIdx) const {
    if (tabIdx < 0 || tabIdx >= (int)symtab.tab.size()) return 0;
    return symtab.tab[tabIdx].adr;
}
