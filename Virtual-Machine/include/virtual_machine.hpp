/**
 * |----------------------------------|
 * | Nuua Stack Based Virtual Machine |
 * |----------------------------------|
 *
 * Copyright 2019 Erik Campobadal <soc@erik.cat>
 * https://nuua.io
 */
#ifndef VIRTUAL_MACHINE_HPP
#define VIRTUAL_MACHINE_HPP

#include "../../Compiler/include/program.hpp"

#define STACK_SIZE 256
#define FRAME_SIZE 256

class VirtualMachine
{
    // The program the virtual machine is going to run.
    Program program;

    // The current instruction to execute.
    uint64_t *program_counter;

    // The value stack to perform operations (it's a stack based virtual machine).
    Value *stack[STACK_SIZE];

    // The top of the stack.
    Value **top_stack;

    // The frame list (latest is the current).
    Frame frames[FRAME_SIZE] = { Frame() };

    // The top frame (current frame).
    Frame *top_frame;

    // The current memory where the program counter is pointing.
    MemoryType current_memory = PROGRAM_MEMORY;

    // Helper to perform the OP_LIST.
    void do_list();

    // Helper to perform OP_DICTIONARY.
    void do_dictionary();

    // Helper to perform OP_ACCESS.
    void do_access();

    // Returns the current used memory.
    Memory *get_current_memory();

    // Returns the current executing line.
    uint32_t get_current_line();

    // Runs the virtual machine.
    void run();

    public:
        // It runs the virtual machine given a source input.
        void interpret(const char *source);

        // Resets the virtual machine program memories.
        void reset();
};

#endif
