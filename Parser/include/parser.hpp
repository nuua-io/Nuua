/**
 * |-------------|
 * | Nuua Parser |
 * |-------------|
 *
 * Copyright 2018 Erik Campobadal <soc@erik.cat>
 * https://nuua.io
 */
#ifndef PARSER_HPP
#define PARSER_HPP

#include "../../Lexer/include/tokens.hpp"
// #include "rules.hpp" // Included already on parser_optimizer
#include "parser_optimizer.hpp"

// Base parser class for nuua.
class Parser
{
    // Stores a pointer to the current token beeing parsed.
    Token *current;

    // Stores the AST optimizer for the parser.
    ParserOptimizer optimizer;

    Token consume(TokenType type, const char* message);
    bool match(TokenType token);
    bool match_any(std::vector<TokenType> tokens);
    std::vector<Statement *> get_block_body();
    bool is_function();

    // Parser basic operations.
    Expression *function();
    Expression *list();
    Expression *dictionary();
    Expression *primary();
    Expression *finish_call(Expression *callee);
    Expression *finish_access(Expression *item);
    Expression *call();
    Expression *unary();
    Expression *mul_div_mod();
    Expression *addition();
    Expression *comparison();
    Expression *equality();
    Expression *and_operator();
    Expression *or_operator();
    Expression *assignment();
    Expression *expression();
    Statement *expression_statement();
    Statement *declaration_statement();
    Statement *return_statement();
    Statement *if_statement();
    Statement *while_statement();
    Statement *statement(bool new_line_ending = true);

    static void debug_rules(std::vector<Rule> rules);
    static void debug_rules(std::vector<Statement *> rules);

    public:
        std::vector<Statement *> parse(const char *source);
};

#endif
