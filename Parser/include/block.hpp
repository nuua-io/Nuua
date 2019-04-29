#ifndef BLOCK_HPP
#define BLOCK_HPP

#include <unordered_map>
#include <vector>
#include <utility>

// Forward declarations
class Node;
class Type;

// The BlockVariableType determines the
// representation of a variable type
// for the block analyzer.
class BlockVariableType
{
    public:
        // Represents the variable type
        Type *type;
        // Stores the AST node where this variable is.
        Node *node;
        // Represents the registers where it's stored.
        uint32_t reg;
        // Determines in the variable is exported. (only applies to TLDs).
        bool exported;
        // Represents the last use of the variable (Variable life)
        Node *last_use = nullptr;
        BlockVariableType() {};
        BlockVariableType(Type *type, Node *node, bool exported = false)
            : type(type), node(node), exported(exported) {}
};

// The block class represents a block
// of nuua code. It is used by the analizer
// to determine the current state of a given block.
class Block
{
    public:
        // Stores the variable name and the type of it.
        std::unordered_map<std::string, BlockVariableType> variables;
        // Gets a variable from the current block or returns nullptr.
        BlockVariableType *get_variable(std::string &name);
        void set_variable(std::string name, BlockVariableType type);
        bool is_exported(std::string &name);
        void debug();
        static BlockVariableType *get_single_variable(std::string &name, std::vector<Block *> *blocks);
        Block()
            : variables({}) {};
        Block(std::unordered_map<std::string, BlockVariableType> variables)
            : variables(variables) {};
};

#endif
