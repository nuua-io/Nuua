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
    "OP_EXIT", // EXIT

    // Register manipulation
    "OP_MOVE_RR", // MOVE RX RY
    "OP_MOVE_RC", // LOAD RX C1

    // Addition
    "OP_ADD_RR", // ADD RX RY RZ
    "OP_ADD_RC", // ADD RX RY C1
    "OP_ADD_CR", // ADD RX C1 RY
    "OP_ADD_CC", // ADD RX C1 C2

    // Substraction
    "OP_SUB_RR", // SUB RX RY RZ
    "OP_SUB_RC", // SUB RX RY C1
    "OP_SUB_CR", // SUB RX C1 RY
    "OP_SUB_CC", // SUB RX C1 C2

    // Multiplication
    "OP_MUL_RR", // MUL RX RY RZ
    "OP_MUL_RC", // MUL RX RY C1
    "OP_MUL_CR", // MUL RX C1 RY
    "OP_MUL_CC", // MUL RX C1 C2

    // Division
    "OP_DIV_RR", // DIV RX RY RZ
    "OP_DIV_RC", // DIV RX RY C1
    "OP_DIV_CR", // DIV RX C1 RY
    "OP_DIV_CC", // DIV RX C1 C2

    // Equality
    "OP_EQ_RR", // EQ RX RY RZ
    "OP_EQ_RC", // EQ RX RY C1
    "OP_EQ_CR", // EQ RX C1 RY
    "OP_EQ_CC", // EQ RX C1 C2

    // No equality
    "OP_NEQ_RR", // NEQ RX RY RZ
    "OP_NEQ_RC", // NEQ RX RY C1
    "OP_NEQ_CR", // NEQ RX C1 RY
    "OP_NEQ_CC", // NEQ RX C1 C2

    // Higher than
    "OP_HT_RR", // HT RX RY RZ
    "OP_HT_RC", // HT RX RY C1
    "OP_HT_CR", // HT RX C1 RY
    "OP_HT_CC", // HT RX C1 C2

    // Higher than or equal
    "OP_HTE_RR", // HTE RX RY RZ
    "OP_HTE_RC", // HTE RX RY C1
    "OP_HTE_CR", // HTE RX C1 RY
    "OP_HTE_CC", // HTE RX C1 C2

    // Lower then
    "OP_LT_RR", // LT RX RY RZ
    "OP_LT_RC", // LT RX RY C1
    "OP_LT_CR", // LT RX C1 RY
    "OP_LT_CC", // LT RX C1 C2

    // Lower than or equal
    "OP_LTE_RR", // LTE RX RY RZ
    "OP_LTE_RC", // LTE RX RY C1
    "OP_LTE_CR", // LTE RX C1 RY
    "OP_LTE_CC", // LTE RX C1 C2

    // Control flow (All relative jumps)
    "OP_FJUMP", // FJUMP A
    "OP_BJUMP", // BJUMP A
    "OP_FJUMP_R", // FJUMP A RX
    "OP_BJUMP_R", // BJUMP A RX
    "OP_FJUMP_C", // FJUMP A C1
    "OP_BJUMP_C", // BJUMP A C1
    "OP_FNJUMP_R", // FNJUMP A RX
    "OP_BNJUMP_R", // BNJUMP A RX
    "OP_FNJUMP_C", // FNJUMP A C1
    "OP_BNJUMP_C", // BNJUMP A C1

    // Utilities
    "OP_PRINT_R", // PRINT RX
    "OP_PRINT_C", // PRINT C1
};

typedef enum : uint8_t {
    OPERAND_NONE, OPERAND_REGISTER, OPERAND_CONSTANT, OPERAND_LITERAL
} OperandType;

static uint8_t opcode_operands[][MAX_OPERANDS] = {
    // Others
    { OPERAND_NONE, OPERAND_NONE, OPERAND_NONE },

    // Register manipulation
    { OPERAND_REGISTER, OPERAND_REGISTER, OPERAND_NONE },
    { OPERAND_REGISTER, OPERAND_CONSTANT, OPERAND_NONE },

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
    { OPERAND_LITERAL, OPERAND_NONE, OPERAND_NONE },
    { OPERAND_LITERAL, OPERAND_NONE, OPERAND_NONE },
    { OPERAND_LITERAL, OPERAND_REGISTER, OPERAND_NONE },
    { OPERAND_LITERAL, OPERAND_REGISTER, OPERAND_NONE },
    { OPERAND_LITERAL, OPERAND_CONSTANT, OPERAND_NONE },
    { OPERAND_LITERAL, OPERAND_CONSTANT, OPERAND_NONE },
    { OPERAND_LITERAL, OPERAND_REGISTER, OPERAND_NONE },
    { OPERAND_LITERAL, OPERAND_REGISTER, OPERAND_NONE },
    { OPERAND_LITERAL, OPERAND_CONSTANT, OPERAND_NONE },
    { OPERAND_LITERAL, OPERAND_CONSTANT, OPERAND_NONE },

    // Utilities
    { OPERAND_REGISTER, OPERAND_NONE, OPERAND_NONE },
    { OPERAND_CONSTANT, OPERAND_NONE, OPERAND_NONE },
};

void Memory::dump()
{
    for (size_t i = 0; i < this->code.size(); i++) {
        auto el = this->code[i];
        printf(opcode_names[el].c_str());
        for (uint8_t k = 0; k < MAX_OPERANDS && opcode_operands[el][k] != OPERAND_NONE; k++) {
            switch (opcode_operands[el][k]) {
                case OPERAND_REGISTER: { printf(" R%llu", this->code[++i]); break; }
                case OPERAND_CONSTANT: { printf(" <%s>", this->constants[this->code[++i]].to_string().c_str()); break; }
                case OPERAND_LITERAL: { printf(" %llu", ++i); break; }
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
