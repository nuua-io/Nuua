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
#include <string>
#include <unordered_map>

#define ADD_TOKEN(token) (tokens->push_back(make_token(token)))
#define TOK_LENGTH() ((int) (scanner->current - scanner->start))
#define IS_AT_END() (*scanner->current == '\0')
#define NEXT() (*(scanner->current++))
#define PEEK() (*scanner->current)
#define PEEK_ON(offset) (*(scanner->current + offset))
#define IS_DIGIT(character) (character >= '0' && character <= '9')
#define IS_ALPHA(character) (character >= 'a' && character <= 'z' || character >= 'A' && character <= 'Z' || character == '_')
#define IS_ALPHANUM(character) (IS_ALPHA(character) || IS_DIGIT(character))

static Scanner *scanner = new Scanner;

static const std::unordered_map<std::string, TokenType> reservedWords = {
    { "true", TOKEN_TRUE },
    { "false", TOKEN_FALSE },
    { "or", TOKEN_OR },
    { "and", TOKEN_AND },
    { "if", TOKEN_IF },
    { "else", TOKEN_ELSE },
    { "for", TOKEN_FOR },
    { "while", TOKEN_WHILE },
    { "none", TOKEN_NONE },
    { "return", TOKEN_RETURN },
    { "class", TOKEN_CLASS },
    { "self", TOKEN_SELF },
    { "unroll", TOKEN_UNROLL }
};

static const std::string token_error()
{
    return std::string("Unexpected token '") + *scanner->start + "'";
}

static Token make_token(TokenType type)
{
    Token token;
    token.type = type;
    token.line = scanner->line;

    if (type == TOKEN_STRING) {
        // The '"' must be eliminated in both sides.
        token.start = scanner->start + 1; // +(The initial '"")
        token.length = TOK_LENGTH() - 2; // -(The initial '"' + The final '"')
    } else {
        token.start = scanner->start;
        token.length = TOK_LENGTH();
    }

    scanner->start = *scanner->current == ' ' ? scanner->current + 1 : scanner->current;

    return token;
}

static bool match(const char c)
{
    if (IS_AT_END() || PEEK() != c) return false;
    scanner->current++;

    return true;
}

static TokenType is_string(bool simple)
{
    while (PEEK() != (simple ? '\'' : '"') && !IS_AT_END()) {
        if (PEEK() == '\n') scanner->line++;
        else if (PEEK() == '\\') { NEXT(); }
        NEXT();
    }

    if (IS_AT_END()) {
        logger->error("Unterminated string literal", scanner->line);
        exit(EXIT_FAILURE);
    }

    NEXT(); // The " itelf

    return TOKEN_STRING;
}

static TokenType is_number()
{
    while (IS_DIGIT(PEEK())) NEXT();

    if (PEEK() == '.' && IS_DIGIT(PEEK_ON(1))) {
        NEXT(); // The . itelf
        while (IS_DIGIT(PEEK())) NEXT();
    }

    return TOKEN_NUMBER;
}

static TokenType is_identifier()
{
    while (IS_ALPHANUM(PEEK())) NEXT();

    std::string key;
    for (int i = 0; i < TOK_LENGTH(); i++) {
        key += *(scanner->start + i);
    }

    /*
    printf(
        "Word is: '%s' - Found: %d - Number: %d\n",
        key.c_str(),
        reservedWords.find(key) != reservedWords.end(),
        (reservedWords.find(key) != reservedWords.end()) ? reservedWords.at(key) : TOKEN_IDENTIFIER
    );
    */

    return (reservedWords.find(key) != reservedWords.end()) ? reservedWords.at(key) : TOKEN_IDENTIFIER;
}

std::vector<Token> *scan(const char *source)
{
    logger->info("Started scanning...");

    scanner->start = source;
    scanner->current = source;
    scanner->line = 1;

    auto tokens = new std::vector<Token>;

    while (!IS_AT_END()) {
        switch (char c = NEXT()) {
            case ' ': { scanner->start = scanner->current; break; }
            case '\r': case '\t': { break; }
            case '\n': { scanner->line++; ADD_TOKEN(TOKEN_NEW_LINE); break; }
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
                if (match('>')) { ADD_TOKEN(TOKEN_RIGHT_ARROW); break; }
                ADD_TOKEN(TOKEN_MINUS); break;
            }
            case '+': { ADD_TOKEN(TOKEN_PLUS); break; }
            case '/': { ADD_TOKEN(TOKEN_SLASH); break; }
            case '*': { ADD_TOKEN(TOKEN_STAR); break; }
            case '=': {
                if (match('=')) { ADD_TOKEN(TOKEN_EQUAL_EQUAL); break; }
                else if (match('>')) { ADD_TOKEN(TOKEN_BIG_RIGHT_ARROW); break; }
                ADD_TOKEN(TOKEN_EQUAL); break;
            }
            case '"': { ADD_TOKEN(is_string(false)); break; }
            case '\'': { ADD_TOKEN(is_string(true)); break; }
            case '<': {
                if (match('-')) { ADD_TOKEN(TOKEN_LEFT_ARROW); break; }
                ADD_TOKEN(match('=') ? TOKEN_LOWER_EQUAL : TOKEN_LOWER); break;
            }
            case '>': { ADD_TOKEN(match('=') ? TOKEN_HIGHER_EQUAL : TOKEN_HIGHER); break; }
            default: {
                if (IS_DIGIT(c)) { ADD_TOKEN(is_number()); break; }
                else if (IS_ALPHA(c)) { ADD_TOKEN(is_identifier()); break; }
                logger->error(token_error(), scanner->line);
                exit(EXIT_FAILURE);
            }
        }
    }
    tokens->push_back(make_token(TOKEN_EOF));

    debug_tokens(*tokens);

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
