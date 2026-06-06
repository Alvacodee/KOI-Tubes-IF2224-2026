#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <fstream>

class ParseTreeNode {
public:
    std::string name;
    std::vector<ParseTreeNode*> children;

    explicit ParseTreeNode(const std::string& name);

    ~ParseTreeNode();

    void addChild(ParseTreeNode* child);

    void print(std::ostream& out, const std::string& prefix = "", bool isLast = true) const;

    void printToFile(const std::string& filepath) const;
};
