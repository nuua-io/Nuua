#include "../include/compiler.hpp"
#include "../../Parser/include/parser.hpp"
#include "../../Logger/include/logger.hpp"

void Compiler::add_opcode(OpCode opcode, MemoryType memory)
{
    Memory mem;

    switch (memory) {
        case PROGRAM_MEMORY: { mem = this->program.program; break; }
        case FUNCTIONS_MEMORY: { mem = this->program.functions; break; }
        case CLASSES_MEMORY: { mem = this->program.classes; break; }
    }

    mem.code.push_back(opcode);
    mem.lines.push_back(this->current_line);
}

Program Compiler::compile(const char *source)
{
    auto parser = new Parser;
    auto structure = parser->parse(source);
    delete parser;

    logger->info("Started compiling...");

    for (auto node : structure) this->compile(node);
    this->add_opcode(OP_EXIT, PROGRAM_MEMORY);

    logger->success("Compiling completed");

    return this->program;
}

void Compiler::compile(Statement *rule)
{
    switch (rule->rule) {
        default: {}
    }
}
