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
    Program program;
    uint64_t *program_counter;
    std::vector<Value *> stack;
    std::vector<Frame> frames;
    MemoryType current_memory = PROGRAM_MEMORY;

    void push(Value *value);
    Value *pop();

    uint64_t read_instruction();
    Value *read_constant();
    int read_integer();
    std::string read_variable();

    void do_list();
    void do_dictionary();
    void do_access();
    Memory *get_current_memory();
    uint32_t get_current_line();

    void run();

    public:
        void interpret(const char *source);

        // Resets the virtual machine program memories.
        void reset();
};

#endif
