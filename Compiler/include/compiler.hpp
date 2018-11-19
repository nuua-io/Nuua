#ifndef COMPILER_HPP
#define COMPILER_HPP

#include "program.hpp"
#include "../../Parser/include/rules.hpp"

class Compiler
{
    void compile(Statement *rule);
    void add_opcode(OpCode opcode, uint8_t memory);
    void add_constant(Value value, uint8_t memory);

    public:
        Program program;

        void compile(const char *source);
};

#endif
