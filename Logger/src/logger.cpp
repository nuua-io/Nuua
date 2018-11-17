#include "../include/logger.hpp"
#include "../include/rang.hpp"

Logger *logger = new Logger;

void Logger::info(const std::string error, int line)
{
    std::cout
        << rang::style::bold
        << rang::fg::cyan
        << "[Info] "
        << rang::fg::yellow
        << error
        << rang::style::reset
        << rang::style::bold;
    if (line >= 0) std::cout << " [Line " << line << "]";
    std::cout << rang::style::reset << std::endl;
}

void Logger::success(const std::string error, int line)
{
    std::cout
        << rang::style::bold
        << rang::fg::green
        << "[Success] "
        << rang::fg::yellow
        << error
        << rang::style::reset
        << rang::style::bold;
    if (line >= 0) std::cout << " [Line " << line << "]";
    std::cout << rang::style::reset << std::endl;
}

void Logger::warning(const std::string error, int line)
{
    std::cout
        << rang::style::bold
        << "[Warning] "
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
        << "[Error] "
        << rang::fg::yellow
        << error
        << rang::style::reset
        << rang::style::bold;
    if (line >= 0) std::cout << " [Line " << line << "]";
    std::cout << rang::style::reset << std::endl;
}
