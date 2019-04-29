/**
 * |-------------|
 * | Nuua Tokens |
 * |-------------|
 *
 * Copyright 2018 Erik Campobadal <soc@erik.cat>
 * https://nuua.io
 */
#include "../include/tokens.hpp"
#include <stdio.h>

std::vector<std::string> Token::token_names = {
    "TOKEN_NEW_LINE",
    "TOKEN_LEFT_PAREN",
    "TOKEN_RIGHT_PAREN",
    "TOKEN_LEFT_BRACE",
    "TOKEN_RIGHT_BRACE",
    "TOKEN_COMMA",
    "TOKEN_DOT",
    "TOKEN_DOUBLE_DOT",
    "TOKEN_TRIPLE_DOT",
    "TOKEN_MINUS",
    "TOKEN_PLUS",
    "TOKEN_SLASH",
    "TOKEN_STAR",
    "TOKEN_RIGHT_ARROW",
    "TOKEN_BANG",
    "TOKEN_BANG_EQUAL",
    "TOKEN_EQUAL",
    "TOKEN_EQUAL_EQUAL",
    "TOKEN_HIGHER",
    "TOKEN_HIGHER_EQUAL",
    "TOKEN_LOWER",
    "TOKEN_LOWER_EQUAL",
    "TOKEN_IDENTIFIER",
    "TOKEN_STRING",
    "TOKEN_INTEGER",
    "TOKEN_FLOAT",
    "TOKEN_AS",
    "TOKEN_OR",
    "TOKEN_AND",
    "TOKEN_CLASS",
    "TOKEN_FUN",
    "TOKEN_ELSE",
    "TOKEN_TRUE",
    "TOKEN_FALSE",
    "TOKEN_WHILE",
    "TOKEN_FOR",
    "TOKEN_IF",
    "TOKEN_SELF",
    "TOKEN_EOF",
    "TOKEN_PERCENT",
    "TOKEN_LEFT_SQUARE",
    "TOKEN_RIGHT_SQUARE",
    "TOKEN_BIG_RIGHT_ARROW",
    "TOKEN_COLON",
    "TOKEN_RETURN",
    "TOKEN_PRINT",
    "TOKEN_USE",
    "TOKEN_FROM",
    "TOKEN_STICK",
    "TOKEN_ELIF",
    "TOKEN_IN",
    "TOKEN_EXPORT"
};
std::vector<std::string> Token::type_names = {
    "\\n",
    "(",
    ")",
    "{",
    "}",
    ",",
    ".",
    "..",
    "...",
    "-",
    "+",
    "/",
    "*",
    "->",
    "!",
    "!=",
    "=",
    "==",
    ">",
    ">=",
    "<",
    "<=",
    "<identifier>",
    "<string>",
    "<integer number>",
    "<integer number>.<integer number>",
    "as",
    "or",
    "and",
    "class",
    "fun",
    "else",
    "true",
    "false",
    "while",
    "for",
    "if",
    "self",
    "\\0",
    "%",
    "[",
    "]",
    "=>",
    ":", // :
    "return", // return
    "print", // print
    "use", // use
    "from", // from,
    "|", // |
    "elif", // ELIF
    "in", // in
    "export", // export
};

const std::unordered_map<char, char> Token::escaped_chars = {
    { '\\', '\\' },
    { '\'', '\'' },
    { '"', '"' },
    { 'n', '\n' },
    { 't', '\t' },
    { 'r', '\r' },
    { 'b', '\b' },
    { 'f', '\f' },
    { 'v', '\v' },
    { '0', '\0' }
};

std::string Token::to_string()
{
    std::string s;
    for (uint32_t i = 0; i < this->length; i++) {
        const char *c = this->start + i;
        s += *c;
        if (*c == '\\') {
            const char nc = *(c + 1);
            if (Token::escaped_chars.find(nc) != Token::escaped_chars.end()) {
                s.pop_back();
                s += Token::escaped_chars.at(nc);
                i++; // Increment i to avoid char repetition
            }
        }
    }

    return s;
}

std::string Token::to_type_string()
{
    return Token::type_names[this->type];
}

void Token::debug_token()
{
    printf("%s\n", Token::token_names[this->type].c_str());
}

void Token::debug_token(TokenType token)
{
    printf("%s\n", Token::token_names[token].c_str());
}

void Token::debug_tokens(std::vector<Token> &tokens)
{
    for (Token token : tokens) {
        printf("%s ", token_names[token.type].c_str());
        if (token.type == TOKEN_NEW_LINE) {
            printf("\n");
        }
    }
    printf("\n");
}

void Token::debug_tokens(std::vector<TokenType> &tokens)
{
    for (TokenType token : tokens) {
        printf("%s ", token_names[token].c_str());
        if (token == TOKEN_NEW_LINE) printf("\n");
    }
    printf("\n");
}
