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
#include <memory>
#include <utility>

class Lexer
{
    // Stores the file where the tokens are beeing scanned.
    std::shared_ptr<const std::string> file;
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
    void read_from_file(std::unique_ptr<std::string> &dest, std::shared_ptr<const std::string> &file);
    public:
        // Stores the source code of the file.
        std::unique_ptr<std::string> source;
        // Scans the source and stores the tokens.
        void scan(std::unique_ptr<std::vector<Token>> &tokens);
        // Initializes a lexer given a file name.
        Lexer(std::shared_ptr<const std::string> &file);
};

#endif
