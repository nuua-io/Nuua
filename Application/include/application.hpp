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
    APPLICATION_FILE,
} ApplicationType;

// The application class handles the application logic
// Given the command line arguments.
class Application
{
    // Defines the application type described above.
    ApplicationType application_type;
    // Stores the vritual machine used by the application.
    VirtualMachine virtual_machine;
    // Stores the file name if the application type requires it.
    std::string file_name;
    // Stores the command line arguments.
    std::vector<std::string> argv;
    // Run the application based on an input string.
    void string(const std::string &string);
    public:
        // The constructor determines the type of the application
        // based on the command line arguments.
        Application(int argc, char *argv[]);
        // Starts (runs) the application.
        int start();
};

#endif
