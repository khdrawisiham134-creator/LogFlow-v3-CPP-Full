#pragma once
#include <map>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <stdexcept>

struct LogRecord {
    std::map<std::string, std::string> fields;
    std::string get(const std::string& key) const {
        auto it = fields.find(key);
        return it == fields.end() ? "" : it->second;
    }
    int status() const {
        try { return std::stoi(get("status")); } catch (...) { return -1; }
    }
    std::string path() const { return get("path"); }
    std::string toTsv(const std::vector<std::string>& selected) const {
        std::ostringstream out;
        for (std::size_t i=0; i<selected.size(); ++i) {
            if (i) out << '\t';
            out << get(selected[i]);
        }
        return out.str();
    }
};
inline const std::vector<std::string> LOG_FIELDS =
    {"timestamp", "clientIp", "method", "path", "status", "bytes"};
inline std::vector<std::string> splitComma(const std::string& value) {
    std::vector<std::string> result;
    std::stringstream stream(value);
    std::string item;
    while (std::getline(stream, item, ',')) {
        if (!item.empty()) result.push_back(item);
    }
    return result;
}
inline bool validFields(const std::vector<std::string>& fields) {
    return !fields.empty() && std::all_of(fields.begin(), fields.end(), [](const auto& f) {
        return std::find(LOG_FIELDS.begin(), LOG_FIELDS.end(), f) != LOG_FIELDS.end();
    });
}
