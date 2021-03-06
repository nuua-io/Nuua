/**
 * |------------|
 * | Nuua Lexer |
 * |------------|
 *
 * Copyright 2018 Erik Campobadal <soc@erik.cat>
 * https://nuua.io
 */

#include "../include/lexer.hpp"
#include <string.h>
#include <fstream>

#define ADD_TOKEN(token) (tokens->push_back(this->make_token(token)))
#define TOK_LENGTH() ((int) (this->current - this->start))
#define IS_AT_END() (*this->current == '\0')
#define NEXT() (*(this->current++))
#define SKIP() NEXT(); this->start = this->current;
#define PEEK() (*this->current)
#define PEEK_ON(offset) (*(this->current + (offset)))
#define IS_DIGIT(character) ((character) >= '0' && (character) <= '9')
#define IS_ALPHA(character) (((character) >= 'a' && (character) <= 'z') || ((character) >= 'A' && (character) <= 'Z') || (character) == '_')
#define IS_ALPHANUM(character) (IS_ALPHA(character) || IS_DIGIT(character))
#define ADD_LOG(msg) logger->add_entity(this->file, this->line, this->column, msg);

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
    { "use", TOKEN_USE },
    { "from", TOKEN_FROM },
    { "elif", TOKEN_ELIF },
    { "in", TOKEN_IN },
    { "export", TOKEN_EXPORT },
    { "delete", TOKEN_DELETE }
};

const std::string Lexer::token_error() const
{
    return std::string("Unexpected token '") + *this->start + "'";
}

Token Lexer::make_token(TokenType type)
{
    const char *start;
    uint32_t length, length_cpy = TOK_LENGTH();

    if (type == TOKEN_STRING) {
        // The '"' must be eliminated in both sides.
        start = this->start + 1; // +(The initial '"")
        length = length_cpy - 2; // -(The initial '"' + The final '"')
    } else {
        start = this->start;
        length = length_cpy;
    }

    this->start = *this->current == ' ' ? this->current + 1 : this->current;

    Token t = Token(type, start, length, this->line, this->column);

    this->column += length_cpy;

    return t;
}

bool Lexer::match(const char c)
{
    if (IS_AT_END() || PEEK() != c) return false;

    this->current++;

    return true;
}

TokenType Lexer::is_string(bool simple)
{
    std::shared_ptr<const std::string> f = this->file;
    line_t l = this->line;
    column_t c = this->column;
    while (PEEK() != (simple ? '\'' : '"') && !IS_AT_END()) {
        if (PEEK() == '\n') this->line++;
        else if (PEEK() == '\\') { NEXT(); }
        NEXT();
    }

    if (IS_AT_END()) {
        logger->add_entity(f, l, c, "Unterminated string literal");
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

void Lexer::read_from_file(const std::shared_ptr<const std::string> &file)
{
    std::ifstream file_stream = std::ifstream(file->c_str());
    if (!file_stream.is_open()) {
        ADD_LOG("Unable to open file '" + *file + "'");
        exit(logger->crash());
    }

    *this->source = std::string((std::istreambuf_iterator<char>(file_stream)), (std::istreambuf_iterator<char>()));
}

void Lexer::scan(std::unique_ptr<std::vector<Token>> &tokens)
{
    this->source = std::make_unique<std::string>();
    this->read_from_file(this->file);

    this->start = this->source->c_str();
    this->current = this->start;
    this->line = 1;
    this->column = 1;

    while (!IS_AT_END()) {
        switch (char c = NEXT()) {
            case ' ': { this->start = this->current; ++this->column; break; }
            case '\r': { break; }
            case '\t': { ++this->column; break; }
            case '\n': {
                ++this->line;
                ADD_TOKEN(TOKEN_NEW_LINE);
                // Remove subsequent '\n'
                while (!IS_AT_END() && PEEK() == '\n') {
                    SKIP();
                    ++this->line;
                }
                this->column = 1;
                break;
            }
            // case '#': { while (PEEK() != '\n' && !IS_AT_END()) NEXT(); break; }
            case '(': { ADD_TOKEN(TOKEN_LEFT_PAREN); break; }
            case ')': { ADD_TOKEN(TOKEN_RIGHT_PAREN); break; }
            case '{': { ADD_TOKEN(TOKEN_LEFT_BRACE); break; }
            case '}': { ADD_TOKEN(TOKEN_RIGHT_BRACE); break; }
            case '[': { ADD_TOKEN(TOKEN_LEFT_SQUARE); break; }
            case ']': { ADD_TOKEN(TOKEN_RIGHT_SQUARE); break; }
            case ',': { ADD_TOKEN(TOKEN_COMMA); break; }
            case '.': {
                // Possible dot, double or triple dot.
                if (this->match('.')) {
                    // Possible double or triple dot.
                    if (this->match('.')) {
                        // Triple dot.
                        ADD_TOKEN(TOKEN_TRIPLE_DOT);
                        break;
                    }
                    ADD_TOKEN(TOKEN_DOUBLE_DOT);
                    break;
                }
                ADD_TOKEN(TOKEN_DOT);
                break;
            }
            case ':': { ADD_TOKEN(TOKEN_COLON); break; }
            // case '|': { ADD_TOKEN(TOKEN_STICK); break; }
            case '!': {
                if (this->match('=')) { ADD_TOKEN(TOKEN_BANG_EQUAL); break; }
                ADD_TOKEN(TOKEN_BANG);
                break;
            }
            case '-': {
                if (this->match('>')) { ADD_TOKEN(TOKEN_RIGHT_ARROW); break; }
                ADD_TOKEN(TOKEN_MINUS); break;
            }
            case '+': { ADD_TOKEN(TOKEN_PLUS); break; }
            case '/': {
                if (this->match('/')) { while (PEEK() != '\n' && !IS_AT_END()) { SKIP(); } SKIP(); ++this->line; break; }
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
                ADD_LOG(this->token_error());
                exit(logger->crash());
            }
        }
    }

    tokens->push_back(this->make_token(TOKEN_EOF));
}

Lexer::Lexer(const std::shared_ptr<const std::string> &file)
{
    this->file = file;
}

#undef ADD_TOKEN
#undef TOK_LENGTH
#undef IS_AT_END
#undef NEXT
#undef SKIP
#undef PEEK
#undef PEEK_ON
#undef IS_DIGIT
#undef IS_ALPHA
#undef IS_ALPHANUM
#undef ADD_LOG
