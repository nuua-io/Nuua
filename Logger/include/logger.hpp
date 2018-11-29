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

// This controlls if we are debugginf the app depending on a preprocessor flag.
#if DEBUG
#define DEBUG_MODE() true
#else
#define DEBUG_MODE() false
#endif

// The logger class.
class Logger
{
    // Set the logger in debug mode or not.
    bool debug = DEBUG_MODE();

    public:
        // Outputs an information message to the screen.
        void info(const std::string error, int line = -1);

        // Outputs a success message to the screen.
        void success(const std::string error, int line = -1);

        // Outputs a warning message to the screen.
        void warning(const std::string error, int line = -1);

        // Outputs an error message to the screen.
        void error(const std::string error, int line = -1);
};

// logger will be a global class instance.
extern Logger *logger;

#undef DEBUG_MODE

#endif
