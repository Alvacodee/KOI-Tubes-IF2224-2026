// INI TADI GUA PAS IMPLEMENTASI COBA BIKIN PUNYA KALIAN JUGA BUAT NGETES PUNYA GUA JALAN GA
// CUMA KALO KALIAN MO NYOBA TINGGAL UNCOMMENT INI SAMA YANG DI PARSER.H SIH
// ABISTU TINGGAL MAKE CLEAN, MAKE, MAKE RUN, INPUT: TEST/HELLO.TXT, OUTPUT: HELLO_PARSETREE.TXT

// #include "parser.hpp"
// #include "node.hpp"

// // =============================================================================
// // File ini berisi:
// //   - parseProgram, parseProgramHeader, parseBlock, parseDeclarationPart (Daniel)
// //   - parseCompoundStatement, parseStatementList, parseStatement, dst (Awan)
// //   - parseConstDeclaration, parseVarDeclaration, dst (Neswa)
// // Saat ini berisi implementasi minimal agar bisa compile dan ditest end-to-end.
// // =============================================================================

// // Entry point
// std::shared_ptr<ParseNode> Parser::parse() {
//     // Skip komentar di awal
//     while (check(TokenType::COMMENT)) consume();
//     return parseProgram();
// }

// // Daniel: Program structure

// std::shared_ptr<ParseNode> Parser::parseProgram() {
//     auto node = makeNode("<program>");
//     node->addChild(parseProgramHeader());
//     node->addChild(parseDeclarationPart());
//     node->addChild(parseCompoundStatement());
//     Token dot = expect(TokenType::PERIOD);
//     node->addChild(makeLeaf("period"));
//     return node;
// }

// std::shared_ptr<ParseNode> Parser::parseProgramHeader() {
//     auto node = makeNode("<program-header>");
//     expect(TokenType::PROGRAMSY);
//     node->addChild(makeLeaf("programsy"));
//     Token name = expect(TokenType::IDENT);
//     node->addChild(makeLeaf("ident", name.value));
//     expect(TokenType::SEMICOLON);
//     node->addChild(makeLeaf("semicolon"));
//     return node;
// }

// std::shared_ptr<ParseNode> Parser::parseBlock() {
//     auto node = makeNode("<block>");
//     node->addChild(parseDeclarationPart());
//     node->addChild(parseCompoundStatement());
//     return node;
// }

// std::shared_ptr<ParseNode> Parser::parseDeclarationPart() {
//     auto node = makeNode("<declaration-part>");
//     while (check(TokenType::CONSTSY) || check(TokenType::TYPESY) ||
//            check(TokenType::VARSY)   || check(TokenType::PROCEDURESY) ||
//            check(TokenType::FUNCTIONSY)) {
//         if (check(TokenType::CONSTSY))        node->addChild(parseConstDeclaration());
//         else if (check(TokenType::TYPESY))    node->addChild(parseTypeDeclaration());
//         else if (check(TokenType::VARSY))     node->addChild(parseVarDeclaration());
//         else                                  node->addChild(parseSubprogramDeclaration());
//     }
//     return node;
// }

// // Neswa: Declarations (stub)

// std::shared_ptr<ParseNode> Parser::parseConstant() {
//     auto node = makeNode("<constant>");
//     if (check(TokenType::CHARCON) || check(TokenType::STRING_TOK)) {
//         Token t = consume();
//         node->addChild(makeLeaf(tokenTypeName(t.type), t.value));
//     } else {
//         if (check(TokenType::PLUS) || check(TokenType::MINUS)) {
//             Token sign = consume();
//             node->addChild(makeLeaf(tokenTypeName(sign.type)));
//         }
//         if (check(TokenType::IDENT)) {
//             Token t = consume(); node->addChild(makeLeaf("ident", t.value));
//         } else if (check(TokenType::INTCON)) {
//             Token t = consume(); node->addChild(makeLeaf("intcon", t.value));
//         } else if (check(TokenType::REALCON)) {
//             Token t = consume(); node->addChild(makeLeaf("realcon", t.value));
//         }
//     }
//     return node;
// }

// std::shared_ptr<ParseNode> Parser::parseConstDeclaration() {
//     auto node = makeNode("<const-declaration>");
//     expect(TokenType::CONSTSY); node->addChild(makeLeaf("constsy"));
//     do {
//         Token name = expect(TokenType::IDENT); node->addChild(makeLeaf("ident", name.value));
//         expect(TokenType::EQL);                node->addChild(makeLeaf("eql"));
//         node->addChild(parseConstant());
//         expect(TokenType::SEMICOLON);          node->addChild(makeLeaf("semicolon"));
//     } while (check(TokenType::IDENT) && !check(TokenType::BEGINSY));
//     return node;
// }

