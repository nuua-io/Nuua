/**
 * |------------|
 * | Nuua Lexer |
 * |------------|
 *
 * Copyright 2018 Erik Campobadal <soc@erik.cat>
 * https://nuua.io
 */

#include "../include/lexer.hpp"
#include "../../Logger/include/logger.hpp"
#include <string.h>

#define ADD_TOKEN(token) (tokens.push_back(this->make_token(token)))
#define TOK_LENGTH() ((int) (this->current - this->start))
#define IS_AT_END() (*this->current == '\0')
#define NEXT() (*(this->current++))
#define PEEK() (*this->current)
#define PEEK_ON(offset) (*(this->current + (offset)))
#define IS_DIGIT(character) ((character) >= '0' && (character) <= '9')
#define IS_ALPHA(character) (((character) >= 'a' && (character) <= 'z') || ((character) >= 'A' && (character) <= 'Z') || (character) == '_')
#define IS_ALPHANUM(character) (IS_ALPHA(character) || IS_DIGIT(character))

const std::unordered_map<std::string, TokenType> Lexer::reserved_words = {
    { "true", TOKEN_TRUE },
    { "false", TOKEN_FALSE },
    { "as", TOKEN_AS },
    { "or", TOKEN_OR },
    { "and", TOKEN_AND },
    { "if", TOKEN_IF },
    { "else", TOKEN_ELSE },
    { "for", TOKEN_FOR },
    { "while", TOKEN_WHILE },
    { "return", TOKEN_RETURN },
    { "print", TOKEN_PRINT },
    { "class", TOKEN_CLASS },
    { "fun", TOKEN_FUN },
    { "self", TOKEN_SELF },
    { "import", TOKEN_IMPORT },
    { "from", TOKEN_FROM }
};

const std::string Lexer::token_error()
{
    return std::string("Unexpected token '") + *this->start + "'";
}

Token Lexer::make_token(TokenType type)
{
    const char *start;
    uint32_t length;

    if (type == TOKEN_STRING) {
        // The '"' must be eliminated in both sides.
        start = this->start + 1; // +(The initial '"")
        length = TOK_LENGTH() - 2; // -(The initial '"' + The final '"')
    } else {
        start = this->start;
        length = TOK_LENGTH();
    }

    this->start = *this->current == ' ' ? this->current + 1 : this->current;

    return Token(type, start, length, this->line);
}

bool Lexer::match(const char c)
{
    if (IS_AT_END() || PEEK() != c) return false;

    this->current++;

    return true;
}

TokenType Lexer::is_string(bool simple)
{
    while (PEEK() != (simple ? '\'' : '"') && !IS_AT_END()) {
        if (PEEK() == '\n') this->line++;
        else if (PEEK() == '\\') { NEXT(); }
        NEXT();
    }

    if (IS_AT_END()) {
        logger->error("Unterminated string literal", this->line);
        exit(EXIT_FAILURE);
    }

    NEXT(); // The " itelf

    return TOKEN_STRING;
}

TokenType Lexer::is_number()
{
    while (IS_DIGIT(PEEK())) NEXT();

    if (PEEK() == '.' && IS_DIGIT(PEEK_ON(1))) {
        NEXT(); // The . itelf
        while (IS_DIGIT(PEEK())) NEXT();
        return TOKEN_FLOAT;
    }

    return TOKEN_INTEGER;
}

TokenType Lexer::is_identifier()
{
    while (IS_ALPHANUM(PEEK())) NEXT();

    std::string key;
    for (int i = 0; i < TOK_LENGTH(); i++) key += *(this->start + i);

    /*
    printf(
        "Word is: '%s' - Found: %d - Number: %d\n",
        key.c_str(),
        Lexer::reserved_words.find(key) != Lexer::reserved_words.end(),
        (Lexer::reserved_words.find(key) != Lexer::reserved_words.end()) ? Lexer::reserved_words.at(key) : TOKEN_IDENTIFIER
    );
    */

    return (Lexer::reserved_words.find(key) != Lexer::reserved_words.end()) ? Lexer::reserved_words.at(key) : TOKEN_IDENTIFIER;
}

std::vector<Token> Lexer::scan(const char *source)
{
    logger->info("Started scanning...");

    this->start = source;
    this->current = source;
    this->line = 1;

    std::vector<Token> tokens;

    while (!IS_AT_END()) {
        switch (char c = NEXT()) {
            case ' ': { this->start = this->current; break; }
            case '\r': case '\t': { break; }
            case '\n': { this->line++; ADD_TOKEN(TOKEN_NEW_LINE); break; }
            case '#': { while (PEEK() != '\n' && !IS_AT_END()) NEXT(); break; }
            case '(': { ADD_TOKEN(TOKEN_LEFT_PAREN); break; }
            case ')': { ADD_TOKEN(TOKEN_RIGHT_PAREN); break; }
            case '{': { ADD_TOKEN(TOKEN_LEFT_BRACE); break; }
            case '}': { ADD_TOKEN(TOKEN_RIGHT_BRACE); break; }
            case '[': { ADD_TOKEN(TOKEN_LEFT_SQUARE); break; }
            case ']': { ADD_TOKEN(TOKEN_RIGHT_SQUARE); break; }
            case ',': { ADD_TOKEN(TOKEN_COMMA); break; }
            case '.': { ADD_TOKEN(TOKEN_DOT); break; }
            case ':': { ADD_TOKEN(TOKEN_COLON); break; }
            case '-': {
                if (this->match('>')) { ADD_TOKEN(TOKEN_RIGHT_ARROW); break; }
                ADD_TOKEN(TOKEN_MINUS); break;
            }
            case '+': { ADD_TOKEN(TOKEN_PLUS); break; }
            case '/': { ADD_TOKEN(TOKEN_SLASH); break; }
            case '*': { ADD_TOKEN(TOKEN_STAR); break; }
            case '=': {
                if (this->match('=')) { ADD_TOKEN(TOKEN_EQUAL_EQUAL); break; }
                else if (this->match('>')) { ADD_TOKEN(TOKEN_BIG_RIGHT_ARROW); break; }
                ADD_TOKEN(TOKEN_EQUAL); break;
            }
            case '"': { ADD_TOKEN(this->is_string(false)); break; }
            case '\'': { ADD_TOKEN(this->is_string(true)); break; }
            case '<': {
                if (this->match('-')) { ADD_TOKEN(TOKEN_LEFT_ARROW); break; }
                ADD_TOKEN(this->match('=') ? TOKEN_LOWER_EQUAL : TOKEN_LOWER); break;
            }
            case '>': { ADD_TOKEN(this->match('=') ? TOKEN_HIGHER_EQUAL : TOKEN_HIGHER); break; }
            default: {
                if (IS_DIGIT(c)) { ADD_TOKEN(this->is_number()); break; }
                else if (IS_ALPHA(c)) { ADD_TOKEN(this->is_identifier()); break; }
                logger->error(this->token_error(), this->line);
                exit(EXIT_FAILURE);
            }
        }
    }

    tokens.push_back(this->make_token(TOKEN_EOF));

    #if DEBUG
        Token::debug_tokens(tokens);
    #endif

    logger->success("Scanning complete");

    return tokens;
}

#undef ADD_TOKEN
#undef TOK_LENGTH
#undef IS_AT_END
#undef NEXT
#undef PEEK
#undef PEEK_ON
#undef IS_DIGIT
#undef IS_ALPHA
#undef IS_ALPHANUM
