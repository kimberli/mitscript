# MITScript Interpreter

## Overview of Directories
* `asm` - translation from intermediate representation (IR) to assembly
* `bc` - compilation from MITScript to bytecode
* `ex` - example folder (staff-provided)
* `gc` - garbage collection
* `ir` - translation from bytecode to IR
* `parser/bc` - bytecode parser
* `parser/ms` - MITScript source parser
* `ref` - simple reference implementation for interpreter (from assignment 2)
* `tests` - tests for parsing, execution, and memory usage
* `vm` - VM execution of bytecode instructions
* `x64asm` - third-party x86 asm library

## Building the Project
* `make ms-print` - makes the MITScript pretty printer (`parser/ms/ms-print`)
* `make bc-print` - makes the bytecode pretty printer (`parser/bc/bc-print`)
* `make bc-parser` - makes the MITScript compilation to bytecode pretty printer (`mitscriptc`)
* `make interpreter` - makes the whole interpreter (which includes a flag allowing you to either read in MITScript or bytecode) (`mitscript`)
* `make ref` - makes the reference interpreter implementation (`ref/mitscript`)
* `make test` - runs all correctness tests (in `tests/vm`)
* `make test-parser` - runs all parser tests (in `tests/parser`)
* `make test-mem` - runs heap memory usage tests (in `tests/gc`)

`make` by default makes the whole interpreter, producing the `mitscript` binary.

