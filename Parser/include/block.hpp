#ifndef BLOCK_HPP
#define BLOCK_HPP

#include <unordered_map>
#include <vector>
#include <utility>

// Forward declaration
class Expression;

// The BlockVariableType determines the
// representation of a variable type
// for the block analyzer.
class BlockVariableType
{
    public:
        // Represents the variable type
        std::string type;
        // Represents the registers where it's stored.
        uint32_t reg;
        // Represents the variable arguments (FUNCTION ARGUMENTS)
        std::vector<std::string> arguments;
        // Represents the variable return type (FUNCTION RETURN TYPE)
        std::string return_type;
        // Represents the last use of the variable (Variable life)
        Expression *last_use = nullptr;
        BlockVariableType() {};
        BlockVariableType(std::string type, std::vector<std::string> arguments, std::string return_type)
            : type(type), arguments(arguments), return_type(return_type) {}
};

// The block class represents a block
// of nuua code. It is used by the analizer
// to determine the current state of a given block.
class Block
{
    public:
        // Stores the variable name and the type of it.
        // var => {type: ..., int_representation: ..., arguments: ..., return_type: ...}
        std::unordered_map<std::string, BlockVariableType> variables;
        // Gets a variable from the current block or returns nullptr.
        BlockVariableType *get_variable(std::string &name);
        Block()
            : variables({}) {};
        Block(std::unordered_map<std::string, BlockVariableType> variables)
            : variables(variables) {};
};

#endif
