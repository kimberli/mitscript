# Testing

* The `parser` folder tests the pretty print functionality for MITScript. Run `test.sh` in that directory to run all tests.
* The `vm` folder contains tests for pretty printing from the bytecode compiler (`test_compiler.sh`), for correctness in bytecode execution (`test_vm.sh`), and for correctness in MITScript execution (`test_interpreter.sh`). See the `README` in that folder for more details.
* The `gc` folder contains heap memory usage tests for the entire interpreter in `test_massif.sh`. See the `README` in that folder for more details.
