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

typedef struct {
    uint64_t instruction; // Instruction to execute
    uint64_t op1; // First operant
    uint64_t op2; // Second operant
    uint64_t op3; // Third operant
} Instruction;

class VirtualMachine
{
    // Stores the program to be executed.
    std::shared_ptr<Program> program;
    // Virtual machine program counter.
    Instruction *program_counter = nullptr;
    // Stores the current frame stack.
    Frame frames[MAX_FRAMES];
    // Indicates the top level frame.
    Frame *top_frame = this->frames;
    // Runs the virtual machine.
    void run();
    public:
        // It runs the virtual machine given a source input.
        void interpret(const char *file);
        // Resets the virtual machine program memories.
        void reset();
        VirtualMachine();
};

#endif
