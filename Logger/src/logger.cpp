/**
 * |-------------------|
 * | Nuua Error Logger |
 * |-------------------|
 *
 * Copyright 2018 Erik Campobadal <soc@erik.cat>
 * https://nuua.io
 */
#include "../include/logger.hpp"
#include "../include/rang.hpp"

Logger *logger = new Logger;

#define DEBUG_CHECK() if (!this->debug) return

void Logger::info(const std::string error, int line)
{
    DEBUG_CHECK();
    std::cout
        << rang::style::bold
        << rang::fg::cyan
        << "-> "
        << rang::fg::yellow
        << error
        << rang::style::reset
        << rang::style::bold;
    if (line >= 0) std::cout << " [Line " << line << "]";
    std::cout << rang::style::reset << std::endl;
}

void Logger::success(const std::string error, int line)
{
    DEBUG_CHECK();
    std::cout
        << rang::style::bold
        << rang::fg::green
        << ">> "
        << rang::fg::yellow
        << error
        << rang::style::reset
        << rang::style::bold;
    if (line >= 0) std::cout << " [Line " << line << "]";
    std::cout << rang::style::reset << std::endl;
}

void Logger::warning(const std::string error, int line)
{
    DEBUG_CHECK();
    std::cout
        << rang::style::bold
        << "!> "
        << rang::fg::yellow
        << error
        << rang::style::reset
        << rang::style::bold;
    if (line >= 0) std::cout << " [Line " << line << "]";
    std::cout << rang::style::reset << std::endl;
}

void Logger::error(const std::string error, int line)
{
    std::cout
        << rang::style::bold
        << rang::fg::red
        << ">< "
        << rang::fg::yellow
        << error
        << rang::style::reset
        << rang::style::bold;
    if (line >= 0) std::cout << " [Line " << line << "]";
    std::cout << rang::style::reset << std::endl;
}

#undef DEBUG_CHECK
