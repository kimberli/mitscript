# VM Testing

This folder has a lot of files, but the testing framework should make running and deciphering tests easier.

We test three components here: the compiler, VM, and interpreter:

## Testing the compiler
This tests compilation of MITScript to bytecode, using the binary `vm/mitscriptc`.

Run `test_compiler.sh`, optionally followed by a string that matches all tests you want to run. If the string matches exactly one filename, and the test fails, you will have the option to overwrite the expected test output file with the actual output just obtained.

Running the command with no argument will run all matching tests.

## Testing the VM
This tests bytecode execution, using the binary `vm/mitscript` and flag `-b`.

Run `test_vm.sh`, optionally followed by a string that matches all tests you want to run. If the string matches exactly one filename, and the test fails, you will have the option to overwrite the expected test output file with the actual output just obtained.

Running the command with no argument will run all matching tests.

## Testing the interpreter
This will test the entire interpreter, which compiles MITScript to bytecode then executes the bytecode, using the binary `vm/mitscript` and flag `-s`.

Run `test_interpreter.sh`, optionally followed by a string that matches all tests you want to run. If the string matches exactly one filename, and the test fails, you will have the option to overwrite the expected test output file with the actual output just obtained.

Running the command with no argument will run all matching tests.

## Test File Organization
Tests in this folder are organized according to the following convention:
* Tests that expect parsing, compilation, and execution to succeed contain `good` in their names, and `bad` otherwise.
* Tests written explicitly to test MITScript to bytecode have the prefix `ms`, and tests written explicitly to test bytecode to program output have the prefix `bc`. Otherwise, tests for MITScript to bytecode have neither prefix.
* Staff-supplied tests are prefixed with `staff`.
