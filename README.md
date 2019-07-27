# Nuua Programming Language

This is the official nuua programming language repository

Nuua is a new high level programming language. It acts as a generic language and it's built using pure C++, no dependencies required.
Nuua is ~~dynamic~~ static typed and interpreted language similar to other high level languages like JavaScript or Python.

The syntax is familiar with no strange structures. However, some syntax sugar is added to perform common tasks quicker.

## Nuua website

Nuua homepage is <https://nuua.io>. You may subscribe to recieve further information.

## Code examples

-   Calculate the area of a triangle (with some OOP programming).
    ```cpp
    class Triangle {
        b: float
        h: float
        fun area(): float -> (self.b * self.h) / 2.0
    }

    fun main(argv: [string]) {
        t := Triangle!{b: 10.0, h: 5.0}
        print "The area is: " + t.area() as string
    }
    ```

- Calculate nth Fibonacci number (using recursion).

    ```cpp
    fun rec_fib(n: int): int {
        if n < 2 => return n
        return rec_fib(n - 2) + rec_fib(n - 1)
    }

    fun main(argv: [string]) {
        print rec_fib(25)
    }
    ```

- High level programming.

    ```cpp
    use list_int_map from "list"

    class Collection {
        numbers: [int]
        fun map(f: (int -> int)): Collection {
            list_int_map(self.numbers, f)
            return self
        }
    }

    fun multiply(n: int): int -> n * 2

    fun main(argv: [string]) {
        c := Collection!{numbers: [1, 2, 3, 4, 5]}
        c.map(multiply).map(multiply)
        print c.numbers
    }

    ```

## Nuua Roadmap

- ~~From dynamic to static~~ :white_check_mark:
- ~~Stack machine to Register Machine~~ :white_check_mark:
- ~~Re-write the syntax~~ :white_check_mark:
- ~~Better error reporting~~ :white_check_mark:
- ~~Implement modules (imports)~~ :white_check_mark:
- ~~Implement functions~~ :white_check_mark:
- ~~Implement classes~~ :white_check_mark:
- Implement function overloading :clock1:
- ~~Minimal NUSL (Nuua Standard Library)~~ :white_check_mark:
- NUPAC (Nuua Package Manager) :clock1:
- Optimizations :warning:
- **Initial Release** :construction:

## How is nuua structured?

Nuua is structured as a layered system. It have diferent tiers (ordered):

- Application
- Virtual Machine
- Compiler (Code Generator)
- Analyzer (Semantic analyzer)
- Parser
- Lexer

Nuua also have independent modules to help with it's development on every layer.
In the current state, nuua have the following independent modules:

- Logger

![Layered System](https://i.imgur.com/wFQHGnp.png)

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
