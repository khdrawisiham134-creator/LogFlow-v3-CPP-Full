# LogFlow v3 — C++17

## What it does
Reads Common Log Format logs, parses records, filters by predicates, projects selected fields, and writes tab-separated output.

## Build prerequisites
- C++17 compiler (GCC or Clang)
- CMake 3.16 or newer

## Build and test (Windows PowerShell with CMake installed)
```powershell
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```
Run on Windows (executable location can vary by generator):
```powershell
.\build\Debug\logflow.exe --input data/sample.log --output output.tsv --min-status 400 --path-prefix /api --fields timestamp,clientIp,status,path
```
For single-configuration generators, executable may be `.\build\logflow.exe`.

## CLI
- `--input FILE` (required)
- `--output FILE` (required)
- `--min-status N` keep status >= N
- `--path-prefix PREFIX` keep paths starting with PREFIX
- `--fields a,b,c` select output fields
- `--from TIME` timestamp lower bound
- `--to TIME` timestamp upper bound
- `--help` show usage

Allowed fields: `timestamp,clientIp,method,path,status,bytes`.

## Project files
See `ARCHITECTURE.md` for component design, composition, privacy/data-minimization notes, filter-order experiment and limitations.

## Important language note
This is a C++17 reimplementation of the project requirements. If the instructor explicitly requires the original Java version, submit the Java project instead or confirm that C++ is allowed.
