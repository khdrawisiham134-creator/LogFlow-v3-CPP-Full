#pragma once
#include "LogRecord.hpp"
#include <stdexcept>
#include <vector>

class ProjectionStage {
    std::vector<std::string> selected_;
public:
    explicit ProjectionStage(std::vector<std::string> fields) : selected_(std::move(fields)) {
        if (!validFields(selected_)) throw std::invalid_argument("Unknown or empty projection field list");
    }
    std::vector<LogRecord> apply(const std::vector<LogRecord>& input) const {
        std::vector<LogRecord> output;
        output.reserve(input.size());
        for (const auto& record : input) {
            LogRecord projected;
            for (const auto& name : selected_) {
                auto it = record.fields.find(name);
                if (it != record.fields.end()) projected.fields[name] = it->second;
            }
            output.push_back(std::move(projected));
        }
        return output;
    }
    const std::vector<std::string>& fields() const { return selected_; }
};
