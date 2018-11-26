/**
 * |------------|
 * | Nuua Lexer |
 * |------------|
 *
 * Copyright 2019 Erik Campobadal <soc@erik.cat>
 * https://nuua.io
 */

#ifndef LEXER_HPP
#define LEXER_HPP

#include "tokens.hpp"
#include <unordered_map>

class Lexer
{
    const char *start;
    const char *current;
    uint32_t line;

    static const std::unordered_map<std::string, TokenType> reservedWords;

    const std::string token_error();
    Token make_token(TokenType type);
    bool match(const char c);
    TokenType is_string(bool simple);
    TokenType is_number();
    TokenType is_identifier();

    public:
        std::vector<Token> scan(const char *source);
};

#endif
