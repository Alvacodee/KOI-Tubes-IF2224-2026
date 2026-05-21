CXX      = g++
CXXFLAGS = -Wall -Wextra -std=c++17
SRC_DIR  = src
OBJ_DIR  = build

SRCS = $(SRC_DIR)/main.cpp \
       $(SRC_DIR)/token.cpp \
       $(SRC_DIR)/identifier.cpp \
       $(SRC_DIR)/operator.cpp \
       $(SRC_DIR)/lexer.cpp \
       $(SRC_DIR)/parse_tree_node.cpp \
       $(SRC_DIR)/parser.cpp \
       $(SRC_DIR)/parser_declaration.cpp \
       $(SRC_DIR)/expression.cpp \
       $(SRC_DIR)/ast_node.cpp \
       $(SRC_DIR)/symbol_table.cpp \
       $(SRC_DIR)/semantic.cpp \
       $(SRC_DIR)/semantic_decl.cpp \
       $(SRC_DIR)/parse_tree_reader.cpp

OBJS   = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))
TARGET = arion

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(TARGET)

.PHONY: all clean