// std::shared_ptr<ParseNode> Parser::parseIdentifierList() {
//     auto node = makeNode("<identifier-list>");
//     Token t = expect(TokenType::IDENT); node->addChild(makeLeaf("ident", t.value));
//     while (check(TokenType::COMMA)) {
//         consume(); node->addChild(makeLeaf("comma"));
//         Token t2 = expect(TokenType::IDENT); node->addChild(makeLeaf("ident", t2.value));
//     }
//     return node;
// }

// std::shared_ptr<ParseNode> Parser::parseRange() {
//     auto node = makeNode("<range>");
//     node->addChild(parseConstant());
//     expect(TokenType::PERIOD); node->addChild(makeLeaf("period"));
//     expect(TokenType::PERIOD); node->addChild(makeLeaf("period"));
//     node->addChild(parseConstant());
//     return node;
// }

// std::shared_ptr<ParseNode> Parser::parseEnumerated() {
//     auto node = makeNode("<enumerated>");
//     expect(TokenType::LPARENT); node->addChild(makeLeaf("lparent"));
//     Token t = expect(TokenType::IDENT); node->addChild(makeLeaf("ident", t.value));
//     while (check(TokenType::COMMA)) {
//         consume(); node->addChild(makeLeaf("comma"));
//         Token t2 = expect(TokenType::IDENT); node->addChild(makeLeaf("ident", t2.value));
//     }
//     expect(TokenType::RPARENT); node->addChild(makeLeaf("rparent"));
//     return node;
// }

// std::shared_ptr<ParseNode> Parser::parseArrayType() {
//     auto node = makeNode("<array-type>");
//     expect(TokenType::ARRAYSY); node->addChild(makeLeaf("arraysy"));
//     expect(TokenType::LBRACK);  node->addChild(makeLeaf("lbrack"));
//     // range atau ident sebagai index type
//     if (check(TokenType::IDENT) && peek().type != TokenType::PERIOD) {
//         Token t = consume(); node->addChild(makeLeaf("ident", t.value));
//     } else {
//         node->addChild(parseRange());
//     }
//     expect(TokenType::RBRACK); node->addChild(makeLeaf("rbrack"));
//     expect(TokenType::OFSY);   node->addChild(makeLeaf("ofsy"));
//     node->addChild(parseType());
//     return node;
// }

// std::shared_ptr<ParseNode> Parser::parseRecordType() {
//     auto node = makeNode("<record-type>");
//     expect(TokenType::RECORDSY); node->addChild(makeLeaf("recordsy"));
//     node->addChild(parseFieldList());
//     expect(TokenType::ENDSY); node->addChild(makeLeaf("endsy"));
//     return node;
// }

// std::shared_ptr<ParseNode> Parser::parseFieldPart() {
//     auto node = makeNode("<field-part>");
//     node->addChild(parseIdentifierList());
//     expect(TokenType::COLON); node->addChild(makeLeaf("colon"));
//     node->addChild(parseType());
//     return node;
// }

// std::shared_ptr<ParseNode> Parser::parseFieldList() {
//     auto node = makeNode("<field-list>");
//     node->addChild(parseFieldPart());
//     while (check(TokenType::SEMICOLON) && peek().type == TokenType::IDENT) {
//         consume(); node->addChild(makeLeaf("semicolon"));
//         node->addChild(parseFieldPart());
//     }
//     return node;
// }

// std::shared_ptr<ParseNode> Parser::parseType() {
//     auto node = makeNode("<type>");
//     if (check(TokenType::ARRAYSY))       node->addChild(parseArrayType());
//     else if (check(TokenType::RECORDSY)) node->addChild(parseRecordType());
//     else if (check(TokenType::LPARENT))  node->addChild(parseEnumerated());
//     else {
//         // cek apakah range (constant .. constant)
//         // kalau bukan, anggap identifier biasa
//         Token t = expect(TokenType::IDENT); node->addChild(makeLeaf("ident", t.value));
//     }
//     return node;
// }

// std::shared_ptr<ParseNode> Parser::parseTypeDeclaration() {
//     auto node = makeNode("<type-declaration>");
//     expect(TokenType::TYPESY); node->addChild(makeLeaf("typesy"));
//     do {
//         Token name = expect(TokenType::IDENT); node->addChild(makeLeaf("ident", name.value));
//         expect(TokenType::EQL);                node->addChild(makeLeaf("eql"));
//         node->addChild(parseType());
//         expect(TokenType::SEMICOLON);          node->addChild(makeLeaf("semicolon"));
//     } while (check(TokenType::IDENT));
//     return node;
// }

