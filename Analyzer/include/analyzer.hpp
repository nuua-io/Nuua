#ifndef ANALIZER_HPP
#define ANALIZER_HPP

#include "../../Parser/include/rules.hpp"
#include "../../Parser/include/block.hpp"

// The base nuua analyzer and optimizer.
//
// Analizes:
// - Declared variables
// - Function return value match
// - Argument type match
// - Assignment type match
// - List / Dictionary index type
// Optimizes:
// -
class Analyzer
{
    // Stores the current analized block.
    std::vector<Block> blocks;

    // Analyzes a given statement.
    void analyze(Statement *rule);

    // Analyzes a given expression.
    void analyze(Expression *rule);

    // Analyzes a block of statements. Additional arguments may be used
    // to initialize the block variable types.
    Block analyze(std::vector<Statement *> &block, std::vector<Statement *> arguments = {}, std::string return_type = "");

    // Stops the execution if the current block does not
    // have the given variable declared. It also returns
    // it's address to use it if nessesary.
    BlockVariableType *must_have(std::string name, uint32_t line);

    // Declares a variable given a name and a type.
    void declare(std::string name, std::string type, Expression *initializer);

    public:
        // Stores the block of code to use.
        std::vector<Statement *> code;

        // Stores the main block.
        Block main_block;

        // Analizes the input string and finds any errors.
        void analyze(const char *source);

        // Optimizes the input AST.
        void optimize();
};

#endif
