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
#include <algorithm>

#define MAX_OPERANDS 3

static std::string opcode_names[] = {
    "OP_LOAD", // LOAD RX CX
    "OP_MOVE", // MOVE RX RY
    "OP_ADD", // ADD RX RY RZ
    "OP_PRINT", // PRINT RX
    "OP_EXIT" // EXIT
};

typedef enum : uint8_t {
    OPERAND_NONE, OPERAND_REGISTER, OPERAND_CONSTANT
} OperandType;
static uint8_t opcode_operands[][MAX_OPERANDS] = {
    { OPERAND_REGISTER, OPERAND_CONSTANT, OPERAND_NONE },
    { OPERAND_REGISTER, OPERAND_REGISTER, OPERAND_NONE },
    { OPERAND_REGISTER, OPERAND_REGISTER, OPERAND_REGISTER },
    { OPERAND_REGISTER, OPERAND_NONE, OPERAND_NONE },
    { OPERAND_NONE, OPERAND_NONE, OPERAND_NONE },
};

void Memory::dump()
{
    for (size_t i = 0; i < this->code.size(); i++) {
        auto el = this->code[i];
        printf(opcode_names[el].c_str());
        for (uint8_t k = 0; k < MAX_OPERANDS && opcode_operands[el][k] != OPERAND_NONE; k++) {
            switch (opcode_operands[el][k]) {
                case OPERAND_REGISTER: { printf(" R%llu", this->code[++i]); break; }
                case OPERAND_CONSTANT: {
                    printf(" <%s>", this->constants[this->code[++i]].to_string().c_str());
                    break;
                }
                default: { /* Ignore */ }
            }
        }
        printf("\n");
    }
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
    return opcode_names[opcode];
}

void print_opcode(uint64_t opcode)
{
    printf("%s", opcode_to_string(opcode).c_str());
}

void Frame::allocate_registers(uint32_t registers_size)
{
    this->registers_size = registers_size;
    if (this->registers_size > 0 && this->registers == nullptr) {
        this->registers = new Value[registers_size];
    }
}

void Frame::free_registers()
{
    if (this->registers != nullptr) {
        delete[] this->registers;
        this->registers = nullptr;
    }
}

Frame::Frame(uint32_t registers_size)
{
    this->allocate_registers(registers_size);
}

uint32_t FrameInfo::get_register(bool protect)
{
    // Sotres the resulting register.
    uint32_t reg;
    // Check if there are free registers to use before
    // allocating a new one.
    if (this->free_registers.size() == 0) {
        // Return a new register and increment it by 1.
        reg = this->current_register++;
    } else {
        // Get the register from the free registers list.
        reg = this->free_registers.back();
        // Delete the register since it will be used.
        this->free_registers.pop_back();
    }
    // Protect it in case protect is true.
    if (protect) this->protected_registers.push_back(reg);
    // Return the register.
    return reg;
}

void FrameInfo::free_register(uint32_t reg, bool force)
{
    // Check if it's a protected register
    auto el = std::find(this->protected_registers.begin(), this->protected_registers.end(), reg);
    if (el != this->protected_registers.end()) {
        // The register is currently protected
        // Check if we're forcing a remove or not.
        if (!force) return;
        // Remove it from the protected list.
        this->protected_registers.erase(el);
    }
    // Add the element to the free list.
    this->free_registers.push_back(reg);
}

#undef MAX_OPERANDS
