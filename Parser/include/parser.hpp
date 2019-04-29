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
#include "type.hpp"
#include "rules.hpp"

class Parser
{
    // Stores the current parsing file.
    const std::string *file;
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
    Expression *cast();
    Expression *multiplication();
    Expression *addition();
    Expression *comparison();
    Expression *equality();
    Expression *logical_and();
    Expression *logical_or();
    Expression *range();
    Expression *assignment();
    Expression *expression();
    // Statements
    Statement *fun_declaration();
    Statement *use_declaration();
    Statement *export_declaration();
    Statement *variable_declaration();
    Statement *print_statement();
    Statement *return_statement();
    Statement *if_statement();
    Statement *while_statement();
    Statement *for_statement();
    Statement *expression_statement();
    Statement *class_statement();
    Statement *statement(bool new_line = true);
    Statement *top_level_declaration();
    Statement *class_body_declaration();
    // Helpers
    void parameters(std::vector<Declaration *> *dest);
    std::vector<Expression *> arguments();
    std::vector<Statement *> body();
    std::vector<Statement *> class_body();
    Type *type(bool optional = true);
    public:
        // Debugging functions
        static void debug_rule(Rule rule);
        static void debug_rule(Statement *statement);
        static void debug_rules(std::vector<Rule> &rules);
        static void debug_rules(std::vector<Statement *> &rules);
        static void debug_ast(Expression *expression, uint16_t spacer = 0, bool print_spacer = true);
        static void debug_ast(Statement *statement, uint16_t spacer = 0);
        static void debug_ast(std::vector<Statement *> &statements, uint16_t spacer = 0);
        // Helper to format a path.
        static void format_path(std::string *path, const std::string *parent = nullptr);
        // Parses a given source code and returns the code.
        void parse(std::vector<Statement *> *code);
        // Creates a new parser and formats the path.
        Parser(const char *file);
        // Creates a new parser with a given formatted and initialized path.
        Parser(const std::string *file)
            : file(file) {}
};

#endif
