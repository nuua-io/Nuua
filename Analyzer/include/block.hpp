#ifndef BLOCK_HPP
#define BLOCK_HPP

#include <unordered_map>
#include <vector>
#include <utility>

// The BlockVariableType determines the
// representation of a variable type
// for the block analyzer.
class BlockVariableType
{
    public:
        // Represents the variable type
        std::string type;

        // Represents the variable arguments (FUNCTION ARGUMENTS)
        std::vector<std::string> arguments;

        // Represents the variable return type (FUNCTION RETURN TYPE)
        std::string return_type;

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
        // var => {type: ..., arguments: ..., return_type: ...}
        std::unordered_map<std::string, BlockVariableType> variables;

        BlockVariableType *get_variable(std::string &name);
};

#endif
