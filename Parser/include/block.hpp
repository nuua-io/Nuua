#ifndef BLOCK_HPP
#define BLOCK_HPP

#include <unordered_map>
#include <vector>
#include <utility>
#include <memory>

typedef uint32_t register_t;

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
        std::shared_ptr<Type> type;
        // Stores the AST node where this variable is.
        std::shared_ptr<Node> node;
        // Represents the registers where it's stored.
        register_t reg = 0;
        // Determines in the variable is exported. (only applies to TLDs).
        bool exported = false;
        // Represents the last use of the variable (Variable life)
        std::shared_ptr<Node> last_use;
        BlockVariableType() {};
        BlockVariableType(const std::shared_ptr<Type> &type, const std::shared_ptr<Node> &node, bool exported = false)
            : type(type), node(node), exported(exported) { }
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
        BlockVariableType *get_variable(const std::string &name);
        // Sets a variable and returns it's reference.
        void set_variable(const std::string &name, const BlockVariableType &var);
        bool is_exported(const std::string &name);
        bool has(const std::string &name);
        void debug();
        static BlockVariableType *get_single_variable(const std::string &name, const std::vector<std::shared_ptr<Block>> *blocks);
};

#endif
