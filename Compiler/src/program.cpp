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

static std::string opcode_names[] = {
    // Others
    "-",
    "EXIT", // EXIT

    // Register manipulation
    "MOVE_RR", // MOVE RX RY
    "MOVE_RC", // LOAD RX C1

    // Addition
    "ADD_RR", // ADD RX RY RZ
    "ADD_RC", // ADD RX RY C1
    "ADD_CR", // ADD RX C1 RY
    "ADD_CC", // ADD RX C1 C2

    // Substraction
    "SUB_RR", // SUB RX RY RZ
    "SUB_RC", // SUB RX RY C1
    "SUB_CR", // SUB RX C1 RY
    "SUB_CC", // SUB RX C1 C2

    // Multiplication
    "MUL_RR", // MUL RX RY RZ
    "MUL_RC", // MUL RX RY C1
    "MUL_CR", // MUL RX C1 RY
    "MUL_CC", // MUL RX C1 C2

    // Division
    "DIV_RR", // DIV RX RY RZ
    "DIV_RC", // DIV RX RY C1
    "DIV_CR", // DIV RX C1 RY
    "DIV_CC", // DIV RX C1 C2

    // Equality
    "EQ_RR", // EQ RX RY RZ
    "EQ_RC", // EQ RX RY C1
    "EQ_CR", // EQ RX C1 RY
    "EQ_CC", // EQ RX C1 C2

    // No equality
    "NEQ_RR", // NEQ RX RY RZ
    "NEQ_RC", // NEQ RX RY C1
    "NEQ_CR", // NEQ RX C1 RY
    "NEQ_CC", // NEQ RX C1 C2

    // Higher than
    "HT_RR", // HT RX RY RZ
    "HT_RC", // HT RX RY C1
    "HT_CR", // HT RX C1 RY
    "HT_CC", // HT RX C1 C2

    // Higher than or equal
    "HTE_RR", // HTE RX RY RZ
    "HTE_RC", // HTE RX RY C1
    "HTE_CR", // HTE RX C1 RY
    "HTE_CC", // HTE RX C1 C2

    // Lower then
    "LT_RR", // LT RX RY RZ
    "LT_RC", // LT RX RY C1
    "LT_CR", // LT RX C1 RY
    "LT_CC", // LT RX C1 C2

    // Lower than or equal
    "LTE_RR", // LTE RX RY RZ
    "LTE_RC", // LTE RX RY C1
    "LTE_CR", // LTE RX C1 RY
    "LTE_CC", // LTE RX C1 C2

    // Control flow (All relative jumps)
    "FJUMP", // FJUMP A
    "BJUMP", // BJUMP A
    "FJUMP_R", // FJUMP A RX
    "BJUMP_R", // BJUMP A RX
    "FJUMP_C", // FJUMP A C1
    "BJUMP_C", // BJUMP A C1
    "FNJUMP_R", // FNJUMP A RX
    "BNJUMP_R", // BNJUMP A RX
    "FNJUMP_C", // FNJUMP A C1
    "BNJUMP_C", // BNJUMP A C1

    // Utilities
    "PRINT_R", // PRINT RX
    "PRINT_C", // PRINT C1
};

typedef enum : uint8_t {
    OPERAND_EMPTY, OPERAND_REGISTER, OPERAND_CONSTANT, OPERAND_LITERAL
} OperandType;

