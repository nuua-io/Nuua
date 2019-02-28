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
    // If const_opcode is true and the expression is a constant expression, it will move it
    // to a new register. Otherwise, it will just add the constant and the constant index.
    // suggested_register will use that register as the result.
    uint64_t compile(Expression *rule, bool const_opcode = true, uint64_t *suggested_register = nullptr);
    // Adds an opcode the the currently used memory.
    void add_opcode(uint64_t opcode);
    // Adds an opcode X times.
    void add_opcodes(uint64_t opcode, uint8_t times);
    // Adds a constant to the code.
    uint64_t add_constant_only(Value value);
    // Modifies a literal in the code given it's index.
    void modify_literal(size_t index, uint64_t value);
    // Returns the currently used memory.
    Memory *get_current_memory();
    // Returns the current line of the code.
    uint64_t current_code_line();
    // Returns The first found variable in the block stack.
    BlockVariableType *get_variable(std::string &name);
    // Determines if the following expression is a safe constant.
    bool is_constant(Expression *expression);
    public:
        // Compile an input source and returns the result program.
        Program compile(const char *source);
};

#endif
