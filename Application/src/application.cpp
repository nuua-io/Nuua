/**
 * |------------------|
 * | Nuua Application |
 * |------------------|
 *
 * Copyright 2019 Erik Campobadal <soc@erik.cat>
 * https://nuua.io
 */
#include "../include/application.hpp"
#include "../../Logger/include/logger.hpp"
#include <iostream>
#include <fstream>

void Application::string(const std::string &string)
{
    this->virtual_machine.interpret(string.c_str());
}

Application::Application(int argc, char *argv[])
{
    switch (argc) {
        case 2: { this->application_type = APPLICATION_FILE; this->file_name = std::string(argv[1]); break; }
        default: {
            logger->error("Invalid usage. Try: nuua <path_to_file>\n");
            exit(64); // Exit status for incorrect command usage.
        }
    }
}

int Application::start()
{
    switch (this->application_type) {
        case APPLICATION_FILE: { this->string(this->file_name); break; }
        case APPLICATION_STRING: { this->string(""); break; }
    }

    return EXIT_SUCCESS;
}
