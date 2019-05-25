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

#include "../../Compiler/include/compiler.hpp"

#define MAX_FRAMES 1024
#define STACK_SIZE 1024

class VirtualMachine
{
    // Stores the program to be executed.
    std::shared_ptr<Program> program = std::make_shared<Program>();
    // Virtual machine program counter.
    opcode_t *program_counter;
    // Stores the virtual machine stack (used to push function arguments / get return values)
    Value stack[STACK_SIZE];
    // Indicates the top of the stack.
    Value *top_stack = this->stack;
    // Stores the current frame stack.
    Frame frames[MAX_FRAMES];
    // Indicates the top level frame.
    Frame *active_frame = this->frames - 1; // It performs a pre-increment when a call is done.
    // Runs the virtual machine.
    void run();
    public:
        // It runs the virtual machine given a source input.
        void interpret(const char *file);
        // Resets the virtual machine program memories.
        void reset();
};

#endif
