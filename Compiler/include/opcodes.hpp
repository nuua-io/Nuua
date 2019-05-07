#ifndef OPCODES_HPP
#define OPCODES_HPP

#include <string>
#include <stdint.h>

typedef size_t opcode_t;

// Defines the known opcodes for the program.
typedef enum : uint8_t {
    // Others
    OP_EXIT, // EXIT - - -

    // Register manipulation
    OP_MOVE, // MOVE RX RY
    OP_LOAD_C, // OP_LOAD_C RX C1
    OP_LOAD_G, // OP_LOAD_G RX G1
    OP_SET_G, // OP_SET_G G1 RX

    // Stack manipulation
    OP_PUSH, // PUSH RX
    OP_PUSH_C, // PUSH C1
    OP_POP, // POP RX

    // List releated
    OP_LPUSH, // LPUSH RX RY
    OP_LPOP, // LPOP RX

    // Function releated
    OP_CALL, // CALL RX
    OP_RETURN, // RETUR

    /* Casting Operations */

    // Value casting
    OP_CAST_INT_FLOAT, // CAST_INT_FLOAT RX RY
    OP_CAST_INT_BOOL, // CAST_INT_BOOL RX RY
    OP_CAST_INT_STRING, // CAST_INT_STRING RX RY
    OP_CAST_FLOAT_INT, // CAST_FLOAT_INT RX RY
    OP_CAST_FLOAT_BOOL, // CAST_FLOAT_BOOL RX RY
    OP_CAST_FLOAT_STRING, // CAST_FLOAT_STRING RX RY
    OP_CAST_BOOL_INT, // CAST_BOOL_INT RX RY
    OP_CAST_BOOL_FLOAT, // CAST_BOOL_FLOAT RX RY
    OP_CAST_BOOL_STRING, // CAST_BOOL_STRING RX RY
    OP_CAST_LIST_STRING, // CAST_LIST_STRING RX RY
    OP_CAST_LIST_BOOL, // CAST_LIST_BOOL RX RY
    OP_CAST_DICT_STRING, // CAST_DICT_STRING RX RY
    OP_CAST_DICT_BOOL, // CAST_DICT_BOOL RX RY
    OP_CAST_STRING_BOOL, // CAST_STRING_BOOL RX RY

    /* Unary Operations */

    // Negation
    OP_NEG_BOOL, // NEG_BOOL RX RY
    // Minus operations
    OP_MINUS_INT, // MINUS_INT RX RY
    OP_MINUS_FLOAT, // MINUS_FLOAT RX RY
    OP_MINUS_BOOL, // MINUS_BOOL RX RY
    // Plus operations
    OP_PLUS_INT, // PLUS_INT RX RY
    OP_PLUS_FLOAT, // PLUS_FLOAT RX RY
    OP_PLUS_BOOL, // PLUS_BOOL RX RY

    /* Binary Operations */

    // Addition
    OP_ADD_INT, // ADD_INT RX RY RZ
    OP_ADD_FLOAT, // ADD_FLOAT RX RY RZ
    OP_ADD_STRING, // ADD_STRING RX RY RZ
    OP_ADD_BOOL, // ADD_BOOL RX RY RZ
    OP_ADD_LIST, // ADD_LIST RX RY RZ
    OP_ADD_DICT, // ADD_DICT RX RY RZ
    // Substraction
    OP_SUB_INT, // SUB_INT RX RY RZ
    OP_SUB_FLOAT, // SUB_FLOAT RX RY RZ
    OP_SUB_BOOL, // SUB_BOOL RX RY RZ
    // Multiplication
    OP_MUL_INT, // MUL_INT RX RY RZ
    OP_MUL_FLOAT, // MUL_FLOAT RX RY RZ
    OP_MUL_BOOL, // MUL_BOOL RX RY RZ
    OP_MUL_INT_STRING, // MUL_INT_STRING RX RY RZ
    OP_MUL_STRING_INT, // MUL_STRING_INT RX RY RZ
    OP_MUL_INT_LIST, // MUL_INT_LIST RX RY RZ
    OP_MUL_LIST_INT, // MUL_LIST_INT RX RY RZ
    // Division
    OP_DIV_INT, // DIV_INT RX RY RZ
    OP_DIV_FLOAT, // DIV_FLOAT RX RY RZ
    OP_DIV_STRING_INT, // DIV_STRING_INT RX RY RZ
    OP_DIV_LIST_INT, // DIV_LIST_INT RX RY RZ
    // Equality
    OP_EQ_INT, // EQ_INT RX RY RZ
    OP_EQ_FLOAT, // EQ_FLOAT RX RY RZ
    OP_EQ_STRING, // EQ_STRING RX RY RZ
    OP_EQ_BOOL, // EQ_BOOL RX RY RZ
    OP_EQ_LIST, // EQ_LIST RX RY RZ
    OP_EQ_DICT, // EQ_DICT RX RY RZ
    // Not Equality
    OP_NEQ_INT, // NEQ_INT RX RY RZ
    OP_NEQ_FLOAT, // NEQ_FLOAT RX RY RZ
    OP_NEQ_STRING, // NEQ_STRING RX RY RZ
    OP_NEQ_BOOL, // NEQ_BOOL RX RY RZ
    OP_NEQ_LIST, // NEQ_LIST RX RY RZ
    OP_NEQ_DICT, // NEQ_DICT RX RY RZ
    // Higher than
    OP_HT_INT, // HT_INT RX RY RZ
    OP_HT_FLOAT, // HT_FLOAT RX RY RZ
    OP_HT_STRING, // HT_STRING RX RY RZ
    OP_HT_BOOL, // HT_BOOL RX RY RZ
    // Higher than or equal to
    OP_HTE_INT, // HTE_INT RX RY RZ
    OP_HTE_FLOAT, // HTE_FLOAT RX RY RZ
    OP_HTE_STRING, // HTE_STRING RX RY RZ
    OP_HTE_BOOL, // HTE_BOOL RX RY RZ
    // Lower than
    OP_LT_INT, // LT_INT RX RY RZ
    OP_LT_FLOAT, // LT_FLOAT RX RY RZ
    OP_LT_STRING, // LT_STRING RX RY RZ
    OP_LT_BOOL, // LT_BOOL RX RY RZ
    // Lower than or equal to
    OP_LTE_INT, // LTE_INT RX RY RZ
    OP_LTE_FLOAT, // LTE_FLOAT RX RY RZ
    OP_LTE_STRING, // LTE_STRING RX RY RZ
    OP_LTE_BOOL, // LTE_BOOL RX RY RZ

    // Control flow (All relative jumps)
    OP_FJUMP, // FJUMP A
    OP_BJUMP, // BJUMP A
    OP_CFJUMP, // FJUMP A RX
    OP_CBJUMP, // BJUMP A RX
    OP_CFNJUMP, // FNJUMP A RX
    OP_CBNJUMP, // BNJUMP A RX

    // Utilities
    OP_PRINT, // PRINT RX
    OP_PRINT_C, // PRINT C1
} OpCode;

#endif
