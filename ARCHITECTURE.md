# LogFlow v3 — Architecture and experiment

## Stage responsibilities
- `FileLineSource.hpp`: reads lines from the input file.
- `ParserStage.hpp`: parses Common Log Format lines into a `LogRecord`; malformed lines are skipped and counted.
- `PredicateFilterStage.hpp`: applies a `bool(const LogRecord&)` predicate. Examples include status >= 400 and path starts with `/api`.
- `ProjectionStage.hpp`: keeps only selected fields. This reduces output size and limits unnecessary exposure of data (data minimization).
- `FileSink.hpp`: writes output; `open()`, `write()`, and `close()` manage resource lifecycle and flush buffered output.
- `Pipeline.hpp`: composes parsing, filters, projection and sink.
- `PipelineBuilder.hpp`: fluent construction facade: `from(source).via(filter).project(fields).to(outputPath)`.
- `main.cpp`: CLI argument parsing and pipeline configuration.

## Example
```sh
logflow --input data/sample.log --output output.tsv --min-status 400 --path-prefix /api --fields timestamp,clientIp,status,path
```

## Filter order experiment
Let P be `status >= 400` and Q be `path starts with /api`. Both are pure filters, so P(Q(records)) and Q(P(records)) produce the same records. The integration test asserts equivalence on a fixture.

The test also times 1,000 repetitions of each order and prints observed durations. These numbers are environment-dependent and small-fixture timings are not a universal performance conclusion. In general, a cheap, selective filter first can reduce work for later filters. Parsing must precede record-level filtering because raw text has no parsed `status` or `path` field.

Projection normally follows filters: projecting too early can remove fields needed by a later predicate. Projecting selected fields reduces output size and can avoid retaining unnecessary information in the output.

## Tests
`tests/tests.cpp` covers parsing and malformed lines, status filtering, path-prefix filtering, projection/data minimization, sink output content after close, integration through the builder, filter-order equivalence, and timing comparison.

## Known limitation
Timestamp bounds compare the timestamp strings lexicographically. This is suitable only when records use the same timestamp format and timezone; a production-grade parser should convert timestamps to a comparable time representation.
