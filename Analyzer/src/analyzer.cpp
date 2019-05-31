#include "../include/analyzer.hpp"
#include "../../Parser/include/parser.hpp"
#include "../../Logger/include/logger.hpp"
#include <algorithm>

Analyzer::Analyzer(const char *file)
{
    // Parse the code AST.
    this->file = file;
}

std::shared_ptr<Block> Analyzer::analyze(std::shared_ptr<std::vector<std::shared_ptr<Statement>>> &destination)
{
    printf("----> Analyzer\n");
    // Parses the code.
    auto p = Parser(this->file);
    p.parse(destination);
    Parser::debug_ast(*destination);
    // Create the main module
    Module m = Module(destination->front()->file);
    // Analyze the module
    m.analyze(destination, true);
    // Return the main module
    printf("----> !Analyzer\n");
    return m.main_block;
}
