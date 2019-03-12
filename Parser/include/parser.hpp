/**
 * |-------------|
 * | Nuua Parser |
 * |-------------|
 *
 * Copyright 2019 Erik Campobadal <soc@erik.cat>
 * https://nuua.io
 */
#ifndef PARSER_HPP
#define PARSER_HPP

#include "../../Lexer/include/tokens.hpp"
#include "rules.hpp"

class Parser
{
    // Stores a pointer to the current token beeing parsed.
    Token *current;
    // Consumes a token and returns it for futher use.
    Token *consume(TokenType type, const char *message);
    // Returns true if the token type matches the current token.
    bool match(TokenType token);
    // Returns true if any of the given token types matches the current token.
    bool match_any(std::vector<TokenType> tokens);

    // Expressions
    Expression *primary();
    Expression *unary_postfix();
    Expression *unary_prefix();
    Expression *multiplication();
    Expression *addition();
    Expression *comparison();
    Expression *equality();
    Expression *logical_and();
    Expression *logical_or();
    Expression *assignment();
    Expression *expression();

    // Statements
    Statement *variable_declaration();
    Statement *expression_statement();
    Statement *statement();

    // Helpers
    std::vector<Statement *> parameters();
    std::vector<Expression *> arguments();
    std::string type();

    public:
        // Debugging functions
        static void debug_rule(Rule rule);
        static void debug_rule(Statement *statement);
        static void debug_rules(std::vector<Rule> &rules);
        static void debug_rules(std::vector<Statement *> &rules);
        static void debug_ast(Expression *expression);
        static void debug_ast(Statement *statement);
        static void debug_ast(std::vector<Statement *> &statements);
        // Parses a given source code and returns the AST.
        std::vector<Statement *> parse(const char *source);
};

#endif
