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
#include <fstream>

#define ADD_TOKEN(token) (tokens->push_back(this->make_token(token)))
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
    { "use", TOKEN_USE },
    { "from", TOKEN_FROM },
    { "elif", TOKEN_ELIF },
    { "in", TOKEN_IN },
    { "export", TOKEN_EXPORT }
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
        logger->add_entity(this->file, this->line, "Unterminated string literal");
        exit(logger->crash());
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

void Lexer::read_from_file(std::string *dest, const std::string *file)
{
    std::ifstream file_stream = std::ifstream(file->c_str());
    if (!file_stream.is_open()) {
        logger->add_entity(this->file, this->line, "Unable to open file '" + *file + "'");
        exit(logger->crash());
    }

    *dest = std::string((std::istreambuf_iterator<char>(file_stream)), (std::istreambuf_iterator<char>()));
}

void Lexer::scan(std::vector<Token> *tokens)
{
    this->source = new std::string;
    this->read_from_file(this->source, this->file);

    this->start = this->source->c_str();
    this->current = this->start;
    this->line = 1;

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
            case '|': { ADD_TOKEN(TOKEN_STICK); break; }
            case '-': {
                if (this->match('>')) { ADD_TOKEN(TOKEN_RIGHT_ARROW); break; }
                ADD_TOKEN(TOKEN_MINUS); break;
            }
            case '+': { ADD_TOKEN(TOKEN_PLUS); break; }
            case '/': {
                if (this->match('/')) { while (PEEK() != '\n' && !IS_AT_END()) { NEXT(); this->start = this->current; } break; }
                ADD_TOKEN(TOKEN_SLASH); break;
            }
            case '*': { ADD_TOKEN(TOKEN_STAR); break; }
            case '=': {
                if (this->match('=')) { ADD_TOKEN(TOKEN_EQUAL_EQUAL); break; }
                else if (this->match('>')) { ADD_TOKEN(TOKEN_BIG_RIGHT_ARROW); break; }
                ADD_TOKEN(TOKEN_EQUAL); break;
            }
            case '"': { ADD_TOKEN(this->is_string(false)); break; }
            case '\'': { ADD_TOKEN(this->is_string(true)); break; }
            case '<': { ADD_TOKEN(this->match('=') ? TOKEN_LOWER_EQUAL : TOKEN_LOWER); break; }
            case '>': { ADD_TOKEN(this->match('=') ? TOKEN_HIGHER_EQUAL : TOKEN_HIGHER); break; }
            default: {
                if (IS_DIGIT(c)) { ADD_TOKEN(this->is_number()); break; }
                else if (IS_ALPHA(c)) { ADD_TOKEN(this->is_identifier()); break; }
                logger->add_entity(this->file, this->line, this->token_error());
                exit(logger->crash());
            }
        }
    }

    tokens->push_back(this->make_token(TOKEN_EOF));
}

Lexer::Lexer(const std::string *file)
{
    this->file = file;
}

Lexer::~Lexer()
{
    if (this->source) delete this->source;
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
