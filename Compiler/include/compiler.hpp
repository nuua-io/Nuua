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
    void add_constant_only(Value *value);

    public:
        Program program;

        Program compile(const char *source);
};

#endif
