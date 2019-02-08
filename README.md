# Nuua Programming Language

This is the official nuua programming language repository

Nuua is a new high level programming language. It acts as a generic language and it's built using pure C++, no dependencies required.
Nuua is ~~dynamic~~ static typed and interpreted language similar to other high level languages like JavaScript or Python.

The syntax is familiar with no strange structures. However, emphasis on arrows and anonymous functions (also called arrow functions)
is there to stay.

## Nuua website

Nuua homepage is <https://nuua.io>. You may subscribe to recieve further information.

## Nuua Roadmap

- ~~From dynamic to static~~ :white_check_mark:
- ~~Implement functions~~ :white_check_mark:
- Polish internals :construction:
    - Add a proper tree walker and use it for the compiler & AST optimizator
    - Optimize dead code elimination using OP_ONLY_STORE opcode
- Implement classes
- Implement function overloading
- Implement modules (imports)
- NSL (Nuua Standard Library)
- NPM (Nuua Package Manager)
- Optimizations + Better error reporting
- **Initial Release**

The following are wishes, not yet decided how or when.

- Cross device and cross platform compilation (yep, that's it)
    -   Android (Dart)
    -   iOS (Dart)
    -   Windows (C++)
    -   Linux (C++)
    -   MacOS (C++)
    -   Web (Javascript)
- **Second Release**
- Nuua UI (Cross device & cross platform User Interface)
    -   Android (Dart Widgets)
    -   iOS (Dart Widgets)
    -   Windows (libui)
    -   Linux (libui)
    -   MacOS (libui)
    -   Web (Vue / React)
- **Third Release**

## How is nuua structured?

Nuua is structured as a layered system. It have diferent tiers (ordered):

- [Application](https://github.com/nuua-io/Application)
- [Virtual Machine](https://github.com/nuua-io/Virtual-Machine)
- [Compiler](https://github.com/nuua-io/Compiler)
- [Parser](https://github.com/nuua-io/Parser)
- [Lexer](https://github.com/nuua-io/Lexer)

Nuua also have independent modules to help with it's development on every layer.
In the case of the current state, nuua have the following modules:

- [Logger](https://github.com/nuua-io/Logger)

![Layered System](https://i.imgur.com/vNaPz7x.png)

### How are these repositories linked with this one?

They are linked using git subtrees. I mainly develop nuua in this repository and then further push all contents to each one,
ensuring a distributed system.

## Building Nuua

Currently, nuua is under **heavy** development meaning it still provides no official release.
However, you're free to build the source yourself. The required software is:

- MinGW + Visual Studio (in windows)
- A C++ compiler (I use g++: *gcc version 8.2.0 (Rev3, Built by MSYS2 project)*)
- GNU makefile (You can also use it on windows by using mingw32-make)

To build the language you just need to invoke the `make` command.
The executable file will be inside the bin folder and will be named `nuua` (.exe in windows)

You may use `make clean` to remove the `*.o` files in the build directory.
