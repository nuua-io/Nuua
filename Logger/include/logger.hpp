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

// The logger class.
class Logger
{
    public:
        // Outputs an information message to the screen.
        void info(const std::string &msg, int line = -1);

        // Outputs a success message to the screen.
        void success(const std::string &msg, int line = -1);

        // Outputs a warning message to the screen.
        void warning(const std::string &msg, int line = -1);

        // Outputs an error message to the screen.
        void error(const std::string &msg, int line = -1);
};

// logger will be a global class instance.
extern Logger *logger;

#undef DEBUG_MODE

#endif
