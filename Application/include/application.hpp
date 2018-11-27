/**
 * |------------------|
 * | Nuua Application |
 * |------------------|
 *
 * Copyright 2019 Erik Campobadal <soc@erik.cat>
 * https://nuua.io
 */
#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include "../../Virtual-Machine/include/virtual_machine.hpp"

// This defines the basic application types
// It includes all nessesary supported types.
typedef enum : uint8_t {
    APPLICATION_PROMPT, APPLICATION_FILE, APPLICATION_STRING,
} ApplicationType;

// The application class handles the application logic
// Given the command line arguments.
class Application
{
    // Defines the application type described above.
    ApplicationType application_type;

    // Stores the vritual machine used by the application.
    VirtualMachine virtual_machine;

    // Using a union reduces the required memory
    union {
        // Stores the file name if the application type requires it.
        std::string *file_name;

        // Stores the input string if the application type requires it.
        std::string *input;
    };

    // Opens the file and returns it's contents.
    // Used when the application type requires it.
    std::string open_file();

    // Run the application in prompt mode.
    void prompt();

    // Run the application based on an input string.
    void string(const std::string string);

    public:
        // The constructor determines the type of the application
        // based on the command line arguments.
        Application(int argc, char *argv[]);

        // Starts (runs) the application.
        void start();
};

#endif
