/**
 * |------------------------|
 * | Nuua Program Structure |
 * |------------------------|
 *
 * Copyright 2019 Erik Campobadal <soc@erik.cat>
 * https://nuua.io
 */
#include "../include/program.hpp"
#include "../include/memory.hpp"
#include "../../Logger/include/logger.hpp"
#include "../include/value.hpp"
#include <algorithm>

static std::pair<std::string, std::vector<OpCodeType>> opcode_names[] = {
   // Others
    { "EXIT", {{ }} }, // EXIT - - -

    // Register manipulation
    { "MOVE", {{ OT_REG, OT_REG }} }, // MOVE RX RY
    { "LOAD_C", {{ OT_REG, OT_CONST }} }, // LOAD_C RX C1
    { "LOAD_G", {{ OT_REG, OT_GLOBAL }} }, // LOAD_G RX G1
    { "SET_G", {{ OT_GLOBAL, OT_REG }} }, // SET_G G1 RX

    // Stack manipulation
    { "PUSH", {{ OT_REG }} }, // PUSH RX
    { "PUSH_C", {{ OT_CONST }} }, // PUSH C1
    { "POP", {{ OT_REG }} }, // POP RX

    // String releated
    { "SGET", {{ OT_REG, OT_REG, OT_REG }} }, // SGET RX RY RZ
    { "SSET", {{ OT_REG, OT_REG, OT_REG }} }, // SSET RX RY RZ
    { "SDELETE", {{ OT_REG, OT_REG }} }, // SDELETE RX RY

    // List releated
    { "LPUSH", {{ OT_REG, OT_REG }} }, // LPUSH RX RY
    { "LPUSH_C", {{ OT_REG, OT_CONST }} }, // LPUSH_C RX C1
    { "LPOP", {{ OT_REG }} }, // LPOP RX
    { "LGET", {{ OT_REG, OT_REG, OT_REG }} }, // LGET RX RY RZ
    { "LSET", {{ OT_REG, OT_REG, OT_REG }} }, // LSET RX RY RZ
    { "LDELETE", {{ OT_REG, OT_REG }} }, // LDELETE RX RY

    // Dictionary releated
    { "DGET", {{ OT_REG, OT_REG, OT_REG }} }, // DGET RX RY RZ
    { "DSET", {{ OT_REG, OT_REG, OT_REG }} }, // DSET RX RY RZ
    { "DDELETE", {{ OT_REG, OT_REG }} }, // DDELETE RX RY

    // Function releated
    { "CALL", {{ OT_REG }} }, // CALL RX
    { "RETURN", {{ }} }, // RETURN

    // Value casting
    { "CAST_INT_FLOAT", {{ OT_REG, OT_REG }} }, // CAST_INT_FLOAT RX RY
    { "CAST_INT_BOOL", {{ OT_REG, OT_REG }} }, // CAST_INT_BOOL RX RY
    { "CAST_INT_STRING", {{ OT_REG, OT_REG }} }, // CAST_INT_STRING RX RY
    { "CAST_FLOAT_INT", {{ OT_REG, OT_REG }} }, // CAST_FLOAT_INT RX RY
    { "CAST_FLOAT_BOOL", {{ OT_REG, OT_REG }} }, // CAST_FLOAT_BOOL RX RY
    { "CAST_FLOAT_STRING", {{ OT_REG, OT_REG }} }, // CAST_FLOAT_STRING RX RY
    { "CAST_BOOL_INT", {{ OT_REG, OT_REG }} }, // CAST_BOOL_INT RX RY
    { "CAST_BOOL_FLOAT", {{ OT_REG, OT_REG }} }, // CAST_BOOL_FLOAT RX RY
    { "CAST_BOOL_STRING", {{ OT_REG, OT_REG }} }, // CAST_BOOL_STRING RX RY
    { "CAST_LIST_STRING", {{ OT_REG, OT_REG }} }, // CAST_LIST_STRING RX RY
    { "CAST_LIST_BOOL", {{ OT_REG, OT_REG }} }, // CAST_LIST_BOOL RX RY
    { "CAST_LIST_INT", {{ OT_REG, OT_REG }} }, // CAST_LIST_INT RX RY
    { "CAST_DICT_STRING", {{ OT_REG, OT_REG }} }, // CAST_DICT_STRING RX RY
    { "CAST_DICT_BOOL", {{ OT_REG, OT_REG }} }, // CAST_DICT_BOOL RX RY
    { "CAST_DICT_INT", {{ OT_REG, OT_REG }} }, // CAST_DICT_INT RX RY
    { "CAST_STRING_BOOL", {{ OT_REG, OT_REG }} }, // CAST_STRING_BOOL RX RY
    { "CAST_STRING_INT", {{ OT_REG, OT_REG }} }, // CAST_STRING_INT RX RY

    // Negation
    { "NEG_BOOL", {{ OT_REG, OT_REG }} }, // NEG_BOOL RX RY

    // Minus operations
    { "MINUS_INT", {{ OT_REG, OT_REG }} }, // MINUS_INT RX RY
    { "MINUS_FLOAT", {{ OT_REG, OT_REG }} }, // MINUS_FLOAT RX RY
    { "MINUS_BOOL", {{ OT_REG, OT_REG }} }, // MINUS_BOOL RX RY

    // Plus operations
    { "PLUS_INT", {{ OT_REG, OT_REG }} }, // PLUS_INT RX RY
    { "PLUS_FLOAT", {{ OT_REG, OT_REG }} }, // PLUS_FLOAT RX RY
    { "PLUS_BOOL", {{ OT_REG, OT_REG }} }, // PLUS_BOOL RX RY

    // Extra binary but unary
    { "INC", {{ OT_REG }} }, // INC RX
    { "DEC", {{ OT_REG }} }, // DEC RX

    // Addition
    { "ADD_INT", {{ OT_REG, OT_REG, OT_REG }} }, // ADD_INT RX RY RZ
    { "ADD_FLOAT", {{ OT_REG, OT_REG, OT_REG }} }, // ADD_FLOAT RX RY RZ
    { "ADD_STRING", {{ OT_REG, OT_REG, OT_REG }} }, // ADD_STRING RX RY RZ
    { "ADD_BOOL", {{ OT_REG, OT_REG, OT_REG }} }, // ADD_BOOL RX RY RZ
    { "ADD_LIST", {{ OT_REG, OT_REG, OT_REG }} }, // ADD_LIST RX RY RZ
    { "ADD_DICT", {{ OT_REG, OT_REG, OT_REG }} }, // ADD_DICT RX RY RZ

    // Substraction
    { "SUB_INT", {{ OT_REG, OT_REG, OT_REG }} }, // SUB_INT RX RY RZ
    { "SUB_FLOAT", {{ OT_REG, OT_REG, OT_REG }} }, // SUB_FLOAT RX RY RZ
    { "SUB_BOOL", {{ OT_REG, OT_REG, OT_REG }} }, // SUB_BOOL RX RY RZ

    // Multiplication
    { "MUL_INT", {{ OT_REG, OT_REG, OT_REG }} }, // MUL_INT RX RY RZ
    { "MUL_FLOAT", {{ OT_REG, OT_REG, OT_REG }} }, // MUL_FLOAT RX RY RZ
    { "MUL_BOOL", {{ OT_REG, OT_REG, OT_REG }} }, // MUL_BOOL RX RY RZ
    { "MUL_INT_STRING", {{ OT_REG, OT_REG, OT_REG }} }, // MUL_INT_STRING RX RY RZ
    { "MUL_STRING_INT", {{ OT_REG, OT_REG, OT_REG }} }, // MUL_STRING_INT RX RY RZ
    { "MUL_INT_LIST", {{ OT_REG, OT_REG, OT_REG }} }, // MUL_INT_LIST RX RY RZ
    { "MUL_LIST_INT", {{ OT_REG, OT_REG, OT_REG }} }, // MUL_LIST_INT RX RY RZ

    // Division
    { "DIV_INT", {{ OT_REG, OT_REG, OT_REG }} }, // DIV_INT RX RY RZ
    { "DIV_FLOAT", {{ OT_REG, OT_REG, OT_REG }} }, // DIV_FLOAT RX RY RZ
    { "DIV_STRING_INT", {{ OT_REG, OT_REG, OT_REG }} }, // DIV_STRING_INT RX RY RZ
    { "DIV_LIST_INT", {{ OT_REG, OT_REG, OT_REG }} }, // DIV_LIST_INT RX RY RZ

    // Equality
    { "EQ_INT", {{ OT_REG, OT_REG, OT_REG }} }, // EQ_INT RX RY RZ
    { "EQ_FLOAT", {{ OT_REG, OT_REG, OT_REG }} }, // EQ_FLOAT RX RY RZ
    { "EQ_STRING", {{ OT_REG, OT_REG, OT_REG }} }, // EQ_STRING RX RY RZ
    { "EQ_BOOL", {{ OT_REG, OT_REG, OT_REG }} }, // EQ_BOOL RX RY RZ
    { "EQ_LIST", {{ OT_REG, OT_REG, OT_REG }} }, // EQ_LIST RX RY RZ
    { "EQ_DICT", {{ OT_REG, OT_REG, OT_REG }} }, // EQ_DICT RX RY RZ

    // Not Equality
    { "NEQ_INT", {{ OT_REG, OT_REG, OT_REG }} }, // NEQ_INT RX RY RZ
    { "NEQ_FLOAT", {{ OT_REG, OT_REG, OT_REG }} }, // NEQ_FLOAT RX RY RZ
    { "NEQ_STRING", {{ OT_REG, OT_REG, OT_REG }} }, // NEQ_STRING RX RY RZ
    { "NEQ_BOOL", {{ OT_REG, OT_REG, OT_REG }} }, // NEQ_BOOL RX RY RZ
    { "NEQ_LIST", {{ OT_REG, OT_REG, OT_REG }} }, // NEQ_LIST RX RY RZ
    { "NEQ_DICT", {{ OT_REG, OT_REG, OT_REG }} }, // NEQ_DICT RX RY RZ

    // Higher than
    { "HT_INT", {{ OT_REG, OT_REG, OT_REG }} }, // HT_INT RX RY RZ
    { "HT_FLOAT", {{ OT_REG, OT_REG, OT_REG }} }, // HT_FLOAT RX RY RZ
    { "HT_STRING", {{ OT_REG, OT_REG, OT_REG }} }, // HT_STRING RX RY RZ
    { "HT_BOOL", {{ OT_REG, OT_REG, OT_REG }} }, // HT_BOOL RX RY RZ

    // Higher than or equal to
    { "HTE_INT", {{ OT_REG, OT_REG, OT_REG }} }, // HTE_INT RX RY RZ
    { "HTE_FLOAT", {{ OT_REG, OT_REG, OT_REG }} }, // HTE_FLOAT RX RY RZ
    { "HTE_STRING", {{ OT_REG, OT_REG, OT_REG }} }, // HTE_STRING RX RY RZ
    { "HTE_BOOL", {{ OT_REG, OT_REG, OT_REG }} }, // HTE_BOOL RX RY RZ

    // Lower than
    { "LT_INT", {{ OT_REG, OT_REG, OT_REG }} }, // LT_INT RX RY RZ
    { "LT_FLOAT", {{ OT_REG, OT_REG, OT_REG }} }, // LT_FLOAT RX RY RZ
    { "LT_STRING", {{ OT_REG, OT_REG, OT_REG }} }, // LT_STRING RX RY RZ
    { "LT_BOOL", {{ OT_REG, OT_REG, OT_REG }} }, // LT_BOOL RX RY RZ

    // Lower than or equal to
    { "LTE_INT", {{ OT_REG, OT_REG, OT_REG }} }, // LTE_INT RX RY RZ
    { "LTE_FLOAT", {{ OT_REG, OT_REG, OT_REG }} }, // LTE_FLOAT RX RY RZ
    { "LTE_STRING", {{ OT_REG, OT_REG, OT_REG }} }, // LTE_STRING RX RY RZ
    { "LTE_BOOL", {{ OT_REG, OT_REG, OT_REG }} }, // LTE_BOOL RX RY RZ

    // Logical operations
    { "OR", {{ OT_REG, OT_REG, OT_REG }} }, // OR RX RY RZ
    { "AND", {{ OT_REG, OT_REG, OT_REG }} }, // AND RX RY RZ

    // Control flow (All relative jumps)
    { "FJUMP", {{ OT_LITERAL }} }, // FJUMP A
    { "BJUMP", {{ OT_LITERAL }} }, // BJUMP A
    { "CFJUMP", {{ OT_LITERAL, OT_REG }} }, // FJUMP A RX
    { "CBJUMP", {{ OT_LITERAL, OT_REG }} }, // BJUMP A RX
    { "CFNJUMP", {{ OT_LITERAL, OT_REG }} }, // FNJUMP A RX
    { "CBNJUMP", {{ OT_LITERAL, OT_REG }} }, // BNJUMP A RX

    // Slice and range
    { "SSLICE", {{ OT_REG, OT_REG, OT_REG, OT_REG, OT_REG }} }, // SSLICE RX RY R1 R2 R3 (dest, target, start, end, step)
    { "SSLICEE", {{ OT_REG, OT_REG, OT_REG, OT_REG }} }, // SSLICE RX RY R1 R2 (dest, target, start, step)
    { "LSLICE", {{ OT_REG, OT_REG, OT_REG, OT_REG, OT_REG }} }, // LSLICE RX RY PR PR R3 (dest, target, start, end, step)
    { "LSLICEE", {{ OT_REG, OT_REG, OT_REG, OT_REG }} }, // LSLICEE RX RY R1 R2 (dest, target, start, step)
    { "RANGEE", {{ OT_REG, OT_REG, OT_REG }} }, // RANGEE RX R1 R2 (dest, start, end)
    { "RANGEI", {{ OT_REG, OT_REG, OT_REG }} }, // RANGEI RX R1 R2 (dest, start, end)

    // Utilities
    { "PRINT", {{ OT_REG }} }, // PRINT RX
    { "PRINT_C", {{ OT_CONST }} },
};

std::string opcode_to_string(const opcode_t opcode)
{
    return opcode_names[opcode].first;
}

std::vector<OpCodeType> *opcode_operands(const opcode_t opcode)
{
    return &opcode_names[opcode].second;
}

void print_opcode(const opcode_t opcode)
{
    printf("%17.17s", opcode_to_string(opcode).c_str());
}

void Frame::allocate_registers()
{
    if (this->registers_size > 0 && this->registers == nullptr) {
        this->registers = new Value[this->registers_size];
    }
}

void Frame::free_registers()
{
    if (this->registers != nullptr) {
        delete[] this->registers;
        this->registers = nullptr;
    }
}


register_t FrameInfo::get_register(bool protect)
{
    // Sotres the resulting register.
    register_t reg;
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

void FrameInfo::free_register(register_t reg, bool force)
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

void FrameInfo::reset()
{
    this->current_register = 0;
    this->free_registers.clear();
    this->protected_registers.clear();
}

Program::Program()
{
    this->memory = std::make_unique<Memory>();
}
