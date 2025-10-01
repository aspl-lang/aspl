# The ASPL compiler & co.
## Overview
This module contains submodules for the `parser`, `compiler` and `formatter` of the ASPL toolchain.
<br>It can be imported and used like any other ASPL module and has no dependencies[^1] except for other modules of the standard library.
<br>Note that this module does not contain any CLI functionality, but it is instead imported and used by the `cli` folder in the root directory of the ASPL project, which implements the `aspl` command along with all its subcommands.

[^1]: When using the AIL backend, the compiler requires pre-compiled binary templates when packaging ASPL programs into an executable; please refer to the `templates` folder in the root directory of the ASPL project for more information on this. When using the C backend, the compiler requires certain files from the `thirdparty` directory as well as a compatible C compiler (GCC, Clang, Zig CC) to be installed on the host OS.

## Example usage
```aspl
import aspl.compiler

aspl.compiler.compile("hello.aspl")
```
That's it. You can now run the compiled program by executing `./hello` (or `hello` if you're on Windows).

For configuration options and more information, please refer to the `parser` and `compiler` submodules.