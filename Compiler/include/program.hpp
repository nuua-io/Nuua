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
#include "opcodes.hpp"
#include <vector>

// Defines the basic memories that exist in the program.
typedef enum : uint8_t {
    PROGRAM_MEMORY, FUNCTIONS_MEMORY, CLASSES_MEMORY
} MemoryType;

// Defines a basic memory for nuua.
class Memory
{
    public:
        // This stores the opcodes and consant indexes of the code.
        std::vector<uint64_t> code;
        // Stores the value constants.
        std::vector<Value> constants;
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
    uint32_t in;

    public:
        // Stores the used registers.
        Value *registers = nullptr;
        // Stores the registers size.
        uint32_t registers_size;
        // Stores the return address to get back to the original program counter.
        uint64_t *return_address = nullptr;
        // Stores the frame caller (the function)
        Value *caller;
        // Allocates the space to store the registers.
        void allocate_registers(uint32_t registers_size);
        // Frees the allocated register space.
        void free_registers();
        Frame(uint32_t registers_size = 0);
};

// This class is used to represent the frame information during compilation
// (not in runtime).
class FrameInfo
{
    // Stores the registers that are free to use again.
    std::vector<uint32_t> free_registers;
    // Stores the registers that are protected to get free unless forçed.
    std::vector<uint32_t> protected_registers;
    public:
        // Stores the next register to give in case no free ones are available.
        uint32_t current_register = 0;
        // Method to return an available register.
        // It will try to get it from the free_registers
        // otherwise it will return a new register and
        // increment current_register by 1
        uint32_t get_register(bool protect = false);
        // Used to free a register.
        void free_register(uint32_t reg, bool force = false);
};

// The base program class that represents a nuua program.
class Program
{
    public:
        // Stores the number of registers used by the main frame.
        uint32_t main_registers;
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