static uint8_t opcode_operands[][MAX_OPERANDS] = {
    // Others
    { OPERAND_EMPTY, OPERAND_EMPTY, OPERAND_EMPTY }, // Should never happen anyway.
    { OPERAND_EMPTY, OPERAND_EMPTY, OPERAND_EMPTY },

    // Register manipulation
    { OPERAND_REGISTER, OPERAND_REGISTER, OPERAND_EMPTY },
    { OPERAND_REGISTER, OPERAND_CONSTANT, OPERAND_EMPTY },

    // Addition
    { OPERAND_REGISTER, OPERAND_REGISTER, OPERAND_REGISTER },
    { OPERAND_REGISTER, OPERAND_REGISTER, OPERAND_CONSTANT },
    { OPERAND_REGISTER, OPERAND_CONSTANT, OPERAND_REGISTER },
    { OPERAND_REGISTER, OPERAND_CONSTANT, OPERAND_CONSTANT },

    // Substraction
    { OPERAND_REGISTER, OPERAND_REGISTER, OPERAND_REGISTER },
    { OPERAND_REGISTER, OPERAND_REGISTER, OPERAND_CONSTANT },
    { OPERAND_REGISTER, OPERAND_CONSTANT, OPERAND_REGISTER },
    { OPERAND_REGISTER, OPERAND_CONSTANT, OPERAND_CONSTANT },

    // Multiplication
    { OPERAND_REGISTER, OPERAND_REGISTER, OPERAND_REGISTER },
    { OPERAND_REGISTER, OPERAND_REGISTER, OPERAND_CONSTANT },
    { OPERAND_REGISTER, OPERAND_CONSTANT, OPERAND_REGISTER },
    { OPERAND_REGISTER, OPERAND_CONSTANT, OPERAND_CONSTANT },

    // Division
    { OPERAND_REGISTER, OPERAND_REGISTER, OPERAND_REGISTER },
    { OPERAND_REGISTER, OPERAND_REGISTER, OPERAND_CONSTANT },
    { OPERAND_REGISTER, OPERAND_CONSTANT, OPERAND_REGISTER },
    { OPERAND_REGISTER, OPERAND_CONSTANT, OPERAND_CONSTANT },

    // Equality
    { OPERAND_REGISTER, OPERAND_REGISTER, OPERAND_REGISTER },
    { OPERAND_REGISTER, OPERAND_REGISTER, OPERAND_CONSTANT },
    { OPERAND_REGISTER, OPERAND_CONSTANT, OPERAND_REGISTER },
    { OPERAND_REGISTER, OPERAND_CONSTANT, OPERAND_CONSTANT },

    // No equality
    { OPERAND_REGISTER, OPERAND_REGISTER, OPERAND_REGISTER },
    { OPERAND_REGISTER, OPERAND_REGISTER, OPERAND_CONSTANT },
    { OPERAND_REGISTER, OPERAND_CONSTANT, OPERAND_REGISTER },
    { OPERAND_REGISTER, OPERAND_CONSTANT, OPERAND_CONSTANT },

    // Higher than
    { OPERAND_REGISTER, OPERAND_REGISTER, OPERAND_REGISTER },
    { OPERAND_REGISTER, OPERAND_REGISTER, OPERAND_CONSTANT },
    { OPERAND_REGISTER, OPERAND_CONSTANT, OPERAND_REGISTER },
    { OPERAND_REGISTER, OPERAND_CONSTANT, OPERAND_CONSTANT },

    // Higher than or equal
    { OPERAND_REGISTER, OPERAND_REGISTER, OPERAND_REGISTER },
    { OPERAND_REGISTER, OPERAND_REGISTER, OPERAND_CONSTANT },
    { OPERAND_REGISTER, OPERAND_CONSTANT, OPERAND_REGISTER },
    { OPERAND_REGISTER, OPERAND_CONSTANT, OPERAND_CONSTANT },

    // Lower then
    { OPERAND_REGISTER, OPERAND_REGISTER, OPERAND_REGISTER },
    { OPERAND_REGISTER, OPERAND_REGISTER, OPERAND_CONSTANT },
    { OPERAND_REGISTER, OPERAND_CONSTANT, OPERAND_REGISTER },
    { OPERAND_REGISTER, OPERAND_CONSTANT, OPERAND_CONSTANT },

    // Lower than or equal
    { OPERAND_REGISTER, OPERAND_REGISTER, OPERAND_REGISTER },
    { OPERAND_REGISTER, OPERAND_REGISTER, OPERAND_CONSTANT },
    { OPERAND_REGISTER, OPERAND_CONSTANT, OPERAND_REGISTER },
    { OPERAND_REGISTER, OPERAND_CONSTANT, OPERAND_CONSTANT },

    // Control flow (All relative jumps)
    { OPERAND_LITERAL, OPERAND_EMPTY, OPERAND_EMPTY },
    { OPERAND_LITERAL, OPERAND_EMPTY, OPERAND_EMPTY },
    { OPERAND_LITERAL, OPERAND_REGISTER, OPERAND_EMPTY },
    { OPERAND_LITERAL, OPERAND_REGISTER, OPERAND_EMPTY },
    { OPERAND_LITERAL, OPERAND_CONSTANT, OPERAND_EMPTY },
    { OPERAND_LITERAL, OPERAND_CONSTANT, OPERAND_EMPTY },
    { OPERAND_LITERAL, OPERAND_REGISTER, OPERAND_EMPTY },
    { OPERAND_LITERAL, OPERAND_REGISTER, OPERAND_EMPTY },
    { OPERAND_LITERAL, OPERAND_CONSTANT, OPERAND_EMPTY },
    { OPERAND_LITERAL, OPERAND_CONSTANT, OPERAND_EMPTY },

    // Utilities
    { OPERAND_REGISTER, OPERAND_EMPTY, OPERAND_EMPTY },
    { OPERAND_CONSTANT, OPERAND_EMPTY, OPERAND_EMPTY },
};

void Memory::dump()
{
    for (size_t i = 0; i < this->code.size(); i++) {
        auto el = this->code[i];
        printf(opcode_names[el].c_str());
        for (uint8_t k = 0; k < MAX_OPERANDS && opcode_operands[el][k] != OPERAND_EMPTY; k++) {
            switch (opcode_operands[el][k]) {
                case OPERAND_REGISTER: { printf(" R%llu", this->code[++i]); break; }
                case OPERAND_CONSTANT: { printf(" <%s>", this->constants[this->code[++i]].to_string().c_str()); break; }
                case OPERAND_LITERAL: { printf(" %llu", this->code[++i]); break; }
                case OPERAND_EMPTY: { printf(" -"); ++i; break; }
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

uint64_t FrameInfo::get_register(bool protect)
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

void FrameInfo::free_register(uint64_t reg, bool force)
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
