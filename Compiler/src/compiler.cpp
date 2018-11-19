#include "../include/compiler.hpp"
#include "../../Parser/include/parser.hpp"
#include "../../Logger/include/logger.hpp"

#define PROGRAM_MEMORY 0
#define FUNCTIONS_MEMORY 1
#define CLASSES_MEMORY 2

void Compiler::add_opcode(OpCode opcode, uint8_t memory)
{

}

void Compiler::compile(const char *source)
{
    auto parser = new Parser;
    auto structure = parser->parse(source);
    delete parser;

    logger->info("Started compiling...");

    for (auto node : structure) this->compile(node);
    this->add_opcode(OP_EXIT, PROGRAM_MEMORY);

    logger->success("Compiling completed");
}

void Compiler::compile(Statement *rule)
{
    switch (rule->rule) {

    }
}

#undef PROGRAM_MEMORY
#undef FUNCTIONS_MEMORY
#undef CLASSES_MEMORY
