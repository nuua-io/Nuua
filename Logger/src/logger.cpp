/**
 * |-------------------|
 * | Nuua Error Logger |
 * |-------------------|
 *
 * Copyright 2019 Erik Campobadal <soc@erik.cat>
 * https://nuua.io
 */
#include "../include/logger.hpp"
#include "../include/rang.hpp"

Logger *logger = new Logger;

void Logger::info(const std::string &msg, int line)
{
    #if DEBUG
        std::cout
            << rang::style::bold
            << rang::fg::cyan
            << " > "
            << rang::fg::yellow
            << msg
            << rang::style::reset
            << rang::style::bold;
        if (line >= 0) std::cout << " [Line " << line << "]";
        std::cout << rang::style::reset << std::endl;
    #endif
}

void Logger::success(const std::string &msg, int line)
{
    #if DEBUG
        std::cout
            << rang::style::bold
            << rang::fg::green
            << " > "
            << rang::fg::yellow
            << msg
            << rang::style::reset
            << rang::style::bold;
        if (line >= 0) std::cout << " [Line " << line << "]";
        std::cout << rang::style::reset << std::endl;
    #endif
}

void Logger::warning(const std::string &msg, int line)
{
    #if DEBUG
        std::cout
            << rang::style::bold
            << " > "
            << rang::fg::yellow
            << msg
            << rang::style::reset
            << rang::style::bold;
        if (line >= 0) std::cout << " [Line " << line << "]";
        std::cout << rang::style::reset << std::endl;
    #endif
}

void Logger::error(const std::string &msg, int line)
{
    std::cerr
        << rang::style::bold
        << rang::fg::red
        << " > "
        << rang::fg::yellow
        << msg
        << rang::style::reset
        << rang::style::bold;
    if (line >= 0) std::cerr << " [Line " << line << "]";
    std::cerr << rang::style::reset << std::endl;
}
