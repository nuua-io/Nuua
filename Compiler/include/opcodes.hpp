#ifndef OPCODES_HPP
#define OPCODES_HPP

#include <string>
#include <stdint.h>

#define MAX_OPERANDS 3

// Defines the known opcodes for the program.
typedef enum : uint8_t {
    // Others
    OP_EMPTY, // Empty space (nothing)
    OP_EXIT, // EXIT - - -

    // Register manipulation
    OP_MOVE_RR, // MOVE RX RY -
    OP_MOVE_RC, // LOAD RX C1 -

    // Addition
    OP_ADD_RR, // ADD RX RY RZ
    OP_ADD_RC, // ADD RX RY C1
    OP_ADD_CR, // ADD RX C1 RY
    OP_ADD_CC, // ADD RX C1 C2

    // Substraction
    OP_SUB_RR, // SUB RX RY RZ
    OP_SUB_RC, // SUB RX RY C1
    OP_SUB_CR, // SUB RX C1 RY
    OP_SUB_CC, // SUB RX C1 C2

    // Multiplication
    OP_MUL_RR, // MUL RX RY RZ
    OP_MUL_RC, // MUL RX RY C1
    OP_MUL_CR, // MUL RX C1 RY
    OP_MUL_CC, // MUL RX C1 C2

    // Division
    OP_DIV_RR, // DIV RX RY RZ
    OP_DIV_RC, // DIV RX RY C1
    OP_DIV_CR, // DIV RX C1 RY
    OP_DIV_CC, // DIV RX C1 C2

    // Equality
    OP_EQ_RR, // EQ RX RY RZ
    OP_EQ_RC, // EQ RX RY C1
    OP_EQ_CR, // EQ RX C1 RY
    OP_EQ_CC, // EQ RX C1 C2

    // No equality
    OP_NEQ_RR, // NEQ RX RY RZ
    OP_NEQ_RC, // NEQ RX RY C1
    OP_NEQ_CR, // NEQ RX C1 RY
    OP_NEQ_CC, // NEQ RX C1 C2

    // Higher than
    OP_HT_RR, // HT RX RY RZ
    OP_HT_RC, // HT RX RY C1
    OP_HT_CR, // HT RX C1 RY
    OP_HT_CC, // HT RX C1 C2

    // Higher than or equal
    OP_HTE_RR, // HTE RX RY RZ
    OP_HTE_RC, // HTE RX RY C1
    OP_HTE_CR, // HTE RX C1 RY
    OP_HTE_CC, // HTE RX C1 C2

    // Lower then
    OP_LT_RR, // LT RX RY RZ
    OP_LT_RC, // LT RX RY C1
    OP_LT_CR, // LT RX C1 RY
    OP_LT_CC, // LT RX C1 C2

    // Lower than or equal
    OP_LTE_RR, // LTE RX RY RZ
    OP_LTE_RC, // LTE RX RY C1
    OP_LTE_CR, // LTE RX C1 RY
    OP_LTE_CC, // LTE RX C1 C2

    // Control flow (All relative jumps)
    OP_FJUMP, // FJUMP A - -
    OP_BJUMP, // BJUMP A - -
    OP_FJUMP_R, // FJUMP A RX -
    OP_BJUMP_R, // BJUMP A RX -
    OP_FJUMP_C, // FJUMP A C1 -
    OP_BJUMP_C, // BJUMP A C1 -
    OP_FNJUMP_R, // FNJUMP A RX -
    OP_BNJUMP_R, // BNJUMP A RX -
    OP_FNJUMP_C, // FNJUMP A C1 -
    OP_BNJUMP_C, // BNJUMP A C1 -

    // Utilities
    OP_PRINT_R, // PRINT RX - -
    OP_PRINT_C, // PRINT C1 - -
} OpCode;

#endif
