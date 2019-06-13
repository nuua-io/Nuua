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
    std::shared_ptr<const std::string> file;
    // Stores a pointer to the current token beeing parsed.
    Token *current = nullptr;
    // Consumes a token and returns it for futher use.
    Token *consume(const TokenType type, const std::string &message);
    // Returns true if the token type matches the current token.
    bool match(const TokenType token);
    // Returns true if any of the given token types matches the current token.
    bool match_any(const std::vector<TokenType> &tokens);
    // Expressions
    std::shared_ptr<Expression> primary();
    std::shared_ptr<Expression> unary_postfix();
    std::shared_ptr<Expression> unary_prefix();
    std::shared_ptr<Expression> cast();
    std::shared_ptr<Expression> multiplication();
    std::shared_ptr<Expression> addition();
    std::shared_ptr<Expression> comparison();
    std::shared_ptr<Expression> equality();
    std::shared_ptr<Expression> logical_and();
    std::shared_ptr<Expression> logical_or();
    std::shared_ptr<Expression> range();
    std::shared_ptr<Expression> assignment();
    std::shared_ptr<Expression> expression();
    // Statements
    std::shared_ptr<Statement> fun_declaration();
    std::shared_ptr<Statement> use_declaration();
    std::shared_ptr<Statement> export_declaration();
    std::shared_ptr<Statement> variable_declaration();
    std::shared_ptr<Statement> print_statement();
    std::shared_ptr<Statement> delete_statement();
    std::shared_ptr<Statement> return_statement();
    std::shared_ptr<Statement> if_statement();
    std::shared_ptr<Statement> while_statement();
    std::shared_ptr<Statement> for_statement();
    std::shared_ptr<Statement> expression_statement();
    std::shared_ptr<Statement> class_statement();
    std::shared_ptr<Statement> statement(bool new_line = true);
    std::shared_ptr<Statement> top_level_declaration(const bool expect_new_line = true);
    std::shared_ptr<Statement> class_body_declaration();
    // Helpers
    void parameters(std::vector<std::shared_ptr<Declaration>> *dest);
    void object_arguments(std::unordered_map<std::string, std::shared_ptr<Expression>> &arguments);
    std::vector<std::shared_ptr<Expression>> arguments();
    std::vector<std::shared_ptr<Statement>> body();
    std::vector<std::shared_ptr<Statement>> class_body();
    std::shared_ptr<Type> type(bool optional = true);
    public:
        // Debugging functions
        static void debug_rule(const Rule rule);
        static void debug_rule(const std::shared_ptr<Statement> &statement);
        static void debug_rules(const std::vector<Rule> &rules);
        static void debug_rules(const std::vector<std::shared_ptr<Statement>> &rules);
        static void debug_ast(const std::shared_ptr<Expression> &expression, const uint16_t spacer = 0, const bool print_spacer = true);
        static void debug_ast(const std::shared_ptr<Statement> &statement, const uint16_t spacer = 0);
        static void debug_ast(const std::vector<std::shared_ptr<Statement>> &statements, const uint16_t spacer = 0);
        // Helper to format a path.
        static void format_path(std::string &path, const std::shared_ptr<const std::string> &parent = std::shared_ptr<const std::string>());
        // Parses a given source code and returns the code.
        void parse(std::shared_ptr<std::vector<std::shared_ptr<Statement>>> &code);
        // Creates a new parser and formats the path.
        Parser(const char *file);
        // Creates a new parser with a given formatted and initialized path.
        Parser(std::shared_ptr<const std::string> &file)
            : file(file) {}
};

#endif
