#include "../include/application.hpp"
#include <iostream>
#include <fstream>

void Application::prompt()
{
    std::string input;
    for (;;) {
        printf(">>> ");
        std::getline(std::cin, input);

        if (input == ".exit" || std::cin.fail() || std::cin.eof()) {
            std::cin.clear();
            exit(EXIT_SUCCESS);
        }

        input += '\n';
        this->virtual_machine.interpret(input.c_str());
        this->virtual_machine.reset();
    }
}

void Application::string(const std::string string)
{
    this->virtual_machine.interpret(string.c_str());
}

std::string Application::open_file()
{
    auto file_stream = std::ifstream(this->file_name->c_str());
    if (!file_stream.is_open()) {
        fprintf(stderr, "Unable to open file '%s'\n", this->file_name->c_str());
        exit(EXIT_FAILURE);
    }

    return std::string((std::istreambuf_iterator<char>(file_stream)), (std::istreambuf_iterator<char>()));
}

Application::Application(int argc, char *argv[])
{
    switch (argc) {
        case 1: { this->application_type = APPLICATION_PROMPT; break; }
        case 2: { this->application_type = APPLICATION_FILE; this->file_name = new std::string(argv[1]); break; }
        default: { fprintf(stderr, "Invalid usage. Try: nuua <path_to_file>\n"); exit(64); } // Exit status for incorrect command usage.
    }
}

void Application::start()
{
    switch (this->application_type) {
        case APPLICATION_PROMPT: { this->prompt(); break; }
        case APPLICATION_FILE: { this->string(this->open_file()); break; }
        case APPLICATION_STRING: { this->string(""); break; }
    }
}
