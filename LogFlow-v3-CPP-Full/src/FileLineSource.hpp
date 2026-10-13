#pragma once
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

class FileLineSource {
    std::string path_;
public:
    explicit FileLineSource(std::string path) : path_(std::move(path)) {}
    std::vector<std::string> readAll() const {
        std::ifstream input(path_);
        if (!input) throw std::runtime_error("Cannot open input file: " + path_);
        std::vector<std::string> lines;
        std::string line;
        while (std::getline(input, line)) lines.push_back(line);
        return lines;
    }
};
