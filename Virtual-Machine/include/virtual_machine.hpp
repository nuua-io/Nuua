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
#define MEMORY_SIZE 256

class VirtualMachine
{
    // The program the virtual machine is going to run.
    Program program;

    // The current instruction to execute.
    uint64_t *program_counter = nullptr;

    // The value stack to perform operations (it's a stack based virtual machine).
    Value stack[STACK_SIZE];

    // The top of the stack.
    Value *top_stack = this->stack;

    // The frame list (latest is the current).
    Frame frames[FRAME_SIZE] = { Frame() };

    // The top frame (current frame).
    Frame *top_frame = this->frames;

    // Stores the stack of the memories used.
    MemoryType memories[MEMORY_SIZE] = { PROGRAM_MEMORY };

    // The current memory where the program counter is pointing.
    MemoryType *current_memory = this->memories;

    // Push a new value to the stack.
    void push(Value value);

    // Pops and returns a value from the stack.
    Value *pop();

    // Helper to perform the OP_LIST.
    void do_list();

    // Helper to perform OP_DICTIONARY.
    void do_dictionary();

    // Helper to perform OP_ACCESS.
    void do_access();

    // Helper to perform OP_DECLARE.
    void do_declare();

    // Helper to perform OP_RETURN.
    void do_return();

    // Helper to perform OP_CALL.
    void do_call();

    // Returns true if the value has been declared.
    bool variable_declared(std::string name);

    // Loads a variable or fails with an error.
    Value load_variable(std::string name);

    // Stores a value
    void store_variable(std::string name, Value *new_value);

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
