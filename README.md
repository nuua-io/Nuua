# Nuua Programming Language

This is the official nuua programming language repository

Nuua is a new high level programming language. It acts as a generic language and it's built using pure C++, no dependencies required.
Nuua is an dynamic and interpreted language similar to other high level languages like JavaScript or Python.

The syntax is familiar with no strange structures. However, emphasis on arrows and anonymous functions (also called arrow functions)
is there to stay.

## Nuua website

Nuua homepage is <https://nuua.io>. You may subscribe to recieve further information.

## How is nuua structured?

Nuua is structured as a layered system. It have diferent tiers (ordered):

- [Virtual Machine](https://github.com/nuua-io/Virtual-Machine)
- [Compiler](https://github.com/nuua-io/Compiler)
- [Parser](https://github.com/nuua-io/Parser)
- [Lexer](https://github.com/nuua-io/Lexer)

Nuua also have independent modules to help with it's development on every layer.
In the case of the current state, nuua have the following modules:

- [Logger](https://github.com/nuua-io/Logger)

### How are these repositories linked with this one?

They are linked using git subtrees. I mainly develop nuua in this repository and then further push all contents to each one,
ensuring a distributed system.

## Building Nuua

Currently, nuua is under **heavy** development meaning it still provides no official release.
However, you're free to build the source yourself. The required software is:

- MinGW + Visual Studio (in windows)
- A C++ compiler (I use g++: *gcc version 8.2.0 (Rev3, Built by MSYS2 project)*)
- GNU makefile (You can also use it on windows by using mingw32-make)

To build the language you just need to invoke the `make nuua` command. The build directory will then be
populated with a few `*.o` objects. The executable file will be inside the bin folder and will be named `nuua` (.exe in windows)

You may use `make clean` to remove the `*.o` files in the build directory.
