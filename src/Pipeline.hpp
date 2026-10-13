#pragma once
#include "FileLineSource.hpp"
#include "FileSink.hpp"
#include "ParserStage.hpp"
#include "PredicateFilterStage.hpp"
#include "ProjectionStage.hpp"
#include <chrono>
#include <functional>
#include <string>
#include <vector>

class Pipeline {
    std::vector<PredicateFilterStage> filters_;
    ProjectionStage projection_;
public:
    Pipeline(std::vector<PredicateFilterStage> filters, ProjectionStage projection)
        : filters_(std::move(filters)), projection_(std::move(projection)) {}
    struct Stats { std::size_t parsed=0, skipped=0, written=0; double elapsedMs=0; };
    Stats run(const FileLineSource& source, const std::string& outputPath) const {
        const auto start = std::chrono::steady_clock::now();
        ParserStage parser;
        FileSink sink(outputPath);
        sink.open();
        Stats stats;
        try {
            for (const auto& line : source.readAll()) {
                LogRecord record;
                if (!parser.parse(line, record)) continue;
                ++stats.parsed;
                bool keep = true;
                for (const auto& filter : filters_) {
                    if (!filter.accepts(record)) { keep=false; break; }
                }
                if (!keep) continue;
                auto projected = projection_.apply(std::vector<LogRecord>{record});
                sink.write(projected.front().toTsv(projection_.fields()));
                ++stats.written;
            }
            stats.skipped = parser.skipped();
            sink.close();
        } catch (...) {
            try { sink.close(); } catch (...) {}
            throw;
        }
        stats.elapsedMs = std::chrono::duration<double, std::milli>(
            std::chrono::steady_clock::now()-start).count();
        return stats;
    }
};
