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
#include "../include/value.hpp"
#include <algorithm>

static std::pair<std::string, std::vector<OpCodeType>> opcode_names[] = {
   // Others
    { "EXIT", {{ OT_NONE, OT_NONE, OT_NONE }} }, // EXIT - - -

    // Register manipulation
    { "MOVE", {{ OT_REG, OT_REG, OT_NONE }} }, // MOVE RX RY
    { "LOAD_C", {{ OT_REG, OT_CONST, OT_NONE }} }, // LOAD_C RX C1
    { "LOAD_G", {{ OT_REG, OT_GLOBAL, OT_NONE }} }, // LOAD_G RX G1
    { "SET_G", {{ OT_GLOBAL, OT_REG, OT_NONE }} }, // SET_G G1 RX

    // Stack manipulation
    { "PUSH", {{ OT_REG, OT_NONE, OT_NONE }} }, // PUSH RX
    { "PUSH_C", {{ OT_CONST, OT_NONE, OT_NONE }} }, // PUSH C1
    { "POP", {{ OT_REG, OT_NONE, OT_NONE }} }, // POP RX

    // List releated
    { "OP_LPUSH", {{ OT_REG, OT_REG, OT_NONE }} }, // LPUSH RX RY
    { "OP_LPOP", {{ OT_REG, OT_NONE, OT_NONE }} }, // LPOP RX

    // Function releated
    { "CALL", {{ OT_REG, OT_NONE, OT_NONE }} }, // CALL RX
    { "RETURN", {{ OT_NONE, OT_NONE, OT_NONE }} }, // RETURN

    // Value casting
    { "CAST_INT_FLOAT", {{ OT_REG, OT_REG, OT_NONE }} }, // CAST_INT_FLOAT RX RY
    { "CAST_INT_BOOL", {{ OT_REG, OT_REG, OT_NONE }} }, // CAST_INT_BOOL RX RY
    { "CAST_INT_STRING", {{ OT_REG, OT_REG, OT_NONE }} }, // CAST_INT_STRING RX RY
    { "CAST_FLOAT_INT", {{ OT_REG, OT_REG, OT_NONE }} }, // CAST_FLOAT_INT RX RY
    { "CAST_FLOAT_BOOL", {{ OT_REG, OT_REG, OT_NONE }} }, // CAST_FLOAT_BOOL RX RY
    { "CAST_FLOAT_STRING", {{ OT_REG, OT_REG, OT_NONE }} }, // CAST_FLOAT_STRING RX RY
    { "CAST_BOOL_INT", {{ OT_REG, OT_REG, OT_NONE }} }, // CAST_BOOL_INT RX RY
    { "CAST_BOOL_FLOAT", {{ OT_REG, OT_REG, OT_NONE }} }, // CAST_BOOL_FLOAT RX RY
    { "CAST_BOOL_STRING", {{ OT_REG, OT_REG, OT_NONE }} }, // CAST_BOOL_STRING RX RY
    { "CAST_LIST_STRING", {{ OT_REG, OT_REG, OT_NONE }} }, // CAST_LIST_STRING RX RY
    { "CAST_LIST_BOOL", {{ OT_REG, OT_REG, OT_NONE }} }, // CAST_LIST_BOOL RX RY
    { "CAST_DICT_STRING", {{ OT_REG, OT_REG, OT_NONE }} }, // CAST_DICT_STRING RX RY
    { "CAST_DICT_BOOL", {{ OT_REG, OT_REG, OT_NONE }} }, // CAST_DICT_BOOL RX RY
    { "CAST_STRING_BOOL", {{ OT_REG, OT_REG, OT_NONE }} }, // CAST_STRING_BOOL RX RY

    // Negation
    { "NEG_BOOL", {{ OT_REG, OT_REG, OT_NONE }} }, // NEG_BOOL RX RY

    // Minus operations
    { "MINUS_INT", {{ OT_REG, OT_REG, OT_NONE }} }, // MINUS_INT RX RY
    { "MINUS_FLOAT", {{ OT_REG, OT_REG, OT_NONE }} }, // MINUS_FLOAT RX RY
    { "MINUS_BOOL", {{ OT_REG, OT_REG, OT_NONE }} }, // MINUS_BOOL RX RY

    // Plus operations
    { "PLUS_INT", {{ OT_REG, OT_REG, OT_NONE }} }, // PLUS_INT RX RY
    { "PLUS_FLOAT", {{ OT_REG, OT_REG, OT_NONE }} }, // PLUS_FLOAT RX RY
    { "PLUS_BOOL", {{ OT_REG, OT_REG, OT_NONE }} }, // PLUS_BOOL RX RY

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

    // Control flow (All relative jumps)
    { "FJUMP", {{ OT_LITERAL, OT_NONE, OT_NONE }} }, // FJUMP A
    { "BJUMP", {{ OT_LITERAL, OT_NONE, OT_NONE }} }, // BJUMP A
    { "CFJUMP", {{ OT_LITERAL, OT_REG, OT_NONE }} }, // FJUMP A RX
    { "CBJUMP", {{ OT_LITERAL, OT_REG, OT_NONE }} }, // BJUMP A RX
    { "CFNJUMP", {{ OT_LITERAL, OT_REG, OT_NONE }} }, // FNJUMP A RX
    { "CBNJUMP", {{ OT_LITERAL, OT_REG, OT_NONE }} }, // BNJUMP A RX

    // Utilities
    { "PRINT", {{ OT_REG, OT_NONE, OT_NONE }} }, // PRINT RX
    { "PRINT_C", {{ OT_CONST, OT_NONE, OT_NONE }} },
};

std::string opcode_to_string(opcode_t opcode)
{
    return opcode_names[opcode].first;
}

std::vector<OpCodeType> *opcode_operands(opcode_t opcode)
{
    return &opcode_names[opcode].second;
}

void print_opcode(opcode_t opcode)
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
