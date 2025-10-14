# Introduction to ASPL
ASPL is a simple yet powerful modern cross-platform programming language.
<br>It is especially suited for people who are just getting into programming, but can also be a powerful tool for more experienced programmers.
<br>ASPL has (obviously) been influenced by multiple different other languages, most notably C# and V.

Going through this introduction will take you under an hour, but you will learn the most important skills for developing software with ASPL.
<br>Please keep in mind that this document is not (yet) a complete walkthrough of the language and there is still a lot of information missing.

> [!NOTE]
> This introduction is written very beginner-friendly, but if you aren't familiar with software engineering at all yet, you might want to learn the basics of programming through other media (e.g. blog posts or videos) first.

## Installation
ASPL supports most modern operating systems and easily cross-compiles between them.
<br>Please refer to the <a href="install.md">installation instructions</a> to install ASPL on your system.

## Table of contents
<table>
<tr><td width=33% valign=top>

* [Hello World](#hello-world)
* [Comments](#comments)
* [Function basics](#function-basics)
* [Expression vs. Statement](#expression-vs-statement)
* [Variables](#variables)
* [Type system](#type-system)
    * [Type aliases](#type-aliases)
    * [Nullability](#nullability)
    * [Oftype](#oftype)
* [If statements](#if-statements)
* [While statements](#while-statements)
* [Repeat statements](#repeat-statements)
* [Assertions](#assertions)
* [Strings](#strings)
* [Lists](#lists)
* [Maps](#maps)
* [Functions](#functions)
* [Callbacks](#callbacks)
    * [Closures](#closures)

</td><td width=33% valign=top>

* [Modules](#modules)
* [Namespaces](#namespaces)
* [Visibility modifiers](#visibility-modifiers)
* [Memory management](#memory-management)
* [Order of evaluation](#order-of-evaluation)
* [Error handling](#error-handling)
    * [Catch blocks](#catch-blocks)
    * [Error Propagation](#error-propagation)
    * [Panics](#panics)
* [Classes](#classes)
    * [Static classes](#static-classes)
* [Properties](#properties)
    * [Static properties](#static-properties)
* [Methods](#methods)
    * [Static methods](#static-methods)
* [Inheritance](#inheritance)
* [Parent method calls](#parent-method-calls)
* [Enums](#enums)
    * [Bitfields/Flags](#bitfieldsflags)

</td><td width=33% valign=top>

* [Compiler options](#compiler-options)
    * [Cross-compilation](#cross-compilation)
    * [Debug vs. production builds](#debug-vs-production-builds)
    * [Conditional compilation](#conditional-compilation)
    * [Choosing a backend](#choosing-a-backend)
    * [Stack vs. heaped based builds](#stack-vs-heaped-based-builds)
    * [Choosing a graphics API](#choosing-a-graphics-api)
* [Resource embedding](#resource-embedding)
* [Debugging](#debugging)
    * [Breakpoints](#breakpoints)
* [Implementation calls](#implementation-calls)
* [Android & iOS](#android--ios)
* [Troubleshooting & common issues](#troubleshooting--common-issues)

</td></tr>
</table>

## Hello World
```aspl
print("Hello World!")
```
Save this code to a file and run it with `aspl run file.aspl`.
<br>That's it. You have just written your very first ASPL program! 🎉

## Comments
ASPL supports two styles of comments:

### Single-line comments
If a line starts with `//`, everything after that is ignored by the ASPL parser and treated as a comment.
For example:
```
print("Hello World!") // You can write whatever you want here
// This is a comment too
print("But this is executed again!")
```

### Multi-line comments
If you want to span a comment over multiple lines (very useful for commenting out pieces of code temporarily), you can use `/*` to start a comment and `*/` to end it.
<br>Note that multi-line comments can be nested in ASPL.
```
print("Hello World!")
/*Start of comment
This is a comment
print("This is ignored since it's in a comment")
/*
Nested comment
*/
End of comment*/
print("This is executed again")
```

## Function basics
Functions are a computer science concept inspired by mathematical functions.
<br>They can take an input value, process it and return an output.
<br>Some functions don't return anything though, for example `print` - it only writes the text you gave it to the console.
<br>As already shown above, you can invoke a function like this:
```aspl
myFunction("argument 1", 42, "argument 3") // takes 3 arguments
myOtherFunction() // takes 0 arguments
```
Functions that return something can be used as an expression and their return value can for example be passed to other functions:
```aspl
print(multiply(20 * 20)) // prints 400
```

## Expression vs. Statement
A statement is an instruction to the computer. For example, `while`, `return`, function declarations, ...
<br>An expression, on the other hand, is also like an instruction, but it returns a value, e.g. an access to a variable, mathematical operators, the `this` keyword, ...
<br>Expressions can be passed to functions, assigned to variables, used with operators and so on.
<br>If you want to refer to any instruction, whether it's an expression or a statement, use the term `node`.

## Variables
ASPL is statically typed. That means that all variables are associated with a certain type, e.g. a string or an integer.
<br>You can declare a variable with the `var` keyword, optionally followed by its types, the variable name, and its initial value.
<br>All variables in ASPL have an initial value. If no types are given, the type is inferred from the initial value.
```aspl
var int i = 0
var float f = 1f
var s = "Hello"
```
Updating a variable works like this:
```aspl
var i = 0
print(i)
i = 1
print(i)
```
The output of the above program is:
```aspl
0
1
```
Note that you cannot assign a value of a wrong type to a variable:
```aspl
var s = "Hello"
s = 0 // error, the variable 's' can only hold strings
```

## Type system
ASPL has a very advanced and powerful <a href="https://en.wikipedia.org/wiki/Type_system">type system</a>.
<br>Every expression and everything holding an expression (e.g. a variable) can have either a single type (e.g. literals or simple variables) or of a multitype:
```aspl
var int i = 0 // i can hold values of the type integer (since int is an alias to integer)
var int|float x = 0f // x can hold values of either integer or float, even though it's initialized with a float here
```
If something expects a certain type (e.g. a variable or function parameter), it will fail to compile if any of the types of the expression is incompatible with the expected type:
```aspl
var int|float x = 0f
var string|float y = 0f
x = y // error, y could be of type string, but x cannot hold strings
```

### Type aliases
Type aliases allow to refer to a certain type with another name:
```aspl
var int a = 0
var integer b = 0
a = b // no error, a and b are both of type integer
```
All built-in type aliases are:
<br>`bool` => `boolean`
<br>`int` => `integer`

It is currently not possible to declare custom type aliases.

### Nullability
All types in ASPL are **non-nullable** by default:
```aspl
var int i = 0
i = null // error, i cannot hold null
```

If you want to allow a variable (or property, etc.) to hold null, you can append a `?` to its type:
```aspl
var int? i = 0
i = null // no error, i can hold null
```
Note that the `?` is just a shorthand for `type|null`, so the following is also valid:
```aspl
var int|null i = 0
i = null // no error, i can hold null
```

If you are sure that a value of type `type?` is not null in a certain situation (for example because you checked it before), you can use the `?!` ("null forgiving") operator to force the compiler to treat it as a value of type `type`:
```aspl
var string? s = "Hello"
if(s != null){
    print(s?!.length) // no error, s is assured to not be null here
}
```
Note that the `?!` operator is just a shorthand for a cast to `type`, so the following is also valid:
```aspl
var string? s = "Hello"
if(s != null){
    print(string(s).length) // no error, s is assured to be not null here
}
```

### Oftype
The `oftype` expression checks if a certain expression is of a certain type at runtime:
```aspl
var any x = 0
print(x oftype int) // true
print(x oftype string) // false
```

## If statements
The `if` statement is an essential control flow structure in computer programming. It only executes a certain block of code if a condition is met, i.e. a given expression evaluates to true:
```aspl
if(true){
    print("Will certainly be printed")
}
if(false){
    print("Will certainly not be printed")
}
```
Combined with, for example, variables or user input, this allows a programmer to dynamically react to a program's environment:
```aspl
if(input("Do you like ASPL? ") == "y"){
    print("Great!")
}
```
For the above example, it would also be useful to execute some other code if the condition was not met. For this, there is the `else` keyword that can only follow after an `if` statement:
```aspl
if(input("Do you like ASPL? ") == "y"){
    print("Great!")
}else{
    print("Please let us know how we can improve!")
}
```
Furthermore, if you want to check for multiple possible scenarios, you might want to use `elseif`:
```aspl
var requestedProduct = input("Which product do you want to buy? ")
if(requestedProduct == "apple"){
    print("You successfully bought an apple")
}elseif(requestedProduct == "banana"){
    print("You successfully bought a banana")
}else{
    print("I'm sorry, but we don't offer the product: " + requestedProduct)
}
```

## While statements
Similar to the `if` statement, a `while` statement is a conditional control structure. But instead of executing the code in the body only once, it will be executed as long as the condition evaluates to true:
```aspl
// This never evaluates to true and thus directly skips the body
while(false){
    print("Will never be executed")
}

// Prints all numbers from 0 to 9
var i = 0
while(i < 10){
    print(i)
    i++
}

// This will repeat the body forever
while(true){
    print("Inside infinite loop...")
}
```
There is no `else` for while loops.

## Repeat statements
A `repeat` statement is like a while statement, but instead of taking a condition, it takes an integer specifying how often the code in the body should be repeated:
```aspl
repeat(100){
    print("Repeat...")
}
```
You can also create a variable that will automatically hold the value of the current iteration:
```aspl
repeat(42, i = 0){
    print(i)
}
```
As you can see, you can (and should) even specify the start value of the variable.

## Assertions
Assertions are statements that are used to make sure certain conditions are met. If they are not met, the program will crash and print the location of the failed assertion.
<br>They are used for debugging purposes and can also help in documentation.
```aspl
assert true // ignored, no error
assert false // error, the assertion failed
```

## Strings
A string is simply a sequence of characters, as in most other programming languages. However, ASPL strings are special in two ways:

1. They are immutable, i.e. you cannot change a string after it has been created
```aspl
var s = "Hello"
s[0] = "h" // error, strings are immutable
s = "h" + s.after(0) // this is the correct way to change a string
```
This makes a lot of code much cleaner, less error-prone, and allows for better optimizations.

2. They are natively Unicode, i.e. they can hold any character from any language:
```aspl
var s = "👋"
print(s) // prints 👋
assert s.length == 1
```
> [!WARNING]  
> Native Unicode strings are NOT fully implemented in ASPL yet. For example, the `length` property currently returns the number of bytes, not the number of characters. This is most likely to change in the future.

## Lists
A list contains a variable amount of elements of the same type:
```aspl
var list<string> l = ["Hello", "World", "!"]
print(l) // ["Hello", "World", "!"]
```

You can access an element in the list through indexing:
```aspl
var list<string> l = ["Hello", "World", "!"]
print(l[0]) // Hello
print(l[2]) // !
```
**Note:** Indices start at **0** in ASPL (like in most other languages)!

Adding elements to the list works using the `.add` method:
```aspl
var list<string> l = ["Hello", "World"]
l.add("!")
print(l) // ["Hello", "World", "!"]
```

Removing elements can be either done by value or by index:
```aspl
var list<string> l = ["Hello", "World", "!"]
l.removeAt(0)
l.remove("!")
print(l) // ["World"]
```

## Maps
A map is similar to a [list](#lists), but contains pairs of elements instead of individual elements.
<br>In a map, each pair contains a key and an associated value; you may know this concept from other languages under the terms `dictionary` or `associative array`.
```aspl
var map<string, int> m = {"Universe" => 42}
print(m["Universe"]) // 42
m["zero"] = 0
print(m["zero"]) // 0
m.remove("zero")
print(m) // {"Universe" => 42}
```

## Functions
As already described above, a function is a block of code that accepts certain arguments, which are then stored in special variables called "parameters". Functions also may or may not return a single value.

There are the following built-in functions:
* `print` - write a string to the console and start a new line after that (except if the second parameter is false)
* `input` - optionally print a prompt to the console and then return the user input
* `key` - optionally print a prompt to the console and then return the key the user has pressed
* `range` - return a list containing all numbers between and including the two given arguments
* `exit` - stop the program and return a given exit code to the operating system
* `panic` - print an error message to the console and exit with code 1; see [here](#panics) for more information on panics

Moreover, you, and of course also developers of code libraries, can define their own custom functions.
<br>This makes it possible to split a program into several smaller subroutines and reuse code. Because of this, functions are an essential concept in virtually every programming language today.

```aspl
function hello(){
    print("Hello World!")
}
```
The above code declares a function called `hello` that prints `Hello World!` to the console when executed.
<br>It can be invoked like this:
```aspl
hello()
```

Another example with parameters:
```aspl
function greet(string person){
    print("Hello " + person)
}

greet("Jonas") // Hello Jonas
greet("Marie") // Hello Marie
```

## Callbacks
A callback is an <a href="https://en.wikipedia.org/wiki/Anonymous_function">anonymous function</a>:
```aspl
var greet = callback(string name){
    print("Hello " + name + "!")
}
greet.invoke("John") // Hello John!
// Alternative (and preferred) syntax for invoking a callback
greet.("John") // Hello John!
```
In this case, the callback itself does not have a name, although it is stored inside a variable called `greet`.

### Closures
A callback can also be used as a <a href="https://en.wikipedia.org/wiki/Closure_(computer_programming)">closure</a>.
<br>That means that callbacks can access the variables in their creation scope, even after it has already been popped.
<br>Furthermore, `this` also points to the instance in whose methods the callable was created.
```aspl
function create() returns callback{
    var num = 42
    return callback(){
        print(num)
    }
}
var callback c = create()
c.invoke() // 42
```

## Modules
In ASPL, the term `module` is used for code libraries, i.e. files that contain e.g. functions and classes and are intended to be used together in other applications.
<br>The modules of the standard library (`stdlib`) are automatically shipped with every complete ASPL installation.
<br>Of course, programmers can also create their own third-party modules.

Modules are imported using the `import` statement:
```aspl
import math

print(math.sqrt(9)) // 3
```

Currently, modules do not need any special structure, but they might require a `module.json` or something similar in the future.

You can install modules that are not in the standard library by using the `install` command, for example: `aspl install https://github.com/user/repo`
<br>These modules are stored in the `.aspl_modules` folder in the user's home directory.
<br>The modules of the standard library are located in the `stdlib` folder of the ASPL installation.

## Namespaces
You can organize your code into folders and modules. To access functions and types defined in other folders, you can import them:

`project/folder/hello.aspl`:
```aspl
function hello(){
    print("Hello World!")
}
```

`project/main.aspl`:
```aspl
import folder

folder.hello() // Hello World!
```

Namespaces are always implicit in ASPL and inferred from the folder structure.

If you import a module, you automatically import the main namespace of the module. You can import other folders in the module by prefixing the name of the folder with the name of the module:
```aspl
import encoding.ascii // imports the ascii folder of the encoding module
```

## Visibility modifiers
By default, all symbols, i.e. functions, methods, classes, properties and enums, are **private** in ASPL; this means that
* functions can only be called from within the same module they are defined in *(module bound)*
* classes can only be used from within the same module they are defined in *(module bound)*
    * note that instantiating a class from outside that class is only possible if the constructor is marked as `[public]`
* enums can only be used from within the same module they are defined in *(module bound)*
* properties and methods can only be accessed from within the same class or children of the class they are defined in *(class bound)*

You can export a symbol/make it public by using the `[public]` attribute:
```aspl
[public]
function hello(){
    print("Hello World!")
}
```

There is also the `[readpublic]` attribute, which makes a property read-only from outside the class it is defined in:
```aspl
class Person{

    [readpublic]
    property string name

    method construct(string name){
        this.name = name
    }

}
```
Note that the property can still be modified from within the class; it is **not instance bound**, that means that all instances of the class can modify properties that are readpublic to the class, even if those properties are in a different instance.

## Memory management
ASPL uses a garbage collector to automatically clean up memory after it has been used.
<br>This means that you don't have to remember manually deallocating memory or checking if it has already been freed. It allows for much faster development times and prevents leaks as well as bugs.

## Order of evaluation
> [!WARNING]
> The C backend currently does not ensure a strict left-to-right evaluation order for function/method/callback invocations! This is at the moment considered a bug, although the specification of the argument evaluation order may still change in the future.

Evaluation in ASPL is always left-to-right; while this may seem irrelevant in some situations, it is especially important for boolean operators like `&&` (logical and):

```aspl
var list<int>? l = get_list_or_null()
if(l != null && l?!.length > 0){
    // ...
}
```

In this example, it is crucial that the `l != null` side of the `&&` operator is evaluated before the `l?!.length > 0` side, as `l?!.length` would throw an error if `l` was `null`.

## Error handling
> [!IMPORTANT]
> The error handling mechanisms described here are still experimental. They may contain bugs and are not fully implemented yet, which is why this feature set is currently hidden behind the `-enableErrorHandling` flag.

ASPL uses a variation of the <a href="https://en.wikipedia.org/wiki/Result_type">result type concept</a> to handle errors with some influence from <a href="https://en.wikipedia.org/wiki/Exception_handling">exception handling</a>.

Every function (or method) that can fail has to be marked with the `[throws]` attribute:
```aspl
[throws]
function divide(int a, int b) returns int{
    if(b == 0){
        throw new ArithmeticError("Division by zero")
    }
    return a / b
}
```
As you can see above, the actual error is thrown using the `throw` keyword, similar to exceptions in other languages.

The error thrown here is an instance of a class called `ArithmeticError`. A declaration of this class could look like this:
```aspl
[error]
class ArithmeticError{

    property string message

    method construct(string message){
        this.message = message
    }

    method string() returns string{
        return message
    }

}
```
The `[error]` attribute is used to mark a class as an error, and the `string()` cast method has to be defined to allow the error to be printed by the ASPL runtime.

### Catch blocks
You can catch errors using `catch` blocks (example 1).

If you try to catch an error from an expression inside another expression, you either have to specify a fallback value using the `fallback` keyword (example 2) or escape from the `catch` block and simultaneously return from the function the catch block was defined in using the `escape` keyword (example 3); both work similar to `return`, but "return on a different level", which is why they have been assigned separate keywords and using the regular `return` is not allowed inside `catch blocks`.

```aspl
divide(10, 0) catch error{
    print(error.message)
}
```
```aspl
print(divide(10, 0) catch error{
    print(error.message)
    fallback 0
})
```
```aspl
function reciprocal_squared(int a) returns int{
    var r = divide(1, a) catch error{
        print(error.message)
        escape 0
    }
    return r * r
}
```

### Error Propagation
If you don't want to or cannot handle an error, you can pass it on to the caller:
```aspl
[throws]
function reciprocal(int a) returns int{
    return divide(1, a)! // ! is the error propagation operator
}
```
As you can see, the `!` operator here is used to propagate the error.

Note that this requires the caller to also be marked with the `[throws]` attribute.

### Panics
If an error is so severe that the program cannot continue, you can use the `panic` keyword to stop the execution immediately:
```aspl
function divide(int a, int b) returns int{
    if(b == 0){
        panic("Division by zero")
    }
    return a / b
}
```
Functions that can panic only have to be marked with the `[throws]` attribute if they can also throw an error. Recovering from panics is, at least currently, not possible.

### Defer blocks
If you want to make sure a certain piece of code is executed once a scope is left (via break, return, error propagation or simply after reaching the end of a block), you can use the `defer` statement:
```aspl
function modifyFile(){
    var file = io.open_file("file.txt", "r")
    defer { file.close() }
    // Modify the file somehow
}
```

Note that...
* ...defer blocks will be evaluated in their reverse order of appearance in the code ("reverse order" like opening/closing tags in markdown languages)
* ...when returning a value, the defer blocks are executed after the return value has been evaluated
* ...defer blocks may not use the defer statement in their own body
* ...defer blocks may not use the return statement or throw/propagate any errors
* ...defer blocks may not use the break or continue statements with a level greater than their own loop depth
* ...defer blocks will not be called when the program exits directly using calls to `exit` or `panic` and thus some resources may stay open, but normally the OS will close most of them (e.g. file handles) automatically when the process exits anyway

## Classes
Classes are the base of <a href="https://en.wikipedia.org/wiki/Object-oriented_programming">object-oriented programming</a> in ASPL.
<br>They encapsulate data and make it easily accessible as well as passable.
<br>Classes can have properties and methods.
```aspl
class Hello{

    property string name
    property int number = 5

    method construct(string name){
        print("An instance has been created")
        this.name = name
    }

    method hello(){
        print("Hello from " + name)
        print("My number is: " + number)
    }

}
```
You can instantiate a new instance of the `Hello` class using the `new` keyword:
```aspl
var Hello instance = new Hello("Daniel")
instance.hello()
```
The above program will output:
```
An instance has been created
Hello from Daniel
My number is: 5
```

For information on inheritance in ASPL, see [inheritance](#inheritance).

### Static classes
Static classes are classes that cannot be instantiated and can only contain static properties and methods (see [static properties](#static-properties) and [static methods](#static-methods)):
```aspl
[static]
class Example{

    [static]
    property int number = 42

    [static]
    method hello(){
        print("Hello World!")
    }

}
```
```aspl
Example:number = 5
Example:hello() // Hello World!
```

## Properties
Properties are a class's way of storing data.
<br>They are similar to variables, but bound to an instance of a class instead of a scope:
```aspl
class Example{

    property int number = 42 // default value is 42

}
```
```aspl
var Example a = new Example()
var Example b = new Example()
print(a.number) // 42
print(a.number) // 42
a.number = -1
print(a.number) // -1
print(b.number) // 42
```

### Static properties
Static properties are properties that are not bound to an instance of a class, but to the class itself:
```aspl
class Example{

    [static]
    property int number = 42

}
```
```aspl
print(Example:number) // 42
Example:number = -1
print(Example:number) // -1
```

## Methods
Methods are class-bound functions:
```aspl
import math

class Point{

    property int x
    property int z

    method construct(int x, int z){
        this.x = x
        this.z = z
    }

    method distanceTo(Point other){
        return math.sqrt(math.pow(double(x - other.x), 2d) + math.pow(double(y - other.y), 2d))
    }

}
```
The class `Point` has two methods:

1. `construct`: This is a special method called the `constructor`, which allows you to initialize an instance with arguments; it is automatically called every time a class is instantiated
2. `distanceTo`: This method calculates the distance between the point it is called on and another point (which is passed to it) using the Pythagorean theorem.

### Static methods
Similar to static properties, static methods are methods that act on a class itself instead of an instance of that class and thus can only use static properties instead of regular ones:
```aspl
class Example{

    [static]
    property string name = "Daniel"

    [static]
    method hello(){
        print("Hello " + name)
    }

}
```
```aspl
Example:hello() // Hello Daniel
```

## Inheritance
Classes can inherit properties and methods from other classes using the `extends` keyword:
```aspl	
class Base{

    property int number = 42

    method construct(){
        print("Base class constructor")
    }

}
```
```aspl
class Example extends Base{
}
```
```
var Example e = new Example() // prints "Base class constructor"
print(e.number) // 42
```
ASPL uses multiple inheritance instead of interfaces or traits:
```aspl
class Mammal{

}
```
```aspl
class Pet{

}
```
```aspl
class Dog extends Mammal, Pet{

}
```
```aspl
var Dog d = new Dog()
assert d oftype Mammal
assert d oftype Pet
```
> [!IMPORTANT]
> Multiple inheritance is a tricky concept and generally not very popular amongst other programming languages. While it simplifies a lot of things and can make intuitive sense sometimes, there are also good reasons not to use it. Because of this, ASPL might switch to single inheritance + interfaces/traits in the future, although this is currently not planned.

## Parent method calls
You can explicitly invoke the implementation of a method in certain parent classes even if the method is overridden in the current class using the `parent` keyword; you might also know this concept from other languages as `super`.
```aspl
class Base{

    [public]
    method hello(){
        print("Hello from Base")
    }

}
```
```aspl
class Example extends Base{

    [public]
    method hello(){
        print("Hello from Example")
        parent(Base).hello()
    }

}
```
```aspl
var Example e = new Example()
e.hello()
```
The above program will output:
```
Hello from Example
Hello from Base
```

## Enums
Enums are a way to define a list of constants.
<br>They are defined using the `enum` keyword:
```aspl
enum Color{
    Red,
    Green,
    Blue
}
```
The above code defines an enum called `Color` with three so-called enum fields:
```aspl
Color.Red
Color.Green
Color.Blue
```
The values of the enum fields are assigned automatically, starting at **0**.
<br>You can also specify custom values though:
```aspl
enum Color{
    Red = 1,
    Green = 2,
    Blue = 3
}
```

### Bitfields/Flags
There is a `[flags]` attribute that allows for creating bitflags.
<br>Multiple enum fields can be merged with the `|` operator in these enums.
<br>This is widely used for passing multiple options as one value.

You can check if an enum flag contains a field like this:
```aspl
var Color c = Color.Red || Color.Green
print((c && Color.Red) == Color.Red) // true - the enum flag contains the Red field
```

**Limitations:**
* No field value overriding
* Only up to 32 enum fields

## Compiler options
### Cross-compilation
One of ASPL's main goals has always been seamless cross-compilation and support for as many platforms as possible. In fact, you can simply cross-compile your ASPL code to any other platform using the `-os` and `-arch` flags:
```bash
aspl -os windows -arch amd64 compile hello.aspl
```
The above command will compile `hello.aspl` to a 64-bit Windows executable.

Such a sophisticated cross-compilation toolchain is only possible due to ASPL's unique architecture based on Zig CC (when using the C backend) and a mechanism called "templating" (when using the AIL backend; see the `templates` folder for more information).

Additionally, ASPL uses dynamic loading of system graphics libraries (see the `thirdparty/ssl` folder for more information) to enable cross-compilation of graphical applications; in theory, this can even work on macOS! In practice, this has not been properly implemented yet and might suffer from licensing issues.
<br>This option is currently hidden behind the `-ssl` flag, but will probably soon be on by default.

> [!NOTE]
> Cross-compiling to macOS using the C backend is currently not possible due to issues with the garbage collector. This will hopefully be fixed soon.

### Debug vs. production builds
You can compile your code in production mode using the `-prod` flag:
```bash
aspl -prod compile hello.aspl
```
This will disable all debug features and use optimizations to make your code run faster. Furthermore, the `production` conditional compilation symbol will be defined instead of `debug`.

Usually, you will want to compile your code in debug mode when developing and in production mode when releasing your application.

### Conditional compilation
Like in many other compiled languages, you can make parts of your code only compile in certain situations using the `$if` and `$else` keywords, for example:
```aspl
$if debug{
    print("Debug build")
}$else{
    print("Production build")
}
```
The above code will print `Debug build` when compiling in debug mode and `Production build` when compiling in production mode (using the `-prod` compiler option).

In addition to the `debug` and `production` symbols, there are also the following symbols:
* `windows`/`linux`/`macos`/`android`/`ios`/... - the target operating system
* `amd64`/`arm64`/`arm32`/`rv64`/`rv32`/`i386`/... - the target architecture
* `x32`/`x64` - the bitness of the target architecture (32-bit or 64-bit)
* `console`/`gui` - the target application type (console or GUI); this is only relevant for Windows
* `main` - whether the current file is part of the main module

You can also define your own symbols using the `-d` compiler option:
```bash
aspl -d mySymbol compile hello.aspl
```
```aspl
$if mySymbol{
    print("mySymbol is defined")
}
```

### Choosing a backend
ASPL currently supports two different backends: `ail` and `c`.

The `ail` backend compiles your code to a **bytecode format called `AIL`** (formerly an acronym for "ASPL Intermediate Language") and uses [templating](templates) to bundle the AIL bytecode together with an AIL interpreter into a single executable.

Benefits of the `ail` backend:
* Faster compilation
* Better debugging experience
* Not dependent on a C compiler
* Less bug prone
* Easier to develop for the ASPL team

On the other hand, the `c` backend compiles your code to **C code** and then uses a C compiler to compile it to an executable.

Benefits of the `c` backend:
* Faster execution
* Smaller executable size (sometimes)
* Even better cross-compilation
* Easier C interoperability
* Does not require prebuilt templates

You can choose a backend using the `-backend` compiler option:
```bash
aspl -backend c compile hello.aspl
```

It's generally advised to use the `ail` backend for development/debug builds and the `c` backend for production; note that the `ail` backend is currently the default for all builds.

The different backends are designed to be **as compatible as possible**, so you can easily switch between them, optimally without changing anything in your code (as long as you don't use C interoperability).

### Stack vs. heaped based builds
> [!NOTE]
> This section only applies to the C backend.

> [!WARNING]
> This is a rather sophisticated optimization technique; while it may speed up your program significantially, it's a complicated matter and might cause issues in some cases (although that's a bug and should be reported).

ASPL allocates wrapper C-objects for all ASPL objects created in your program; i.e. if you write `"Hello World"`, it will be translated to something like `ASPL_STRING_LITERAL("Hello World")` (where `ASPL_STRING_LITERAL` is a helper function allocating the actual wrapper object).

These objects can either be allocated on the stack or on the heap, both having their own advantages and disadvantages. Since ASPL is permanently using a GC and the C backend has been carefully designed to support both of these options, you can easily toggle between the different allocation methods.

By default, wrapper objects are allocated on the stack. You can change this using the `-heapBased` compiler flag.

Various performance tests have shown that neither of these methods is per se faster than the other one, so carefully profile your program first and test if switching the allocation method is really worth it.

> [!TIP]
> Some very large programs might experience stack-overflow issues when allocating on the stack; passing `-heapBased` can potentially fix this. Alternatively, you can also try increasing the stack size using the `-stacksize` compiler option (although this is not supported on all platforms).

## Choosing a graphics API
> [!NOTE]
> This section only applies to the C backend and when the target OS is Windows.

By default, the `graphics` stdlib module uses the OpenGL API to communicate drawing calls to Windows - this is primarily done for compatibility reasons. That being said, using Direct3D 11 (D3D11) is preferred for production builds for several reasons, including significantly lower CPU usage and thus energy consumption on some devices (see [here](https://github.com/floooh/sokol/issues/1295)) and no flickering bugs in fullscreen mode. If you are sure that the machine that you want to deploy your ASPL app to has D3D11 installed, consider passing the `-d3d11` flag to the compiler to use the D3D11 interface instead of OpenGL.

## Resource embedding
You can embed resources (such as images, audio files, etc.) directly into your executable using the `$embed` compile-time function:
```aspl
var image = $embed("image.png")
```
In the above example, the `image.png` file will be embedded into the executable and the `image` variable will contain a `list<byte>` containing the raw image data.
You can, for example, then load this image with the graphics module:
```aspl
import graphics

var image = Canvas:fromFileData($embed("image.png"))
img.save("embedded.png")
```

## Debugging
### Breakpoints
> [!NOTE]
> This feature is currently only supported in the AIL backend.

ASPL has a built-in debugger that allows you to step through your code and inspect variables, stack traces, etc.

Just import the `breakpoints` module and use the `break` function to set a breakpoint:
```aspl
import breakpoints

var x = ["a", "b", "c"]
breakpoints.break()
print(x.length)
```

You can also pass a message to the `break` function to make it easier to identify the breakpoint, as it will be printed to the console when the breakpoint is reached:
```aspl
import breakpoints

var x = ["a", "b", "c"]
breakpoints.break("x is initialized")
print(x.length)
```

As soon as the program reaches the breakpoint, the debugger will be started, and you can use several commands such as `stack`, `scope`, `continue` or `quit` to inspect the current state of the program.
<br>For more commands, see the `help` command.

## Implementation calls
An "implementation call" is how ASPL code interacts with native (e.g. C) libraries and thus also the operating system and the actual hardware.

For example, the `graphics` module uses the `graphics.window.show` call to tell the host OS to open a graphical window:
```aspl
implement("graphics.window.show", handle)
```

Note that you will almost never directly use implementation calls inside your codebase; instead, modules wrapping around native libraries (including most stdlib modules) should expose wrapper functions for these implementation calls. For example, this is the code of the `nanosleep` function from the `time` module:
```aspl
[public]
function nanosleep(long nanoseconds){
    implement("time.nanosleep", nanoseconds)
}
```

Implementation calls are a special compiler feature and should not be confused with functions or other types of callables; amongst other things, their return type is always `any`, so you have to manually cast the return value to some other ASPL type:
```aspl
[public]
function abs(string relative) returns string{
    return string(implement("io.path.relative_to_absolute", relative))
}
```
The `implement` keyword also takes an arbitrary amount of arguments - beware that there is no compiler check that validates the arguments in any way; making a mistake here could either lead to compilation errors, runtime crashes or even broken runtime behavior (e.g. memory corruptions).

Also note that the built-in functions (i.e. `print`, `input`, `exit`, ...) are, by definition, no implementation calls, although they behave very similarly. This is simply due to these functions being universally available on nearly every operating system and very frequently used.

More documentation on how to write your own implementation calls will be added soon.

## Android & iOS
Deploying apps to mobile operating systems has traditionally been a horrible experience. Not only because of bad tooling, but also because of the lack of cross-platform GUI toolkits in many programming languages.
<br>Since ASPL's `graphics` module is designed to be platform-agnostic and non-native, most apps written for desktop operating systems will also work on phones, tablets, etc. with little to no adjustments.

ASPL apps can easily be deployed to Android using [ASAPP](https://github.com/aspl-lang/asapp).

iOS support is also planned, but currently not one of the main priorities.

## Troubleshooting & common issues
> [!NOTE]
> This section contains information about the solutions for common problems that might arrise when installing or using parts of the ASPL toolchain. For tips on how to debug your own programs written in ASPL, check out [this chapter](#debugging) of the introduction.

One of the central motivations behind creating ASPL was and still is the ease of its use - because after all, what's better than effortless, beautiful simplicity and that feeling of "it just works"?

Nonetheless, every computer, and every person using one, is different, and even if they weren't, there will always be at least some bugs in a project as big as a programming language. So, if you're facing any issues while installing or using ASPL, you might find some helpful information below:

### `libwinpthread-1.dll` not found
You might receive this error message when running a program compiled using the (default) AIL backend on a computer without a MinGW-w64 installation.
In a nutshell: this is a "bug" in the ASPL toolchain and these libraries should not be required on the target computer. Nonetheless, they currently are needed, so you can fix this issue either by compiling your code using the C backend or by installing [MinGW-w64 **with MCF threads**](https://winlibs.com/#download-release) on the computer where the problem occurs.

### Namespace `aspl.parser`, `aspl.compiler`, ... not found
These error messages are probably due to the fact that you've placed your own ASPL code inside the same folder where your ASPL installation is stored; that causes the `aspl run` command to actually compile the whole compiler alongside your own code, which is not what you want (and since it's also not the proper way to do it, you get these error messages).
So to fix it, simply place your own code somewhere else and try again; note that you might want to add the `aspl` command to your PATH in that case (if you haven't done so already).