// std::shared_ptr<ParseNode> Parser::parseVarDeclaration() {
//     auto node = makeNode("<var-declaration>");
//     expect(TokenType::VARSY); node->addChild(makeLeaf("varsy"));
//     do {
//         node->addChild(parseIdentifierList());
//         expect(TokenType::COLON);     node->addChild(makeLeaf("colon"));
//         node->addChild(parseType());
//         expect(TokenType::SEMICOLON); node->addChild(makeLeaf("semicolon"));
//     } while (check(TokenType::IDENT));
//     return node;
// }

// std::shared_ptr<ParseNode> Parser::parseParameterGroup() {
//     auto node = makeNode("<parameter-group>");
//     node->addChild(parseIdentifierList());
//     expect(TokenType::COLON); node->addChild(makeLeaf("colon"));
//     if (check(TokenType::ARRAYSY)) node->addChild(parseArrayType());
//     else { Token t = expect(TokenType::IDENT); node->addChild(makeLeaf("ident", t.value)); }
//     return node;
// }

// std::shared_ptr<ParseNode> Parser::parseFormalParameterList() {
//     auto node = makeNode("<formal-parameter-list>");
//     expect(TokenType::LPARENT); node->addChild(makeLeaf("lparent"));
//     node->addChild(parseParameterGroup());
//     while (check(TokenType::SEMICOLON)) {
//         consume(); node->addChild(makeLeaf("semicolon"));
//         node->addChild(parseParameterGroup());
//     }
//     expect(TokenType::RPARENT); node->addChild(makeLeaf("rparent"));
//     return node;
// }

// std::shared_ptr<ParseNode> Parser::parseProcedureDeclaration() {
//     auto node = makeNode("<procedure-declaration>");
//     expect(TokenType::PROCEDURESY); node->addChild(makeLeaf("proceduresy"));
//     Token name = expect(TokenType::IDENT); node->addChild(makeLeaf("ident", name.value));
//     if (check(TokenType::LPARENT)) node->addChild(parseFormalParameterList());
//     expect(TokenType::SEMICOLON); node->addChild(makeLeaf("semicolon"));
//     node->addChild(parseBlock());
//     expect(TokenType::SEMICOLON); node->addChild(makeLeaf("semicolon"));
//     return node;
// }

// std::shared_ptr<ParseNode> Parser::parseFunctionDeclaration() {
//     auto node = makeNode("<function-declaration>");
//     expect(TokenType::FUNCTIONSY); node->addChild(makeLeaf("functionsy"));
//     Token name = expect(TokenType::IDENT); node->addChild(makeLeaf("ident", name.value));
//     if (check(TokenType::LPARENT)) node->addChild(parseFormalParameterList());
//     expect(TokenType::COLON); node->addChild(makeLeaf("colon"));
//     Token ret = expect(TokenType::IDENT); node->addChild(makeLeaf("ident", ret.value));
//     expect(TokenType::SEMICOLON); node->addChild(makeLeaf("semicolon"));
//     node->addChild(parseBlock());
//     expect(TokenType::SEMICOLON); node->addChild(makeLeaf("semicolon"));
//     return node;
// }

// std::shared_ptr<ParseNode> Parser::parseSubprogramDeclaration() {
//     auto node = makeNode("<subprogram-declaration>");
//     if (check(TokenType::PROCEDURESY)) node->addChild(parseProcedureDeclaration());
//     else                               node->addChild(parseFunctionDeclaration());
//     return node;
// }

// // Awan: Statements & control flow (stub)

// std::shared_ptr<ParseNode> Parser::parseCompoundStatement() {
//     auto node = makeNode("<compound-statement>");
//     expect(TokenType::BEGINSY); node->addChild(makeLeaf("beginsy"));
//     node->addChild(parseStatementList());
//     expect(TokenType::ENDSY); node->addChild(makeLeaf("endsy"));
//     return node;
// }

// std::shared_ptr<ParseNode> Parser::parseStatementList() {
//     auto node = makeNode("<statement-list>");
//     node->addChild(parseStatement());
//     while (check(TokenType::SEMICOLON)) {
//         consume(); node->addChild(makeLeaf("semicolon"));
//         // Jangan parse statement kosong setelah endsy / until
//         if (check(TokenType::ENDSY) || check(TokenType::UNTILSY)) break;
//         node->addChild(parseStatement());
//     }
//     return node;
// }

// std::shared_ptr<ParseNode> Parser::parseStatement() {
//     auto node = makeNode("<statement>");

//     // skip komentar di dalam statement
//     while (check(TokenType::COMMENT)) consume();

