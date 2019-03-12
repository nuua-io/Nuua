/**
 * |-------------|
 * | Nuua Tokens |
 * |-------------|
 *
 * Copyright 2019 Erik Campobadal <soc@erik.cat>
 * https://nuua.io
 */

#ifndef TOKEN_HPP
#define TOKEN_HPP

#include <vector>
#include <unordered_map>
#include <string>
#include <stdint.h>

typedef enum : uint8_t {
    TOKEN_NEW_LINE, // \n
    TOKEN_LEFT_PAREN, // (
    TOKEN_RIGHT_PAREN, // )
    TOKEN_LEFT_BRACE, // {
    TOKEN_RIGHT_BRACE, // }
    TOKEN_COMMA, // ,
    TOKEN_DOT, // .
    TOKEN_MINUS, // -
    TOKEN_PLUS, // +
    TOKEN_SLASH, // /
    TOKEN_STAR, // *
    TOKEN_RIGHT_ARROW, // ->
    TOKEN_LEFT_ARROW, // <-
    TOKEN_BANG, // !
    TOKEN_BANG_EQUAL, // !=
    TOKEN_EQUAL, // =
    TOKEN_EQUAL_EQUAL, // ==
    TOKEN_HIGHER, // >
    TOKEN_HIGHER_EQUAL, // >=
    TOKEN_LOWER, // <
    TOKEN_LOWER_EQUAL, // <=
    TOKEN_IDENTIFIER, // <identifier>
    TOKEN_STRING, // <string>
    TOKEN_INTEGER, // <integer number>
    TOKEN_FLOAT, // <integer number>.<integer number>
    TOKEN_AS, // as
    TOKEN_OR, // or
    TOKEN_AND, // and
    TOKEN_CLASS, // class
    TOKEN_FUN, // fun
    TOKEN_ELSE, // else
    TOKEN_TRUE, // true
    TOKEN_FALSE, // false
    // TOKEN_FUN, // Not really needed
    TOKEN_WHILE, // while
    TOKEN_FOR, // for
    TOKEN_IF, // if
    // TOKEN_RETURN,
    // TOKEN_SUPER,
    TOKEN_SELF, // self
    TOKEN_EOF, // \0
    TOKEN_PERCENT, // %
    TOKEN_LEFT_SQUARE, // [
    TOKEN_RIGHT_SQUARE, // ]
    TOKEN_BIG_RIGHT_ARROW, // =>
    TOKEN_COLON, // :
    TOKEN_RETURN, // return
    TOKEN_PRINT, // print
    TOKEN_IMPORT, // import
    TOKEN_FROM, // from
} TokenType;

class Token
{
    public:
        TokenType type;
        const char *start;
        uint32_t length;
        uint32_t line;

        static std::vector<std::string> token_names;

        // Contains the escaped chars of the language.
        static const std::unordered_map<char, char> escaped_chars;

        Token(TokenType type, const char *start, uint32_t length, uint32_t line)
            : type(type), start(start), length(length), line(line) {}

        void debug_token();

        std::string to_string();

        static void debug_token(TokenType token);
        static void debug_tokens(std::vector<Token> tokens);
        static void debug_tokens(std::vector<TokenType> tokens);
};

#endif
