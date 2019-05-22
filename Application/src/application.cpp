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
    printf("----> Application\n");
    this->virtual_machine.interpret(string.c_str());
    printf("----> !Application\n");
}

Application::Application(int argc, char *argv[])
{
    if (argc < 1) {
        exit(logger->crash());
    }
    // Set the executable_path.
    logger->executable_path = argv[0];
    // Fire the application
    switch (argc) {
        case 2: { this->application_type = APPLICATION_FILE; this->file_name = std::string(argv[1]); break; }
        default: {
            this->application_type = APPLICATION_FILE; this->file_name = std::string("C:/Nuua/simple_app/test.nu"); break;
            // logger->add_entity(this->file, LINE(), "Invalid usage. Try: nuua <path_to_file>\n");
            logger->crash();
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
