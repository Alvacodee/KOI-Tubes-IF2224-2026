CXX      = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -Isrc/lexer -Isrc/parser -Isrc/ast -Isrc/semantic -Isrc/icg -Isrc/interpreter
OBJ_DIR  = build
TARGET   = arion

SRCS = src/main.cpp \
       src/lexer/token.cpp \
       src/ast/identifier.cpp \
       src/ast/operator.cpp \
       src/lexer/lexer.cpp \
       src/parser/parse_tree_node.cpp \
       src/parser/parser.cpp \
       src/parser/parser_declaration.cpp \
       src/ast/expression.cpp \
       src/ast/ast_node.cpp \
       src/semantic/symbol_table.cpp \
       src/semantic/semantic.cpp \
       src/semantic/semantic_decl.cpp \
       src/parser/parse_tree_reader.cpp \
       src/icg/intermediate_code.cpp \
       src/interpreter/interpreter.cpp

OBJS = $(patsubst src/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

$(OBJ_DIR)/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf $(OBJ_DIR) $(TARGET)

.PHONY: all clean run
