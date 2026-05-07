CC      = g++
CFLAGS  = -Wall -Wextra -std=c++17
SRC_DIR = src
OBJ_DIR = build
TARGET  = arion

SRCS = $(SRC_DIR)/main.cpp \
       $(SRC_DIR)/token.cpp \
       $(SRC_DIR)/identifier.cpp \
       $(SRC_DIR)/literal.cpp \
       $(SRC_DIR)/operator.cpp \
       $(SRC_DIR)/lexer.cpp \
       $(SRC_DIR)/parse_tree_node.cpp \
       $(SRC_DIR)/parser.cpp \
       $(SRC_DIR)/parser_declaration.cpp \
       $(SRC_DIR)/expression.cpp

OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf $(OBJ_DIR) $(TARGET)

.PHONY: all clean run
