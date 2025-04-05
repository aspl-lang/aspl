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

### Conventions in the ASPL monorepo
To ensure a high quality and consistent style across the whole codebase and also the non-code elements of the ASPL monorepo, you are expected to stick to the following conventions whenever you are contributing to this project (e.g. through issues or pull requests).

None of these conventions are set in stone and may change over time, but they _should_ generally be respected. Expect your issues and PRs to be ignored as long as they don't follow the proper style. If you've got any questions or suggestions regarding the conventions in the ASPL monorepo, make sure to reach out to some maintainers or other experienced contributors and voice your concerns.

#### Issue/PR titles and commit messages
All titles of issues/PRs as well as all commit messages should follow this pattern: `module: concise description`

Examples:
* graphics: add `Canvas:fromData` to load images directly from a bitmap buffer in memory
* cli: fix the `update` subcommand
* introduction: fix a typo (wrong variable name in example code)

As you can see from the examples, quotes or identifiers should be marked using backticks (`` ` ``). Some other details include not putting a period at the end of a message, starting in lowercase, and using an imperative tone and the present tense.

Changes that affect the whole language should be labeled with the `aspl` prefix (e.g. "aspl: add the `string.replaceMany` method"); changes that only affect parts of the implementation (e.g. the compiler or the AIL interpreter) should be labeled accordingly (e.g. "parser: fix a naming inconsistency").

If you feel like you cannot stick to this convention in your specific situation (e.g. because your change is affecting multiple modules at the same time) or cannot come up with a meaningful description message, don't be afraid to reach out to other contributors and ask them for advice.

#### Naming conventions
In theory, ASPL does not impose a lot of restrictions on how you name symbols in your source code. Yet, in practice, it is very reasonable to stick to a single naming convention in each project, and it can also be very useful when many projects use the same style. Thus, you might want to adhere to these principles when publishing ASPL code, and especially when you're contributing to this project itself:

* Variable names shall be camelCase
* Function names shall be snake_case and imperative
* Method names shall be camelCase and imperative
* Class names and enum names shall be PascalCase and singular
* Namespaces shall be flatcase and plural

#### Formatting conventions
There is currently no official or even consistent convention for formatting ASPL code. Yet, most of the standard library (including the compiler) is written in a very similar style and mostly sticks to the same rules. This will be clarified and cleaned up later (when the ASPL formatter is finally done).