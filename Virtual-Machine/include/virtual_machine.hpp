/**
 * |-------------------------------------|
 * | Nuua Register Based Virtual Machine |
 * |-------------------------------------|
 *
 * Copyright 2019 Erik Campobadal <soc@erik.cat>
 * https://nuua.io
 */
#ifndef VIRTUAL_MACHINE_HPP
#define VIRTUAL_MACHINE_HPP

#include "../../Compiler/include/program.hpp"

#define MAX_FRAMES 1024
#define MEMORY_SIZE 1024

class VirtualMachine
{
    // Stores the program to be executed.
    Program program;
    // Virtual machine program counter.
    uint64_t *program_counter;
    // Stores the stack of the memories used.
    MemoryType memories[MEMORY_SIZE] = { PROGRAM_MEMORY };
    // The current memory where the program counter is pointing.
    MemoryType *current_memory = this->memories;
    // Returns the current used memory.
    Memory *get_current_memory();
    // Stores the current frame stack.
    Frame frames[MAX_FRAMES];
    // Indicates the top level frame.
    Frame *top_frame;
    // Runs the virtual machine.
    void run();
    public:
        // It runs the virtual machine given a source input.
        void interpret(const char *source);
        // Resets the virtual machine program memories.
        void reset();
};

#endif
