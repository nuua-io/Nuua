/**
 * |------------------------|
 * | Nuua Program Structure |
 * |------------------------|
 *
 * Copyright 2019 Erik Campobadal <soc@erik.cat>
 * https://nuua.io
 */
#ifndef PROGRAM_HPP
#define PROGRAM_HPP

#include "value.hpp"
#include <vector>
#include <unordered_map>
#include <stdint.h>

// Defines the known opcodes for the program.
typedef enum : uint8_t {
    OP_CONSTANT,

    // Unary operations
    OP_MINUS, OP_NOT,

    // Binary operations
    OP_ADD, OP_SUB, OP_MUL, OP_DIV,
    OP_EQ, OP_NEQ, OP_LT, OP_LTE,
    OP_HT, OP_HTE,

    // Jumps and conditional jumps
    OP_JUMP, OP_RJUMP, OP_BRANCH_TRUE, OP_BRANCH_FALSE,

    // Store and load
    OP_STORE, OP_LOAD, OP_STORE_ACCESS,

    // Lists and dictionaries
    OP_LIST, OP_DICTIONARY, OP_ACCESS,

    // Functions
    OP_FUNCTION, OP_CALL,

    // Others
    OP_LEN, OP_PRINT, OP_EXIT
} OpCode;

// Defines the basic memories that exist in the program.
typedef enum : uint8_t {
    PROGRAM_MEMORY, FUNCTIONS_MEMORY, CLASSES_MEMORY
} MemoryType;

class Value;

// Defines a basic memory for nuua.
class Memory
{
    public:
        // This stores the opcodes and consant indexes of the code.
        std::vector<uint64_t> code;

        // Stores the value constants.
        std::vector<Value *> constants;

        // Stores the lines corresponding to the opcodes.
        std::vector<uint32_t> lines;

        // Dumps the memory.
        void dump();

        // Reset the memory.
        void reset();
};

// A frame is the one responsible for storing variables in a program.
class Frame
{
    public:
        std::unordered_map<std::string, Value *> heap;
        uint64_t *return_address;
};

// The base program class that represents a nuua program.
class Program
{
    public:
        // Stores the program memory (The main code).
        Memory program;

        // Stores the code regarding to functions.
        Memory functions;

        // Stores the code regarding to classes.
        Memory classes;

        // Resets the whole program memory.
        void reset();
};

// Basic conversation from opcode to string.
std::string opcode_to_string(uint64_t opcode);

// Prints a given opcode to the screen.
void print_opcode(uint64_t opcode);

#endif
