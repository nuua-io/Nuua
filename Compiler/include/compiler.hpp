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
#include "../../Parser/include/rules.hpp"

// Base compiler class for nuua.
class Compiler
{
    // Stores the current compilation line (regarding to the original source file).
    uint32_t current_line = 0;

    // Stores the currently used memory where compilation result is going.
    MemoryType current_memory = PROGRAM_MEMORY;

    // Defines a basic compilation for a Statement.
    void compile(Statement *rule);

    // Defines a basic compilation for an Expression.
    void compile(Expression *rule);

    // Defines a basic compilation for an operation.
    void compile(Token op, bool unary = false);

    // Adds an opcode the the currently used memory.
    void add_opcode(OpCode opcode);

    // Adds a constant with it's respective OP_CONSTANT opcode.
    void add_constant(Value *value);

    // Adds a constant without it's OP_CONSTANT.
    uint64_t add_constant_only(Value *value);

    // Modifies a constant given it's index in the current memory to the given value.
    void modify_constant(uint64_t index, Value *value);

    // Returns the currently used memory.
    Memory *get_current_memory();

    // Returns the current line of the code.
    uint32_t current_code_line();

    public:
        // Stores the program itself where everything is beeing compiled to.
        Program program;

        // Compile an input source and returns the result program.
        Program compile(const char *source);
};

#endif
