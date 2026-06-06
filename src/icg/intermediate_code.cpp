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
        case OP_LDA: return "LDA";
        case OP_LDI: return "LDI";
        case OP_STI: return "STI";
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
    int savedLevel = currentLevel;
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
    currentLevel = savedLevel;
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
    
    int psze = symtab.btab[btabIdx].psze;
    for (int i = 0; i < psze; i++) {
        emit(OP_LOD, 0, -psze + i);
        emit(OP_STO, 0, 3 + i);
    }
    
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
    
    int psze = symtab.btab[btabIdx].psze;
    for (int i = 0; i < psze; i++) {
        emit(OP_LOD, 0, -psze + i);
        emit(OP_STO, 0, 3 + i);
    }
    
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
            if (node->children.size() >= 1) {
                int diff = currentLevel - node->level;
                int offs = varOffset(node->tabIndex);

                int atabRef = symtab.tab[node->tabIndex].ref;
                int arrRef = atabRef - 1;
                if (arrRef < 0) arrRef = 0; // fallback
                int low = 0;
                int elsz = 1;
                if (arrRef < (int)symtab.atab.size()) {
                    low = symtab.atab[arrRef].low;
                    elsz = symtab.atab[arrRef].elsz;
                }

                emit(OP_LDA, diff, offs);

                genExpr(node->children[0]);

                emit(OP_LIT, 0, low);
                emit(OP_OPR, 0, OPR_SUB);
                emit(OP_LIT, 0, elsz);
                emit(OP_OPR, 0, OPR_MUL);
                emit(OP_OPR, 0, OPR_ADD);

                emit(OP_LDI, 0, 0);
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

    if (lhs->kind == AST_VAR) {
        int tabIdx = lhs->tabIndex;
        if (tabIdx >= 0 && symtab.tab[tabIdx].obj == OBJ_FUNC) {
            genExpr(rhs);
            int btabIdx = symtab.tab[tabIdx].ref;
            int psze = symtab.btab[btabIdx].psze;
            int diff = currentLevel - (symtab.tab[tabIdx].lev + 1);
            int offs = -psze - 1;
            emit(OP_STO, diff, offs);
        } else {
            genExpr(rhs);
            int diff = currentLevel - lhs->level;
            int offs = varOffset(lhs->tabIndex);
            emit(OP_STO, diff, offs);
        }
    }
    else if (lhs->kind == AST_ARRAY_ACCESS) {
        if (lhs->children.size() >= 1) {
            int diff = currentLevel - lhs->level;
            int offs = varOffset(lhs->tabIndex);

            int atabRef = symtab.tab[lhs->tabIndex].ref;
            int arrRef = atabRef - 1;
            if (arrRef < 0) arrRef = 0; // fallback
            int low = 0;
            int elsz = 1;
            if (arrRef < (int)symtab.atab.size()) {
                low = symtab.atab[arrRef].low;
                elsz = symtab.atab[arrRef].elsz;
            }

            emit(OP_LDA, diff, offs);

            genExpr(lhs->children[0]);

            emit(OP_LIT, 0, low);
            emit(OP_OPR, 0, OPR_SUB);
            emit(OP_LIT, 0, elsz);
            emit(OP_OPR, 0, OPR_MUL);
            emit(OP_OPR, 0, OPR_ADD);

            genExpr(rhs);

            emit(OP_STI, 0, 0);
        }
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
            int diff = currentLevel - symtab.tab[tabIdx].lev;
            int btabIdx = symtab.tab[tabIdx].ref;
            int psze = symtab.btab[btabIdx].psze;
            bool isFunc = (symtab.tab[tabIdx].obj == OBJ_FUNC);
            
            if (isFunc) {
                emit(OP_INT, 0, 1);
            }
            
            for (auto* param : node->children) {
                genExpr(param);
            }
    
            auto it = procAddrs.find(tabIdx);
            int targetAddr = (it != procAddrs.end()) ? it->second : 0;
            emit(OP_CAL, diff, targetAddr);
            
            if (psze > 0) {
                emit(OP_INT, 0, -psze);
            }
        } else {

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
        case AST_CASE:
            genCase(node);
            break;
        case AST_EMPTY:
            break;
        default:
            break;
    }
}

void IntermediateCodeGenerator::genIf(ASTNode* node) {
    if (!node || node->children.empty()) return;
    genExpr(node->children[0]);
    int jpcIdx = emit(OP_JPC, 0, 0);
    
    if (node->children.size() > 1) {
        genStmt(node->children[1]);
    }
    
    if (node->children.size() > 2) {
        int jmpIdx = emit(OP_JMP, 0, 0);
        backpatch(jpcIdx, nextAddr());
        genStmt(node->children[2]);
        backpatch(jmpIdx, nextAddr());
    } else {
        backpatch(jpcIdx, nextAddr());
    }
}

