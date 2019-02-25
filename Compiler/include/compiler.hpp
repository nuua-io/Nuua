/**
 * |---------------|
 * | Nuua Compiler |
 * |---------------|
 *
 * Copyright 2019 Erik Campobadal <soc@erik.cat>
 * https://nuua.io
 */
#ifndef COMPILER_HPP
#define COMPILER_HPP

#include "program.hpp"
#include "../../Analyzer/include/analyzer.hpp"

// Base compiler class for nuua.
class Compiler
{
    // Stores the program itself where everything is beeing compiled to.
    Program program;
    // Stores the currently used memory where compilation result is going.
    MemoryType current_memory = PROGRAM_MEMORY;
    // Stores the current compilation line (in the source code).
    uint32_t current_line;
    // Stores the current block.
    std::vector<Block*> blocks;
    // Stores the frame information.
    std::vector<FrameInfo> frame_info = { FrameInfo() };
    // Defines a basic compilation for a Statement.
    void compile(Statement *rule);
    // Defines a basic compilation for an Expression. (Returns the register with the result).
    uint32_t compile(Expression *rule);
    // Adds an opcode the the currently used memory.
    void add_opcode(uint64_t opcode);
    // Adds a constant to the code.
    uint64_t add_constant_only(Value value);
    // Modifies a constant given it's index in the current memory to the given value.
    void modify_constant(uint64_t index, Value value);
    // Returns the currently used memory.
    Memory *get_current_memory();
    // Returns the current line of the code.
    uint32_t current_code_line();
    public:
        // Compile an input source and returns the result program.
        Program compile(const char *source);
};

#endif
