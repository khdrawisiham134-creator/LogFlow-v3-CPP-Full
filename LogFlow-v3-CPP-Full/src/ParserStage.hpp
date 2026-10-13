#pragma once
#include "LogRecord.hpp"
#include <regex>
#include <string>

class ParserStage {
    const std::regex pattern_{
        R"(^(\S+) \S+ \S+ \[([^\]]+)\] "(\S+) (\S+)[^"]*" (\d{3}) (\d+|-)$)"
    };
    std::size_t skipped_ = 0;
public:
    bool parse(const std::string& line, LogRecord& record) {
        std::smatch match;
        if (!std::regex_match(line, match, pattern_)) {
            ++skipped_;
            return false;
        }
        record.fields["clientIp"] = match[1].str();
        record.fields["timestamp"] = match[2].str();
        record.fields["method"] = match[3].str();
        record.fields["path"] = match[4].str();
        record.fields["status"] = match[5].str();
        record.fields["bytes"] = match[6].str() == "-" ? "0" : match[6].str();
        return true;
    }
    std::size_t skipped() const { return skipped_; }
};
