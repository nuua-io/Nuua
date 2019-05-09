#ifndef MEMORY_HPP
#define MEMORY_HPP

#include "value.hpp"
#include "opcodes.hpp"

// Defines a basic memory for nuua.
class Memory
{
    public:
        // This stores the opcodes and consant indexes of the code.
        std::vector<opcode_t> code;
        // Stores the value constants.
        std::vector<Value> constants;
        // Stores the lines corresponding to the opcodes.
        std::unordered_map<size_t, std::shared_ptr<const std::string>> files;
        // Stores the lines corresponding to the opcodes.
        std::unordered_map<size_t, line_t> lines;
        // Stores the lines corresponding to the opcodes.
        std::unordered_map<size_t, column_t> columns;
        // Dumps the memory.
        void dump();
};

#endif
