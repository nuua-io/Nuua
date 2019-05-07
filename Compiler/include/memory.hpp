#ifndef MEMORY_HPP
#define MEMORY_HPP

#include "value.hpp"
#include "opcodes.hpp"

// Defines a basic memory for nuua.
class Memory
{
    public:
        // This stores the opcodes and consant indexes of the code.
        std::vector<opcode_t> *code = nullptr;
        // Stores the value constants.
        std::vector<Value> constants;
        // Stores the lines corresponding to the opcodes.
        std::unordered_map<size_t, const std::string *> *files = nullptr;
        // Stores the lines corresponding to the opcodes.
        std::unordered_map<size_t, const line_t> *lines = nullptr;
        // Stores the lines corresponding to the opcodes.
        std::unordered_map<size_t, const column_t> *columns = nullptr;
        // Dumps the memory.
        void dump();
        Memory()
            : code(new std::vector<opcode_t>),
              files(new std::unordered_map<size_t, const std::string *>),
              lines(new std::unordered_map<size_t, const line_t>),
              columns(new std::unordered_map<size_t, const column_t>)
            {}
        ~Memory() { delete code; delete files; delete lines; delete columns; }
};

#endif
