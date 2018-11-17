#include "../include/tokens.hpp"
#include <stdio.h>

std::vector<std::string> Token::tokenNames = {
    "TOKEN_NEW_LINE",
    "TOKEN_LEFT_PAREN",
    "TOKEN_RIGHT_PAREN",
    "TOKEN_LEFT_BRACE",
    "TOKEN_RIGHT_BRACE",
    "TOKEN_COMMA",
    "TOKEN_DOT",
    "TOKEN_MINUS",
    "TOKEN_PLUS",
    "TOKEN_SLASH",
    "TOKEN_STAR",
    "TOKEN_RIGHT_ARROW",
    "TOKEN_LEFT_ARROW",
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
    "TOKEN_NUMBER",
    "TOKEN",
    "TOKEN_AND",
    "TOKEN_CLASS",
    "TOKEN_ELSE",
    "TOKEN_TRUE",
    "TOKEN_FALSE",
    "TOKEN_WHILE",
    "TOKEN_FOR",
    "TOKEN_IF",
    "TOKEN_NONE",
    "TOKEN_SELF",
    "TOKEN_EOF",
    "TOKEN_PERCENT",
    "TOKEN_LEFT_SQUARE",
    "TOKEN_RIGHT_SQUARE",
    "TOKEN_BIG_RIGHT_ARROW",
    "TOKEN_COLON",
    "TOKEN_UNROLL",
    "TOKEN_RETURN"
};

void Token::debug_token()
{
    printf("%s\n", Token::tokenNames[this->type].c_str());
}

void Token::debug_token(TokenType token)
{
    printf("%s\n", Token::tokenNames[token].c_str());
}

void Token::debug_tokens(std::vector<Token> tokens)
{
    for (auto token : tokens) {
        printf("%s ", tokenNames[token.type].c_str());
        if (token.type == TOKEN_NEW_LINE) {
            printf("\n");
        }
    }
    printf("\n");
}

void Token::debug_tokens(std::vector<TokenType> tokens)
{
    for (auto token : tokens) {
        printf("%s ", tokenNames[token].c_str());
        if (token == TOKEN_NEW_LINE) printf("\n");
    }
    printf("\n");
}
