#include "token.hpp"

std::string tokenTypeName(TokenType type) {
    switch (type) {
        case TokenType::INTCON:      return "intcon";
        case TokenType::REALCON:     return "realcon";
        case TokenType::CHARCON:     return "charcon";
        case TokenType::STRING_TOK:  return "string";

        case TokenType::PLUS:        return "plus";
        case TokenType::MINUS:       return "minus";
        case TokenType::TIMES:       return "times";
        case TokenType::IDIV:        return "idiv";
        case TokenType::RDIV:        return "rdiv";
        case TokenType::IMOD:        return "imod";

        case TokenType::NOTSY:       return "notsy";
        case TokenType::ANDSY:       return "andsy";
        case TokenType::ORSY:        return "orsy";

        case TokenType::EQL:         return "eql";
        case TokenType::NEQ:         return "neq";
        case TokenType::GTR:         return "gtr";
        case TokenType::GEQ:         return "geq";
        case TokenType::LSS:         return "lss";
        case TokenType::LEQ:         return "leq";

        case TokenType::LPARENT:     return "lparent";
        case TokenType::RPARENT:     return "rparent";
        case TokenType::LBRACK:      return "lbrack";
        case TokenType::RBRACK:      return "rbrack";
        case TokenType::COMMA:       return "comma";
        case TokenType::SEMICOLON:   return "semicolon";
        case TokenType::PERIOD:      return "period";
        case TokenType::COLON:       return "colon";
        case TokenType::BECOMES:     return "becomes";

        case TokenType::CONSTSY:     return "constsy";
        case TokenType::TYPESY:      return "typesy";
        case TokenType::VARSY:       return "varsy";
        case TokenType::FUNCTIONSY:  return "functionsy";
        case TokenType::PROCEDURESY: return "proceduresy";
        case TokenType::ARRAYSY:     return "arraysy";
        case TokenType::RECORDSY:    return "recordsy";
        case TokenType::PROGRAMSY:   return "programsy";

        case TokenType::BEGINSY:     return "beginsy";
        case TokenType::ENDSY:       return "endsy";
        case TokenType::IFSY:        return "ifsy";
        case TokenType::THENSY:      return "thensy";
        case TokenType::ELSESY:      return "elsesy";
        case TokenType::CASESY:      return "casesy";
        case TokenType::OFSY:        return "ofsy";
        case TokenType::WHILESY:     return "whilesy";
        case TokenType::DOSY:        return "dosy";
        case TokenType::FORSY:       return "forsy";
        case TokenType::TOSY:        return "tosy";
        case TokenType::DOWNTOSY:    return "downtosy";
        case TokenType::REPEATSY:    return "repeatsy";
        case TokenType::UNTILSY:     return "untilsy";

        case TokenType::IDENT:       return "ident";
        case TokenType::COMMENT:     return "comment";
        case TokenType::ERROR_TOK:   return "error";
        default:                     return "unknown";
    }
}

bool tokenHasValue(TokenType type) {
    switch (type) {
        case TokenType::INTCON:
        case TokenType::REALCON:
        case TokenType::CHARCON:
        case TokenType::STRING_TOK:
        case TokenType::IDENT:
        case TokenType::COMMENT:
        case TokenType::ERROR_TOK:
            return true;
        default:
            return false;
    }
}