#ifndef COMPILER_HPP
#define COMPILER_HPP

#include "program.hpp"
#include "../../Parser/include/rules.hpp"

class Compiler
{
    uint32_t current_line;

    void compile(Statement *rule);
    void add_opcode(OpCode opcode, MemoryType memory);
    void add_constant(Value value, MemoryType memory);

    public:
        Program program;

        Program compile(const char *source);
};

#endif
