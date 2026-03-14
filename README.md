<div align="center">
<img src="https://github.com/aspl-lang/logo/blob/main/logo.svg" width=192 alt="The ASPL logo">
<h1>The ASPL Programming Language</h1>
</div>

<div align="center">

[![CI/CD Suite](https://github.com/aspl-lang/aspl/actions/workflows/ci_cd.yml/badge.svg)](https://github.com/aspl-lang/aspl/actions/workflows/ci_cd.yml)
![Commit Activity](https://img.shields.io/github/commit-activity/m/aspl-lang/aspl)
[![Discord](https://img.shields.io/discord/1053681798430859264?label=Discord&logo=discord&logoColor=white)](https://discord.gg/UUNzAFrKU2)
[![X](https://img.shields.io/badge/follow-%40aspl__lang-1DA1F2?logo=x&style=flat&logoColor=white)](https://x.com/aspl_lang)

ASPL is a high-level, general-purpose programming language for building modern, safe, and portable software.

</div>

## Why ASPL?
* **Simple** 💡
<br>The syntax is clean and readable, the compiler is easy to use and provides meaningful insights, and installing the whole ASPL toolchain is as straightforward as a few clicks.

* **Safe** 🔒
<br>High-level abstractions, automatic memory management, a strong type system, and out-of-the-box TLS support make ASPL safe by design.

* **Cross-platform** 🌐
<br>Newcomers and experienced programmers alike can easily create powerful cross-platform applications with ASPL; seamless cross-compilation is one of ASPL's core design principles.

* **Powerful standard library** 🔥
<br>Built-in JSON, graphics, and advanced networking support are just a few of the things that will drastically speed up your coding process.

* **Modular architecture** 📚
<br>ASPL has a great and simple modular library system that helps you reuse any kind of third-party code in your projects, including even the ASPL compiler itself.

> [!IMPORTANT]
> ASPL is still in early development. Even though the language is already quite stable and very practical, it is not yet recommended for production use. Contributions are always welcome!

# Installing
Please have a look at <a href="install.md">the installation guide</a>.

# Introduction
Make sure to check out <a href="introduction.md">the official introduction to ASPL</a>.
<br>It also documents most of the language and contains a lot of examples.

# Bugs and Suggestions
ASPL is currently in early development and still has some bugs and missing features.
<br>Please report bugs or suggest features by <a href="https://github.com/aspl-lang/aspl/issues/new">opening an issue</a> or posting them in <a href="https://discord.gg/UUNzAFrKU2">the ASPL Discord server</a>.

# Code Examples
-> means the program prints text to the console
<br><- means the user types something into the console
## Hello World
```aspl
print("Hello World!") // print means "write into the console"
```
**Output:**
<br>-> Hello World!

## Favorite food
```aspl
var favoriteFood = input("What's your favorite food?") // prints "What's your favorite food" and waits until the user types something into the console
if(favoriteFood == "Spaghetti"){ // checks whether the users input matches a certain string, here: "Spaghetti"; if it doesn't, the code between the braces will be skipped
    print("Hm, yummy, that's my favorite food too!")
}else{ // the following code is executed only if the condition in the if statement evaluated to false, here: the input was not "Spaghetti"
    print("Sounds great!")
}
```
**Example Output:**
<br>-> What's your favorite food?
<br><- Spaghetti
<br>-> Hm, yummy, that's my favorite food too!

## Random number
```aspl
import rand

print(rand.irange(1, 100)) // prints a random number between 1 and 100 to the console
```
**Example Output:**
<br>-> 42

> [!NOTE]  
> **More examples can be found <a href="./examples/">here</a> or in the <a href="introduction.md">ASPL introduction</a>.**

# Star History
[![Star History Chart](https://api.star-history.com/svg?repos=aspl-lang/aspl&type=Date)](https://star-history.com/#aspl-lang/aspl)

<h3>👋 Feel free to join <a href="https://discord.gg/UUNzAFrKU2">the official ASPL Discord server</a>.</h3>