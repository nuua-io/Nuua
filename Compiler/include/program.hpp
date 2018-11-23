#ifndef PROGRAM_HPP
#define PROGRAM_HPP

#include "value.hpp"
#include <vector>
#include <unordered_map>
#include <stdint.h>

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

typedef enum : uint8_t {
    PROGRAM_MEMORY, FUNCTIONS_MEMORY, CLASSES_MEMORY
} MemoryType;

class Value;

class Memory
{
    public:
        std::vector<uint64_t> code;
        std::vector<Value *> constants;
        std::vector<uint32_t> lines;

        void dump();
};

class Frame
{
    public:
        std::unordered_map<std::string, Value *> heap;
        uint64_t *return_address;
};

class Program
{
    public:
        Memory program;
        Memory functions;
        Memory classes;
};

std::string opcode_to_string(uint64_t opcode);
void print_opcode(uint64_t opcode);

#endif
