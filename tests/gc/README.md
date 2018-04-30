# GC Testing

This folder is primarily used to test garbage collection.

## Testing garbage collection
This will test the entire interpreter, which compiles MITScript to bytecode then executes the bytecode, using the binary `vm/mitscript` and flag `-s`, using massif, a Valgrind tool for profiling heap memory usage.

Run `test_massif.sh`, optionally followed by a string that matches all tests you want to run. If the string matches exactly one filename, and the test fails, you will have the option to overwrite the expected test output file with the actual output just obtained.

To change the memory limit, set `$LIMIT` inside the script to be your intended limit in MB (default is 4 MB).

Running the command with no argument will run all tests.

## Test File Organization
Tests in this folder are organized according to the following convention:
* Tests that expect parsing, compilation, and execution to succeed contain `good` in their names, and `bad` otherwise.
* Tests written explicitly to test MITScript to bytecode have the prefix `ms`, and tests written explicitly to test bytecode to program output have the prefix `bc`. Otherwise, tests for MITScript to bytecode have neither prefix.
* Staff-supplied tests are prefixed with `staff`.
