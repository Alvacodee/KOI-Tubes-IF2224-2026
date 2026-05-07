#pragma once
#include <string>
#include <vector>
#include <memory>

// ParseNode merepresentasikan satu node di dalam parse tree.
// node bisa berupa non-terminal (punya child) atau terminal (leaf), berisi token.
struct ParseNode {
    std::string label; // nama node, misal "<expression>" atau "intcon(5)"
    std::vector<std::shared_ptr<ParseNode>> children;

    explicit ParseNode(const std::string& l) : label(l) {}

    void addChild(std::shared_ptr<ParseNode> child) {
        children.push_back(std::move(child));
    }
};

// helper: buat non-terminal node, misal makeNode("<expression>")
inline std::shared_ptr<ParseNode> makeNode(const std::string& label) {
    return std::make_shared<ParseNode>(label);
}

// helper: buat terminal/leaf node dari token
inline std::shared_ptr<ParseNode> makeLeaf(const std::string& tokenName, const std::string& value = "") {
    if (value.empty())
        return std::make_shared<ParseNode>(tokenName);
    return std::make_shared<ParseNode>(tokenName + "(" + value + ")");
}

// print parse tree ke ostream dengan indentasi gaya ASCII tree
void printTree(const std::shared_ptr<ParseNode>& node, std::ostream& out,
               const std::string& prefix = "", bool isLast = true);
