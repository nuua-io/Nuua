/**
 * |-------------|
 * | Nuua Parser |
 * |-------------|
 *
 * Copyright 2018 Erik Campobadal <soc@erik.cat>
 * https://nuua.io
 */
#ifndef PARSER_OPTIMIZER_HPP
#define PARSER_OPTIMIZER_HPP

#include "rules.hpp"

// Base optimizer for the nuua parser.
class ParserOptimizer
{
    // Stores the number of optimizations done.
    uint16_t number;

    // Optimize individual elements.
    void optimize(Statement *statement);
    void optimize(Expression *expression);

    public:
        // Optimizes the AST.
        void optimize(std::vector<Statement *> *ast);
};

#endif
