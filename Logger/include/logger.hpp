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

class Logger
{
    public:
        void info(const std::string error, int line = -1);
        void success(const std::string error, int line = -1);
        void warning(const std::string error, int line = -1);
        void error(const std::string error, int line = -1);
};

extern Logger *logger;

#endif