//     if (check(TokenType::BEGINSY)) {
//         node->addChild(parseCompoundStatement());
//     } else if (check(TokenType::IFSY)) {
//         node->addChild(parseIfStatement());
//     } else if (check(TokenType::CASESY)) {
//         node->addChild(parseCaseStatement());
//     } else if (check(TokenType::WHILESY)) {
//         node->addChild(parseWhileStatement());
//     } else if (check(TokenType::REPEATSY)) {
//         node->addChild(parseRepeatStatement());
//     } else if (check(TokenType::FORSY)) {
//         node->addChild(parseForStatement());
//     } else if (check(TokenType::IDENT)) {
//         // Ident bisa jadi: assignment (x := ...) atau procedure/function-call (writeln(...))
//         if (peek().type == TokenType::BECOMES ||
//             peek().type == TokenType::LBRACK  ||
//             peek().type == TokenType::PERIOD) {
//             node->addChild(parseAssignmentStatement());
//         } else {
//             // procedure call (tanpa assignment)
//             node->addChild(parseProcFuncCall());
//         }
//     }
//     // statement kosong (epsilon) - ga nambah apa-apa
//     return node;
// }

// std::shared_ptr<ParseNode> Parser::parseAssignmentStatement() {
//     auto node = makeNode("<assignment-statement>");
//     node->addChild(parseVariable());
//     expect(TokenType::BECOMES); node->addChild(makeLeaf("becomes"));
//     node->addChild(parseExpression());
//     return node;
// }

// std::shared_ptr<ParseNode> Parser::parseIfStatement() {
//     auto node = makeNode("<if-statement>");
//     expect(TokenType::IFSY); node->addChild(makeLeaf("ifsy"));
//     node->addChild(parseExpression());
//     expect(TokenType::THENSY); node->addChild(makeLeaf("thensy"));
//     node->addChild(parseStatement());
//     if (check(TokenType::ELSESY)) {
//         consume(); node->addChild(makeLeaf("elsesy"));
//         node->addChild(parseStatement());
//     }
//     return node;
// }

// std::shared_ptr<ParseNode> Parser::parseCaseBlock() {
//     auto node = makeNode("<case-block>");
//     node->addChild(parseConstant());
//     while (check(TokenType::COMMA)) {
//         consume(); node->addChild(makeLeaf("comma"));
//         node->addChild(parseConstant());
//     }
//     expect(TokenType::COLON); node->addChild(makeLeaf("colon"));
//     node->addChild(parseStatement());
//     while (check(TokenType::SEMICOLON) && !check(TokenType::ENDSY)) {
//         consume(); node->addChild(makeLeaf("semicolon"));
//         if (check(TokenType::ENDSY)) break;
//         node->addChild(parseCaseBlock());
//     }
//     return node;
// }

// std::shared_ptr<ParseNode> Parser::parseCaseStatement() {
//     auto node = makeNode("<case-statement>");
//     expect(TokenType::CASESY); node->addChild(makeLeaf("casesy"));
//     node->addChild(parseExpression());
//     expect(TokenType::OFSY); node->addChild(makeLeaf("ofsy"));
//     node->addChild(parseCaseBlock());
//     expect(TokenType::ENDSY); node->addChild(makeLeaf("endsy"));
//     return node;
// }

// std::shared_ptr<ParseNode> Parser::parseWhileStatement() {
//     auto node = makeNode("<while-statement>");
//     expect(TokenType::WHILESY); node->addChild(makeLeaf("whilesy"));
//     node->addChild(parseExpression());
//     expect(TokenType::DOSY); node->addChild(makeLeaf("dosy"));
//     node->addChild(parseStatement());
//     return node;
// }

// std::shared_ptr<ParseNode> Parser::parseRepeatStatement() {
//     auto node = makeNode("<repeat-statement>");
//     expect(TokenType::REPEATSY); node->addChild(makeLeaf("repeatsy"));
//     node->addChild(parseStatementList());
//     expect(TokenType::UNTILSY); node->addChild(makeLeaf("untilsy"));
//     node->addChild(parseExpression());
//     return node;
// }

// std::shared_ptr<ParseNode> Parser::parseForStatement() {
//     auto node = makeNode("<for-statement>");
//     expect(TokenType::FORSY); node->addChild(makeLeaf("forsy"));
//     Token var = expect(TokenType::IDENT); node->addChild(makeLeaf("ident", var.value));
//     expect(TokenType::BECOMES); node->addChild(makeLeaf("becomes"));
//     node->addChild(parseExpression());
//     if (check(TokenType::TOSY)) {
//         consume(); node->addChild(makeLeaf("tosy"));
//     } else {
//         expect(TokenType::DOWNTOSY); node->addChild(makeLeaf("downtosy"));
//     }
//     node->addChild(parseExpression());
//     expect(TokenType::DOSY); node->addChild(makeLeaf("dosy"));
//     node->addChild(parseStatement());
//     return node;
// }
