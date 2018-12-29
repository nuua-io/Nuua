#include "../include/parser_optimizer.hpp"

void ParserOptimizer::optimize(Statement *statement)
{
    switch (statement->rule) {
        default: { /* Ignore statement */ }
    }
}

void ParserOptimizer::optimize(Expression *expression)
{
    switch (expression->rule) {
        default: { /* Ignore expression */ }
    }
}

void ParserOptimizer::optimize(std::vector<Statement *> *ast)
{
    for (auto element : *ast) this->optimize(element);
}
