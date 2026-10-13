#include "PipelineBuilder.hpp"
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>

static std::string usage() {
    return "LogFlow v3 (C++17)\n"
           "Usage: logflow --input FILE --output FILE [--min-status N] "
           "[--path-prefix PREFIX] [--fields a,b,c] [--from TIME] [--to TIME]\n";
}
int main(int argc, char** argv) {
    std::map<std::string, std::string> args;
    for (int i=1; i<argc; ++i) {
        std::string key=argv[i];
        if (key=="--help") { std::cout << usage(); return 0; }
        if (key.rfind("--",0)!=0 || i+1>=argc) { std::cerr << usage(); return 2; }
        args[key]=argv[++i];
    }
    if (!args.count("--input") || !args.count("--output")) {
        std::cerr << usage(); return 2;
    }
    try {
        FileLineSource source(args["--input"]);
        std::vector<PredicateFilterStage> filters;
        if (args.count("--min-status")) {
            const int minimum=std::stoi(args["--min-status"]);
            filters.emplace_back([minimum](const LogRecord& r){ return r.status() >= minimum; });
        }
        if (args.count("--path-prefix")) {
            const std::string prefix=args["--path-prefix"];
            filters.emplace_back([prefix](const LogRecord& r){
                return r.path().rfind(prefix,0)==0;
            });
        }
        // Common Log Format timestamps have one consistent textual format in a file.
        if (args.count("--from")) {
            const std::string lower=args["--from"];
            filters.emplace_back([lower](const LogRecord& r){ return r.get("timestamp") >= lower; });
        }
        if (args.count("--to")) {
            const std::string upper=args["--to"];
            filters.emplace_back([upper](const LogRecord& r){ return r.get("timestamp") <= upper; });
        }
        auto fields=args.count("--fields") ? splitComma(args["--fields"]) : LOG_FIELDS;
        if (!validFields(fields)) throw std::invalid_argument("Invalid --fields. Valid fields: timestamp,clientIp,method,path,status,bytes");
        PipelineBuilder builder=PipelineBuilder::from(source);
        for (const auto& filter : filters) builder.via(filter);
        builder.project(fields);
        Pipeline pipeline=builder.to(args["--output"]);
        auto stats=pipeline.run(source,args["--output"]);
        std::cout << "Parsed: " << stats.parsed << ", skipped malformed: " << stats.skipped
                  << ", written: " << stats.written << ", elapsed: " << stats.elapsedMs << " ms\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }
}
