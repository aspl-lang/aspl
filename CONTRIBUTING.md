# Contributing to ASPL
## Creating something awesome
One of the best ways to support ASPL is by creating a project in it. You may port an already exisiting tool, develop something innovative and new or even rewrite your own applications in ASPL.

Only through such projects, we can find out how usable ASPL really is in different fields of computer science, catch and patch bugs, and implement new features. It also just massively helps in making ASPL more know and popular.

## Actually contributing to the implementation
You can of course also directly contribute to ASPLs sourcecode (and the stuff around it) here on Github.
<br>The best way to get started is probably just looking around at the files and folders; here's a quick overview on how the code is structured:

### Compiler and standard library
The compiler and all the stuff around it is written in ASPL and part of the ASPL standard library, which is located in the `stdlib` folder.
<br>Please read the REAMDE of the `stdlib/aspl` folder for more information.

### Runtime and AIL interpreter
The runtime and the AIL interpreter are written in C; they are currently located at `stdlib/aspl/compiler/backend/stringcode/c/template.c` and `runtime/ailinterpreter` respectively.