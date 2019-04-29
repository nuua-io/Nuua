#include "../include/analyzer.hpp"
#include "../../Parser/include/parser.hpp"
#include "../../Logger/include/logger.hpp"
#include <algorithm>

Analyzer::Analyzer(const char *file)
{
    // Parse the code AST.
    this->file = file;
}

Module Analyzer::analyze(std::vector<Statement *> *destination)
{
    // Parses the code.
    Parser(this->file).parse(destination);
    // Parser::debug_ast(*destination);
    // Create the main module
    Module m = Module(destination->front()->file);
    // Analyze the module
    m.analyze(destination, true);
    // Return the main module
    return m;
}
