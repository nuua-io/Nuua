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
    // Stores the file where the tokens are beeing scanned.
    const std::string *file;
    const char *start;
    const char *current;
    line_t line;
    column_t column;

    static const std::unordered_map<std::string, TokenType> reserved_words;

    const std::string token_error();
    Token make_token(TokenType type);
    bool match(const char c);
    TokenType is_string(bool simple);
    TokenType is_number();
    TokenType is_identifier();
    void read_from_file(std::string *dest, const std::string *file);
    public:
        // Stores the source code of the file.
        std::string *source = nullptr;
        // Scans the source and stores the tokens.
        void scan(std::vector<Token> *tokens);
        // Initializes a lexer given a file name.
        Lexer(const std::string *file);
        ~Lexer();
};

#endif
