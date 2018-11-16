/**
 * |--------------|
 * | Nuua Scanner |
 * |--------------|
 *
 * Copyright 2018 Erik Campobadal <soc@erik.cat>
 * https://nuua.io
 */

#ifndef LEXER_HPP
#define LEXER_HPP

#include "tokens.hpp"

typedef struct {
    const char *start;
    const char *current;
    uint64_t line;
} Scanner;

std::vector<Token> *scan(const char *source);

#endif
