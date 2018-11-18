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

class Parser
{
    Token *current;

    static const std::unordered_map<char, char> escapedChars;

    Token consume(TokenType type, const char* message);
    bool match(TokenType token);
    bool matchAny(std::vector<TokenType> tokens);
    std::vector<Statement *> getBlockBody();
    std::string toString(Token token);
    bool isFunction();
    Expression *function();
    Expression *list();
    Expression *dictionary();
    Expression *primary();
    Expression *finishCall(Expression *callee);
    Expression *finishAccess(Expression *item);
    Expression *call();
    Expression *unary();
    Expression *mulDivMod();
    Expression *addition();
    Expression *comparison();
    Expression *equality();
    Expression *andOperator();
    Expression *orOperator();
    Expression *assignment();
    Expression *expression();
    Statement *expressionStatement();
    Statement *ifStatement();
    Statement *whileStatement();
    Statement *unrollStatement();
    Statement *statement();

    static void debug_rules(std::vector<Rule> rules);
    static void debug_rules(std::vector<Statement *> rules);

    public:
        std::vector<Statement *> parse(const char *source);
};

#endif