void IntermediateCodeGenerator::genWhile(ASTNode* node) {
    if (!node || node->children.empty()) return;
    int startAddr = nextAddr();
    genExpr(node->children[0]);
    int jpcIdx = emit(OP_JPC, 0, 0);
    
    if (node->children.size() > 1) {
        genStmt(node->children[1]);
    }
    emit(OP_JMP, 0, startAddr);
    backpatch(jpcIdx, nextAddr());
}

void IntermediateCodeGenerator::genFor(ASTNode* node) {
    if (!node || node->children.size() < 4) return;
    
    ASTNode* counterVar = node->children[0];
    ASTNode* initExpr = node->children[1];
    ASTNode* finalExpr = node->children[2];
    ASTNode* body = node->children[3];
    bool isDownTo = (node->op == "downto");

    genExpr(initExpr);
    int diff = currentLevel - counterVar->level;
    int offs = varOffset(counterVar->tabIndex);
    emit(OP_STO, diff, offs);
    
    int startAddr = nextAddr();
    
    emit(OP_LOD, diff, offs);
    genExpr(finalExpr);
    if (isDownTo) {
        emit(OP_OPR, 0, OPR_GEQ);
    } else {
        emit(OP_OPR, 0, OPR_LEQ);
    }
    
    int jpcIdx = emit(OP_JPC, 0, 0);
    
    genStmt(body);
    
    emit(OP_LOD, diff, offs);
    emit(OP_LIT, 0, 1);
    if (isDownTo) {
        emit(OP_OPR, 0, OPR_SUB);
    } else {
        emit(OP_OPR, 0, OPR_ADD);
    }
    emit(OP_STO, diff, offs);
    
    emit(OP_JMP, 0, startAddr);
    backpatch(jpcIdx, nextAddr());
}

void IntermediateCodeGenerator::genRepeat(ASTNode* node) {
    if (!node || node->children.size() < 2) return;
    int startAddr = nextAddr();
    genStmt(node->children[0]);
    genExpr(node->children[1]);
    emit(OP_JPC, 0, startAddr);
}

void IntermediateCodeGenerator::genCase(ASTNode* node) {
    if (!node || node->children.size() < 2) return;
    ASTNode* exprNode = node->children[0];
    ASTNode* cbNode = node->children[1];
    
    std::vector<int> endJmps;
    std::vector<ASTNode*> currentConsts;
    
    for (auto* child : cbNode->children) {
        if (child->kind == AST_INT_LIT || child->kind == AST_CHAR_LIT) {
            currentConsts.push_back(child);
        } else {
            std::vector<int> stmtJmps;
            int nextCheckIdx = -1;
            
            for (size_t i = 0; i < currentConsts.size(); i++) {
                if (nextCheckIdx != -1) {
                    backpatch(nextCheckIdx, nextAddr());
                }
                genExpr(exprNode);
                genExpr(currentConsts[i]);
                emit(OP_OPR, 0, OPR_EQL);
                
                if (i == currentConsts.size() - 1) {
                    nextCheckIdx = emit(OP_JPC, 0, 0);
                } else {
                    int jpcToNextCheck = emit(OP_JPC, 0, 0);
                    stmtJmps.push_back(emit(OP_JMP, 0, 0));
                    backpatch(jpcToNextCheck, nextAddr());
                }
            }
            
            for (int jmp : stmtJmps) {
                backpatch(jmp, nextAddr());
            }
            
            genStmt(child);
            endJmps.push_back(emit(OP_JMP, 0, 0));
            
            if (nextCheckIdx != -1) {
                backpatch(nextCheckIdx, nextAddr());
            }
            
            currentConsts.clear();
        }
    }
    
    for (int jmp : endJmps) {
        backpatch(jmp, nextAddr());
    }
}

int IntermediateCodeGenerator::mapOpToOpr(const std::string& op) const {
    if (op == "plus" || op == "+")      return OPR_ADD;
    if (op == "minus" || op == "-")     return OPR_SUB;
    if (op == "times" || op == "*")     return OPR_MUL;
    if (op == "rdiv" || op == "/")      return OPR_DIV;
    if (op == "idiv" || op == "div")    return OPR_DIV;
    if (op == "imod" || op == "mod")    return OPR_MOD;
    if (op == "andsy" || op == "and")   return OPR_AND;
    if (op == "orsy" || op == "or")     return OPR_OR;
    if (op == "notsy" || op == "not")   return OPR_NOT;
    if (op == "eql" || op == "=")       return OPR_EQL;
    if (op == "neq" || op == "<>")      return OPR_NEQ;
    if (op == "lss" || op == "<")       return OPR_LSS;
    if (op == "leq" || op == "<=")      return OPR_LEQ;
    if (op == "gtr" || op == ">")       return OPR_GTR;
    if (op == "geq" || op == ">=")      return OPR_GEQ;
    return 0;
}

int IntermediateCodeGenerator::varOffset(int tabIdx) const {
    if (tabIdx < 0 || tabIdx >= (int)symtab.tab.size()) return 0;
    return 3 + symtab.tab[tabIdx].adr;
}
