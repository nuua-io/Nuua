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

class Compiler
{
    uint32_t current_line = 0;
    MemoryType current_memory = PROGRAM_MEMORY;

    void compile(Statement *rule);
    void compile(Expression *rule);
    void compile(Token op, bool unary = false);
    void add_opcode(OpCode opcode);
    void add_constant(Value *value);
    void modify_constant(uint64_t index, Value *value);
    Memory *get_current_memory();
    uint64_t add_constant_only(Value *value);
    uint32_t current_code_line();

    public:
        Program program;

        Program compile(const char *source);
};

#endif
