#include "ast_node.hpp"
#include <fstream>

static const char* kindStr(ASTKind k) {
    switch (k) {
        case AST_PROGRAM:     return "ProgramNode";
        case AST_BLOCK:       return "Block";
        case AST_DECL_PART:   return "Declarations";
        case AST_CONST_DECL:  return "ConstDecl";
        case AST_TYPE_DECL:   return "TypeDecl";
        case AST_VAR_DECL:    return "VarDecl";
        case AST_PROC_DECL:   return "ProcDecl";
        case AST_FUNC_DECL:   return "FuncDecl";
        case AST_PARAM_LIST:  return "ParamList";
        case AST_PARAM_GROUP: return "ParamGroup";
        case AST_COMPOUND:    return "CompoundStatement";
        case AST_STMT_LIST:   return "StatementList";
        case AST_ASSIGN:      return "Assign";
        case AST_IF:          return "If";
        case AST_WHILE:       return "While";
        case AST_FOR:         return "For";
        case AST_REPEAT:      return "Repeat";
        case AST_CASE:        return "Case";
        case AST_CASE_BLOCK:  return "CaseBlock";
        case AST_PROC_CALL:   return "ProcCall";
        case AST_BINOP:       return "BinOp";
        case AST_UNOP:        return "UnOp";
        case AST_VAR:         return "Var";
        case AST_ARRAY_ACCESS:return "ArrayAccess";
        case AST_FIELD_ACCESS:return "FieldAccess";
        case AST_INT_LIT:     return "IntLit";
        case AST_REAL_LIT:    return "RealLit";
        case AST_CHAR_LIT:    return "CharLit";
        case AST_STR_LIT:     return "StrLit";
        case AST_BOOL_LIT:    return "BoolLit";
        case AST_EMPTY:       return "Empty";
        default:              return "Node";
    }
}

static void printAnnotation(std::ostream& out, const ASTNode& n) {
    bool hasAnnot = (n.typeCode != T_NONE || n.tabIndex >= 0 || n.level > 0);
    if (!hasAnnot) return;

    out << " \u2192 ";
    bool first = true;

    if (n.tabIndex >= 0) {
        if (!first) out << ", ";
        out << "tab[" << n.tabIndex << "]";
        first = false;
    }
    if (n.typeCode != T_NONE) {
        if (!first) out << ", ";
        out << "type:" << typeName(n.typeCode);
        first = false;
    }
    if (n.level > 0) {
        if (!first) out << ", ";
        out << "lev:" << n.level;
        first = false;
    }
}

void ASTNode::print(std::ostream& out,
                    const std::string& prefix,
                    bool isLast) const {

    out << prefix;
    out << (isLast ? "\u2514\u2500\u2500 " : "\u251C\u2500\u2500 ");

    out << kindStr(kind);
    if (!value.empty()) out << "('" << value << "')";
    if (!op.empty())    out << " op=" << op;

    printAnnotation(out, *this);
    out << "\n";

    std::string childPrefix = prefix + (isLast ? "    " : "\u2502   ");
    for (int i = 0; i < (int)children.size(); i++) {
        if (children[i])
            children[i]->print(out, childPrefix, i == (int)children.size()-1);
    }
}

void ASTNode::printToFile(const std::string& path) const {
    std::ofstream f(path);
    if (!f.is_open()) return;

    f << kindStr(kind);
    if (!value.empty()) f << "('" << value << "')";
    if (!op.empty())    f << " op=" << op;
    printAnnotation(f, *this);
    f << "\n";

    std::string childPrefix = "";
    for (int i = 0; i < (int)children.size(); i++) {
        if (children[i])
            children[i]->print(f, childPrefix, i == (int)children.size()-1);
    }
}
