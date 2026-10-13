#pragma once
#include <fstream>
#include <stdexcept>
#include <string>

class FileSink {
    std::string path_;
    std::ofstream output_;
public:
    explicit FileSink(std::string path) : path_(std::move(path)) {}
    FileSink(const FileSink&) = delete;
    FileSink& operator=(const FileSink&) = delete;
    void open() {
        if (output_.is_open()) return;
        output_.open(path_, std::ios::out | std::ios::trunc);
        if (!output_) throw std::runtime_error("Cannot open output file: " + path_);
    }
    void write(const std::string& line) {
        if (!output_.is_open()) throw std::runtime_error("FileSink must be opened before writing");
        output_ << line << '\n';
        if (!output_) throw std::runtime_error("Failed while writing output file");
    }
    void close() {
        if (output_.is_open()) {
            output_.flush();
            output_.close();
            if (output_.fail()) throw std::runtime_error("Failed to close output file");
        }
    }
    ~FileSink() { if (output_.is_open()) output_.close(); }
};
