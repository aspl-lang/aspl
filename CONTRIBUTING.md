# Contributing to ASPL
## Creating something awesome
One of the best ways to support ASPL is by writing a project in it. You may port an already existing tool, develop something innovative and new, or even rewrite your own applications in ASPL.

Such projects are essential for assessing ASPL's usability across various fields of computer science, catching and fixing bugs, and adding new features. It also just massively helps in making ASPL more known and popular.

## Actually contributing to the implementation
You can, of course, also directly contribute to ASPL's sourcecode (and the surrounding stuff) here on Github.
<br>The best way to get started is probably just looking around at the files and folders; here's a quick overview of how the code is structured:

### Compiler and standard library
The ASPL standard library is mostly written in ASPL itself (plus some C for low-level code) and located in the `stdlib` folder.
<br>Due to the special design of the ASPL monorepo, the compiler (along with some related components such as the parser) is actually _part of the standard library_ and thus also self-hosted.
<br>Please read the REAMDE of the `stdlib/aspl` folder for more information.

### Runtime and AIL interpreter
The runtime and the AIL interpreter are written in C; they are currently located at `stdlib/aspl/compiler/backend/stringcode/c/template.c` and `runtime/ailinterpreter` respectively.

> [!TIP]
> When you're new to contributing to ASPL or open source in particular, make sure to check out [issues labelled with `good first issue`](https://github.com/aspl-lang/aspl/issues?q=state%3Aopen%20label%3A%22good%20first%20issue%22) - they're generally pretty easy to handle, even with little to no prior experience with the codebase.