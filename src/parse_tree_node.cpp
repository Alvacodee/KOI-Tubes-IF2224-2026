#include "parse_tree_node.hpp"

ParseTreeNode::ParseTreeNode(const std::string& name)
    : name(name) {}

ParseTreeNode::~ParseTreeNode() {
    for (size_t i = 0; i < children.size(); i++) {
        delete children[i];
    }
    children.clear();
}

void ParseTreeNode::addChild(ParseTreeNode* child) {
    children.push_back(child);
}

void ParseTreeNode::print(std::ostream& out, const std::string& prefix, bool isLast) const {
    std::string currentPrefix = prefix;
    if (currentPrefix == " ") currentPrefix = ""; 

    if (prefix.empty()) { 
        out << name << std::endl;
        for (size_t i = 0; i < children.size(); i++) {
            bool last = (i == children.size() - 1);
            children[i]->print(out, " ", last); 
        }
    } else {
        out << currentPrefix;
        out << (isLast ? "└── " : "├── ") << name << std::endl;

        std::string childPrefix = currentPrefix + (isLast ? "    " : "│   ");
        for (size_t i = 0; i < children.size(); i++) {
            bool last = (i == children.size() - 1);
            children[i]->print(out, childPrefix, last);
        }
    }
}

void ParseTreeNode::printToFile(const std::string& filepath) const {
    std::ofstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Error: Tidak dapat membuka file " << filepath << std::endl;
        return;
    }
    print(file);
    file.close();
}