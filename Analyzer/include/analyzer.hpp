#ifndef ANALIZER_HPP
#define ANALIZER_HPP

#include "../../Parser/include/rules.hpp"
#include "block.hpp"

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
    void analyze(std::vector<Statement *> block, std::vector<Statement *> arguments = {}, std::string return_type = "");

    // Stops the execution if the current block does not
    // have the given variable declared.
    void must_have(std::string name, uint32_t line);

    // Declares a variable given a name and a type.
    void declare(std::string name, std::string type, Expression *initializer);

    // Replaces the variable of the given type
    // into the current block's variable types.
    //
    // <var> - Replaces <var> with the type of var.
    // <:var:> - Replaces <:var:> with the return type of var.
    // <|var|> - Replaces <|var|> with the inner type of var.
    void replace_types(std::string &dest);

    // A generic implementation of find and replace
    void find_replace(std::string &dest, const std::string &find, const std::string &replace);

    public:
        // Stores the block of code to use.
        std::vector<Statement *> code;

        // Analizes the input string and finds any errors.
        Analyzer *analyze(const char *source);

        // Optimizes the input AST.
        Analyzer *optimize();
};

#endif
