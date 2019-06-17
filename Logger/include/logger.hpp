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
#include <memory>
#include <utility>

// Defines a max line length when printing file lengths.
#define MAX_LINE_LENGTH 500
// Defines the max chars to show when printing errors to prevent lines that are too long.
#define MAX_LINE_CHARS 60

typedef uint32_t line_t;
typedef uint16_t column_t;

// Represents a log entity.
class LoggerEntity {
    public:
        // Stores the file where the log comes from.
        const std::shared_ptr<const std::string> file;
        // Stores the line of the log.
        const line_t line;
        // Stores the column of the log.
        const column_t column;
        // Stores the message of the log.
        const std::string msg;
        LoggerEntity(const std::shared_ptr<const std::string> &file, const line_t line, const column_t column, const std::string &msg)
            : file(file), line(line), column(column), msg(msg) {}
};

// Represents the logger used in the whole toolchain.
class Logger
{
    // Stores all the log entities.
    std::vector<LoggerEntity> entities;
    // Displays a specific log entity.
    void display_log(const uint16_t index, const bool red) const;
    public:
        // Stores the executable path.
        std::string executable_path;
        // Stores some command line options.
        bool show_tokens = false;
        bool show_ast = false;
        bool show_opcodes = false;
        bool show_references = false;
        // Adds a new entity to the entity stack.
        void add_entity(const std::shared_ptr<const std::string> &file, const line_t line, const column_t column, const std::string &msg);
        // Pops an entity from the entity stack.
        void pop_entity();
        // Crashes the program by emmiting the whole entity stack as an error.
        int crash() const;
};

// logger will be a global class instance.
extern Logger *logger;

#endif
