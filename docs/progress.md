# a5 progress

We have code generation on the way working. We've been testing on basic bytecode, without closures/function calls. See the command

    ./mitscript -b tests/vm/good01-emptyReturn.mitbc --opt=all

Our plan for optimization is to implement register allocation, then maybe other add-ons like constant propagation, generational garbage collection, selective assembly generation.
