#ifndef BLOCK_HPP
#define BLOCK_HPP

#include <unordered_map>
#include <vector>
#include <utility>
#include <memory>

typedef uint32_t reg_t;

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
        reg_t reg = 0;
        // Determines in the variable is exported. (only applies to TLDs).
        bool exported = false;
        // Determines in the variable is a method. (only applies to functions).
        bool is_method = false;
        // Represents the last use of the variable (Variable life)
        std::shared_ptr<Node> last_use;
        BlockVariableType() {};
        BlockVariableType(
            const std::shared_ptr<Type> &type,
            const std::shared_ptr<Node> &node,
            const bool exported = false,
            const bool is_method = false
        ) : type(type), node(node), exported(exported), is_method(is_method) { }
};

class Block; // Forward declaration.
class BlockClassType
{
    public:
        std::shared_ptr<Block> block;
        bool exported = false;
        // Stores the AST node where this variable is.
        std::shared_ptr<Node> node;
        BlockClassType() {}
        BlockClassType(
            const std::shared_ptr<Block> &block,
            const std::shared_ptr<Node> &node,
            const bool exported = false
        ) : block(block), node(node), exported(exported) {}
};

// The block class represents a block
// of nuua code. It is used by the analizer
// to determine the current state of a given block.
class Block
{
    public:
        // Stores the variable name and the type of it.
        std::unordered_map<std::string, BlockVariableType> variables;
        // Stores the custom types of the block.
        std::unordered_map<std::string, BlockClassType> classes;
        // Gets a variable from the current block or returns nullptr.
        BlockVariableType *get_variable(const std::string &name);
        // Gets a class from the current block or returns nullptr.
        BlockClassType *get_class(const std::string &name);
        // Sets a variable.
        void set_variable(const std::string &name, const BlockVariableType &var);
        // Sets a class.
        void set_class(const std::string &name, const BlockClassType &c);
        // Determines if a variable is exported.
        bool is_exported(const std::string &name);
        // Determines if a class is exported.
        bool is_exported_class(const std::string &name);
        // Determines if the block have a variable.
        bool has(const std::string &name);
        // Determines if the block have a class.
        bool has_class(const std::string &name);
        // Debug the block by printing it to the screen.
        void debug() const;
        // Helper to get a single variable out of a list of blocks.
        // It iterates through it starting from the end till the front.
        static BlockVariableType *get_single_variable(const std::string &name, const std::vector<std::shared_ptr<Block>> *blocks);
};

#endif
