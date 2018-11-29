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

class VirtualMachine
{
    // The program the virtual machine is going to run.
    Program program;

    // The current instruction to execute.
    uint64_t *program_counter;

    // The value stack to perform operations (it's a stack based virtua machine).
    std::vector<Value *> stack;

    // The frame list (latest is the current).
    std::vector<Frame> frames = { Frame() };

    // The current memory where the program counter is pointing.
    MemoryType current_memory = PROGRAM_MEMORY;

    // Push a new value to the stack.
    void push(Value *value);

    // Pop the latest value in the stack and returns it.
    Value *pop();

    // Reads the next instruction to execute.
    uint64_t read_instruction();

    // Read the next instruction as a constant value.
    Value *read_constant();

    // read the next instruction as an integer.
    int read_integer();

    // Read the next instruction as a string.
    std::string read_variable();

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
