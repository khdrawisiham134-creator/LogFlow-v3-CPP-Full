#pragma once
#include "LogRecord.hpp"
#include <functional>
#include <vector>

class PredicateFilterStage {
    std::function<bool(const LogRecord&)> predicate_;
public:
    explicit PredicateFilterStage(std::function<bool(const LogRecord&)> predicate)
        : predicate_(std::move(predicate)) {}
    bool accepts(const LogRecord& record) const { return predicate_(record); }
    std::vector<LogRecord> apply(const std::vector<LogRecord>& input) const {
        std::vector<LogRecord> output;
        output.reserve(input.size());
        for (const auto& record : input) if (accepts(record)) output.push_back(record);
        return output;
    }
};
