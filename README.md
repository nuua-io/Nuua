# Nuua Programming Language

This is the official nuua programming language repository

Nuua is a new high level programming language. It acts as a generic language and it's built using pure C++, no dependencies required.
Nuua is ~~dynamic~~ static typed and interpreted language similar to other high level languages like JavaScript or Python.

The syntax is familiar with no strange structures. However, some syntax sugar is added to perform common tasks quicker.

## Nuua website

Nuua homepage is <https://nuua.io>. You may subscribe to recieve further information.

## Nuua Roadmap

- ~~From dynamic to static~~ :white_check_mark:
- ~~Stack machine to Register Machine~~ :white_check_mark:
- ~~Re-write the syntax~~ :white_check_mark:
- ~~Better error reporting~~ :white_check_mark:
- ~~Implement modules (imports)~~ :white_check_mark:
- Implement functions :construction:
- Implement classes :construction:
- Implement function overloading (?)
- NUSL (Nuua Standard Library)
- NUPAC (Nuua Package Manager)
- Optimizations
- **Initial Release**

## How is nuua structured?

Nuua is structured as a layered system. It have diferent tiers (ordered):

- Application
- Virtual Machine
- Compiler
- Analyzer
- Parser
- Lexer

Nuua also have independent modules to help with it's development on every layer.
In the case of the current state, nuua have the following modules:

- Logger

![Layered System](https://i.imgur.com/vNaPz7x.png)

## Building Nuua

Currently, nuua is under **heavy** development meaning it still provides no official release.
However, you're free to build the source yourself. The required software is:

- MinGW + Visual Studio (in windows)
- A C++ compiler (I use g++: *gcc version 8.2.0 (Rev3, Built by MSYS2 project)*)
- CMake

To build the language you just need to use Cmake in the root directory.
For example, create a directory called `my_build` in the root directory
and then inside of it invoke Cmake `cmake ..`. Then you may build it using the
selected target (Makefile, Vistual Studio, etc).
