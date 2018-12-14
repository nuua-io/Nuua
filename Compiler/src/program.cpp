/**
 * |------------------------|
 * | Nuua Program Structure |
 * |------------------------|
 *
 * Copyright 2019 Erik Campobadal <soc@erik.cat>
 * https://nuua.io
 */
#include "../include/program.hpp"
#include "../../Logger/include/logger.hpp"

static auto opcode_names = std::vector<std::string>({
    "OP_CONSTANT",

    // Unary operations
    "OP_MINUS", "OP_NOT",

    // Binary operations
    "OP_ADD", "OP_SUB", "OP_MUL", "OP_DIV",
    "OP_EQ", "OP_NEQ", "OP_LT", "OP_LTE",
    "OP_HT", "OP_HTE",

    // Jumps and conditional jumps
    /*"OP_JUMP",*/ "OP_RJUMP", "OP_BRANCH_TRUE", "OP_BRANCH_FALSE",

    // Store and load
    "OP_DECLARE", "OP_STORE", "OP_LOAD", "OP_STORE_ACCESS",

    // Lists and dictionaries
    "OP_LIST", "OP_DICTIONARY", "OP_ACCESS",

    // Functions
    "OP_FUNCTION", "OP_CALL",

    // Others
    "OP_LEN", "OP_PRINT", "OP_EXIT"
});

void Memory::dump()
{
    printf("Memory Dump: (size: %zu)\n",  this->code.size());
    for (uint64_t i = 0; i < this->code.size(); i++) {
        auto opcode = this->code[i];
        if (
            opcode == OP_CONSTANT
            || opcode == OP_LOAD
            || opcode == OP_STORE
            || opcode == OP_BRANCH_FALSE
            || opcode == OP_BRANCH_TRUE
            || opcode == OP_RJUMP
            || opcode == OP_ACCESS
            || opcode == OP_LIST
            || opcode == OP_DICTIONARY
            || opcode == OP_STORE_ACCESS
            || opcode == OP_FUNCTION
            || opcode == OP_CALL
            || opcode == OP_DECLARE
        ) {
            // It's a constant load
            print_opcode(this->code[i++]);
            printf(" ");
            this->constants[this->code[i]].print();
            if (opcode == OP_DECLARE) {
                printf(" ");
                this->constants[this->code[++i]].print();
            }
            printf("\n");
            continue;
        }
        print_opcode(this->code[i]);
        printf("\n");
    }

    printf("\nCONSTANTS: (size: %zu)\n", this->constants.size());
    for (auto c : this->constants) {
        c.print();
        printf("\n");
    }
    printf("\nLITERAL OPCODE NUMBERS: (size: %zu)\n", this->code.size());
    for (auto c : this->code) printf("%llu ", c);
    printf("\n");
}

void Memory::reset()
{
    this->code.clear();
    this->constants.clear();
    this->lines.clear();
}

void Program::reset()
{
    this->program.reset();
    this->functions.reset();
    this->classes.reset();
}

std::string opcode_to_string(uint64_t opcode)
{
    if (opcode > (opcode_names.size() - 1)) {
        logger->warning("Opcode: " + std::to_string(opcode) + "Cannot be converted to string. Make sure you have a correct opcode number");
        return "";
    }

    return opcode_names[opcode];
}

void print_opcode(uint64_t opcode)
{
    printf("%s", opcode_to_string(opcode).c_str());
}
