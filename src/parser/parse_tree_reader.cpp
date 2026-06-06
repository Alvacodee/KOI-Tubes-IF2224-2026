#include "parse_tree_reader.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

std::string ParseTreeReader::trimRight(const std::string& s) {
    size_t end = s.find_last_not_of(" \t\r\n");
    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

int ParseTreeReader::computeDepth(const std::string& line) {
    size_t branchPos = std::string::npos;

    for (size_t i = 0; i + 2 < line.size(); i++) {
        unsigned char b0 = (unsigned char)line[i];
        unsigned char b1 = (unsigned char)line[i + 1];
        unsigned char b2 = (unsigned char)line[i + 2];

        if (b0 == 0xE2 && b1 == 0x94 && (b2 == 0x9C || b2 == 0x94)) {
            branchPos = i;
            break;
        }
    }

    if (branchPos == std::string::npos) {
        return 0;
    }

    int depth = 0;
    size_t pos = 0;

    while (pos < branchPos) {
        unsigned char b = (unsigned char)line[pos];

        if (b == 0xE2 && pos + 2 < line.size()) {
            unsigned char b1 = (unsigned char)line[pos + 1];
            unsigned char b2 = (unsigned char)line[pos + 2];
            if (b1 == 0x94 && b2 == 0x82) {
                pos += 3;
                int skipped = 0;
                while (pos < branchPos && line[pos] == ' ' && skipped < 3) {
                    pos++;
                    skipped++;
                }
                depth++;
                continue;
            }
        }

        if (b == ' ') {
            int spaces = 0;
            while (pos < branchPos && line[pos] == ' ' && spaces < 4) {
                pos++;
                spaces++;
            }
            if (spaces >= 4) depth++;
            continue;
        }

        pos++;
    }

    return depth + 1;
}

std::string ParseTreeReader::extractLabel(const std::string& line) {
    for (size_t i = 0; i + 2 < line.size(); i++) {
        unsigned char b0 = (unsigned char)line[i];
        unsigned char b1 = (unsigned char)line[i + 1];
        unsigned char b2 = (unsigned char)line[i + 2];

        if (b0 == 0xE2 && b1 == 0x94 && (b2 == 0x9C || b2 == 0x94)) {
            size_t labelStart = i + 3;

            while (labelStart + 2 < line.size()) {
                unsigned char lb0 = (unsigned char)line[labelStart];
                unsigned char lb1 = (unsigned char)line[labelStart + 1];
                unsigned char lb2 = (unsigned char)line[labelStart + 2];
                if (lb0 == 0xE2 && lb1 == 0x94 && lb2 == 0x80) {
                    labelStart += 3;
                } else {
                    break;
                }
            }

            while (labelStart < line.size() && line[labelStart] == ' ')
                labelStart++;

            return trimRight(line.substr(labelStart));
        }
    }

    return trimRight(line);
}

std::vector<RawParsedLine> ParseTreeReader::parseLines(
    const std::vector<std::string>& lines) {

    std::vector<RawParsedLine> result;

    for (const auto& line : lines) {
        std::string trimmed = trimRight(line);
        if (trimmed.empty()) continue;

        RawParsedLine entry;
        entry.depth = computeDepth(trimmed);
        entry.label = extractLabel(trimmed);

        if (!entry.label.empty()) {
            result.push_back(entry);
        }
    }

    return result;
}

ParseTreeNode* ParseTreeReader::buildTree(
    const std::vector<RawParsedLine>& parsed) {

    if (parsed.empty()) return nullptr;

    ParseTreeNode* root = new ParseTreeNode(parsed[0].label);

    std::vector<std::pair<int, ParseTreeNode*>> stack;
    stack.push_back({parsed[0].depth, root});

    for (size_t i = 1; i < parsed.size(); i++) {
        int depth = parsed[i].depth;
        const std::string& label = parsed[i].label;

        ParseTreeNode* node = new ParseTreeNode(label);

        while (stack.size() > 1 && stack.back().first >= depth) {
            stack.pop_back();
        }

        ParseTreeNode* parent = stack.back().second;
        parent->addChild(node);

        stack.push_back({depth, node});
    }

    return root;
}

ParseTreeNode* ParseTreeReader::readFromFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Error: Tidak dapat membuka file parse tree: "
                  << filepath << std::endl;
        return nullptr;
    }

    std::vector<std::string> lines;
    std::string line;
    while (std::getline(file, line)) {
        lines.push_back(line);
    }
    file.close();

    if (lines.empty()) {
        std::cerr << "Error: File parse tree kosong: " << filepath << std::endl;
        return nullptr;
    }

    auto parsed = parseLines(lines);
    return buildTree(parsed);
}

ParseTreeNode* ParseTreeReader::readFromString(const std::string& content) {
    std::istringstream iss(content);
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(iss, line)) {
        lines.push_back(line);
    }

    if (lines.empty()) return nullptr;

    auto parsed = parseLines(lines);
    return buildTree(parsed);
}
