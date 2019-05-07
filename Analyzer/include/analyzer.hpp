#ifndef ANALIZER_HPP
#define ANALIZER_HPP

#include "../../Parser/include/rules.hpp"
#include "../../Parser/include/block.hpp"
#include "module.hpp"

// The base nuua analyzer and optimizer.
//
// Analyzes:
// - Declared variables
// - Function return value match
// - Non-assignment for functions without return
// - Argument type match
// - Assignment type match
// - List / Dictionary index type
// - Variable lifetime (last_use)
// - Use / Export declarations.
// - Check for iterator (must be list / dict)
// Optimizes:
// -
class Analyzer
{
    // Stores the main file name.
    const char *file;
    public:
        // Creates an analyzer given the file name.
        Analyzer(const char *file);
        // Analyzes the code. The result AST is stored in the destination. It returns the main module.
        Block analyze(std::vector<Statement *> *destination);
};

#endif
