# AIL Bytecode Interpreter
This is a simple AIL bytecode interpreter, implemented in C.

It partially uses the C backend template as its runtime library.

TODO: Clean & speed up the code.

## Dependencies
* `stdlib/backend/stringcode/c/template.c` as well as all of its dependencies
* `thirdparty/appbundle` (only when building the interpreter as a standalone application, i.e. `-DASPL_AILI_BUNDLED`)