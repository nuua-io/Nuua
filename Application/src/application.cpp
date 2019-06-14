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
    this->virtual_machine.interpret(string.c_str(), this->argv);
}

Application::Application(int argc, char *argv[])
{
    // Generic error in case the argc is lower than 2.
    if (argc < 2) {
        logger->add_entity(std::shared_ptr<const std::string>(), 0, 0, "Invalid usage. Try: nuua <path_to_file> [option1, option2, ...]\n");
        logger->crash();
        exit(64); // Exit status for incorrect command usage.
    }
    // Set the executable_path.
    logger->executable_path = { argv[0] };
    // Create the argv: [string] vector
    // And parse command line arguments.
    for (int i = 0; i < argc; i++) {
        this->argv.push_back({ argv[i] });
        // Check if it's the application file.
        if (this->argv.back() == "--ast") {
            logger->show_ast = true;
        } else if (this->argv.back() == "--tokens") {
            logger->show_tokens = true;
        } else if (this->argv.back() == "--opcodes") {
            logger->show_opcodes = true;
        } else if (this->file_name == "") {
            this->file_name = std::string(argv[1]);
        }
    }
    // Check if the path is set.
    if (this->file_name == "") {
        logger->add_entity(std::shared_ptr<const std::string>(), 0, 0, "Invalid usage. Try: nuua <path_to_file> [option1, option2, ...]\n");
        logger->crash();
        exit(64); // Exit status for incorrect command usage.
    }
    // Setup the application
    this->application_type = APPLICATION_FILE;
}

int Application::start()
{
    switch (this->application_type) {
        case APPLICATION_FILE: { this->string(this->file_name); break; }
    }

    return EXIT_SUCCESS;
}
