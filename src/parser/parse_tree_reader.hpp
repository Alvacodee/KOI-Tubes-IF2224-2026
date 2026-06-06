#pragma once
#include "parse_tree_node.hpp"
#include <string>
#include <vector>

struct RawParsedLine {
    int depth;
    std::string label;
};

class ParseTreeReader {
public:
    static ParseTreeNode* readFromFile(const std::string& filepath);
    static ParseTreeNode* readFromString(const std::string& content);

private:
    static std::vector<RawParsedLine> parseLines(const std::vector<std::string>& lines);
    static ParseTreeNode* buildTree(const std::vector<RawParsedLine>& parsed);
    static int computeDepth(const std::string& line);
    static std::string extractLabel(const std::string& line);
    static std::string trimRight(const std::string& s);
};
