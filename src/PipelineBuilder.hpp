#pragma once
#include "Pipeline.hpp"
#include <functional>
#include <utility>
#include <vector>

// Fluent builder: from(source).via(filter).project(fields).to(outputPath)
class PipelineBuilder {
    const FileLineSource* source_ = nullptr;
    std::vector<PredicateFilterStage> filters_;
    std::vector<std::string> fields_ = LOG_FIELDS;
public:
    static PipelineBuilder from(const FileLineSource& source) {
        PipelineBuilder builder; builder.source_ = &source; return builder;
    }
    PipelineBuilder& via(const PredicateFilterStage& stage) {
        filters_.push_back(stage); return *this;
    }
    PipelineBuilder& project(std::vector<std::string> fields) {
        if (!validFields(fields)) throw std::invalid_argument("Unknown or empty field list");
        fields_ = std::move(fields); return *this;
    }
    Pipeline to(const std::string& outputPath) const {
        if (!source_) throw std::logic_error("PipelineBuilder::from(source) must be called first");
        (void)outputPath; // The path is supplied to Pipeline::run to allow reusing a pipeline.
        return Pipeline(filters_, ProjectionStage(fields_));
    }
};
