/**
 * |-------------------|
 * | Nuua Error Logger |
 * |-------------------|
 *
 * Copyright 2019 Erik Campobadal <soc@erik.cat>
 * https://nuua.io
 */
#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <string>
#include <vector>

// Defines a max line length when printing file lengths.
#define MAX_LINE_LENGTH 500

// Represents a log entity.
class LoggerEntity {
    public:
        // Stores the file where the log comes from.
        const std::string *file;
        // Stores the line of the log.
        const uint32_t line;
        // Stores the message of the log.
        const std::string msg;
        LoggerEntity(const std::string *file, const uint32_t line, const std::string msg)
            : file(file), line(line), msg(msg) {}
};

// Represents the logger used in the whole toolchain.
class Logger
{
    // Stores all the log entities.
    std::vector<LoggerEntity> entities;
    public:
        // Adds a new entity to the entity stack.
        void add_entity(const std::string *file, const uint32_t line, const std::string msg);
        // Pops an entity from the entity stack.
        void pop_entity();
        // Crashes the program by emmiting the whole entity stack as an error.
        int crash();
};

// logger will be a global class instance.
extern Logger *logger;

#endif
