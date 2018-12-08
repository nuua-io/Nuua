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
#include "rules.hpp"

// Base parser class for nuua.
class Parser
{
    // Stores a pointer to the current token beeing parsed.
    Token *current;

    // Contains the escaped chars of the language.
    static const std::unordered_map<char, char> escaped_chars;

    Token consume(TokenType type, const char* message);
    bool match(TokenType token);
    bool match_any(std::vector<TokenType> tokens);
    std::vector<Statement *> get_block_body();
    std::string to_string(Token token);
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
    Statement *if_statement();
    Statement *while_statement();
    Statement *statement();

    static void debug_rules(std::vector<Rule> rules);
    static void debug_rules(std::vector<Statement *> rules);

    public:
        std::vector<Statement *> parse(const char *source);
};

#endif
