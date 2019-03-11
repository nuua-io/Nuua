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
    // Gets a written type.
    std::string get_type();

    // Expressions
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

    public:
        // Parses a given source code and returns the AST.
        std::vector<Statement *> parse(const char *source);
};

#endif
