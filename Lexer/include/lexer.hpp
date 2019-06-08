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
    // Stores the start char of the token being scanned.
    const char *start;
    // Stores the current char of the token being scanned.
    const char *current;
    // Stores the current line in the source file.
    line_t line;
    // Stores the current column in the source file.
    column_t column;
    // Stores a list of reserved words for the identifiers.
    static const std::unordered_map<std::string, TokenType> reserved_words;
    // Generates a token error.
    const std::string token_error() const;
    // Build the token and set the start char to the current one.
    Token make_token(TokenType type);
    // Helper to build tokens.
    bool match(const char c);
    TokenType is_string(bool simple);
    TokenType is_number();
    TokenType is_identifier();
    // Reads a file and stores the contents in the current instance.
    void read_from_file(const std::shared_ptr<const std::string> &file);
    public:
        // Stores the source code of the file.
        std::unique_ptr<std::string> source;
        // Scans the source and stores the tokens.
        void scan(std::unique_ptr<std::vector<Token>> &tokens);
        // Initializes a lexer given a file name.
        Lexer(const std::shared_ptr<const std::string> &file);
};

#endif
