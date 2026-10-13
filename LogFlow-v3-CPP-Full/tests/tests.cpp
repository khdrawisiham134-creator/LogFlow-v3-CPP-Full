#include "PipelineBuilder.hpp"
#include <cassert>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

static std::vector<LogRecord> parseFixture(const std::vector<std::string>& lines, std::size_t* skipped=nullptr) {
    ParserStage parser;
    std::vector<LogRecord> records;
    for (const auto& line : lines) {
        LogRecord r;
        if (parser.parse(line,r)) records.push_back(r);
    }
    if (skipped) *skipped=parser.skipped();
    return records;
}
int main() {
    const std::vector<std::string> fixture = {
        "10.0.0.1 - - [10/Oct/2023:13:55:36 +0000] \"GET /api/users HTTP/1.1\" 200 512",
        "10.0.0.2 - - [10/Oct/2023:13:56:01 +0000] \"GET /index.html HTTP/1.1\" 200 1024",
        "10.0.0.3 - - [10/Oct/2023:13:57:12 +0000] \"POST /api/login HTTP/1.1\" 401 87",
        "10.0.0.4 - - [10/Oct/2023:13:58:45 +0000] \"GET /api/orders HTTP/1.1\" 500 0",
        "this is not a log line"
    };
    std::size_t skipped=0;
    auto parsed=parseFixture(fixture,&skipped);
    assert(parsed.size()==4 && skipped==1);

    PredicateFilterStage statusFilter([](const LogRecord& r){return r.status()>=400;});
    auto errors=statusFilter.apply(parsed);
    assert(errors.size()==2);

    PredicateFilterStage pathFilter([](const LogRecord& r){return r.path().rfind("/api",0)==0;});
    auto api=pathFilter.apply(parsed);
    assert(api.size()==3);

    ProjectionStage projection({"timestamp","clientIp","status","path"});
    auto projected=projection.apply(errors);
    assert(projected.size()==2);
    assert(projected.front().fields.size()==4);
    assert(projected.front().fields.count("method")==0);

    // Integration test: pipeline output file is checked after close/flush.
    const std::string inputPath="logflow-test-input.log";
    const std::string outputPath="logflow-test-output.tsv";
    { std::ofstream out(inputPath); for (const auto& line:fixture) out<<line<<'\n'; }
    FileLineSource source(inputPath);
    Pipeline pipeline=PipelineBuilder::from(source).via(statusFilter).via(pathFilter)
        .project({"timestamp","clientIp","status","path"}).to(outputPath);
    auto stats=pipeline.run(source,outputPath);
    assert(stats.parsed==4 && stats.skipped==1 && stats.written==2);
    std::ifstream in(outputPath);
    std::string contents((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    assert(contents.find("401")!=std::string::npos);
    assert(contents.find("500")!=std::string::npos);
    assert(contents.find("/api/login")!=std::string::npos);

    // Filter order experiment: P(Q(records)) == Q(P(records)).
    auto pq=statusFilter.apply(pathFilter.apply(parsed));
    auto qp=pathFilter.apply(statusFilter.apply(parsed));
    assert(pq.size()==qp.size());
    for (std::size_t i=0;i<pq.size();++i) {
        assert(pq[i].path()==qp[i].path());
        assert(pq[i].status()==qp[i].status());
    }

    // Benchmark-like timing experiment: reports timings but does not claim one order is always faster.
    auto t0=std::chrono::steady_clock::now();
    volatile std::size_t count1=0;
    for(int i=0;i<1000;++i) count1 += statusFilter.apply(pathFilter.apply(parsed)).size();
    auto t1=std::chrono::steady_clock::now();
    volatile std::size_t count2=0;
    for(int i=0;i<1000;++i) count2 += pathFilter.apply(statusFilter.apply(parsed)).size();
    auto t2=std::chrono::steady_clock::now();
    auto ms1=std::chrono::duration<double,std::milli>(t1-t0).count();
    auto ms2=std::chrono::duration<double,std::milli>(t2-t1).count();
    std::cout << "All tests passed. Order P(Q): " << ms1 << " ms; Q(P): " << ms2 << " ms\n";
    std::remove(inputPath.c_str()); std::remove(outputPath.c_str());
    return 0;
}
