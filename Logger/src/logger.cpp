/**
 * |-------------------|
 * | Nuua Error Logger |
 * |-------------------|
 *
 * Copyright 2019 Erik Campobadal <soc@erik.cat>
 * https://nuua.io
 */
#include "../include/logger.hpp"
#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <cstdlib>

Logger *logger = new Logger;

static int red_printf(const char *format, ...)
{
    va_list arg;
    int done;
    va_start(arg, format);

    #if defined(_WIN32) || defined(_WIN64)
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
        WORD saved_attributes;
        /* Save current attributes */
        GetConsoleScreenBufferInfo(hConsole, &consoleInfo);
        saved_attributes = consoleInfo.wAttributes;
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
        done = vfprintf(stderr, format, arg);
        /* Restore original attributes */
        SetConsoleTextAttribute(hConsole, saved_attributes);
    #else
        char *fmt = malloc(sizeof(char) * (strlen(format) + 10)); // \x1b[31m\x1b[0m = (9 + '\0')
        strcpy(fmt, "\x1b[31m");
        strcat(fmt, format);
        strcat(fmt, "\x1b[0m");
        done = vfprintf(stderr, fmt, arg);
    #endif

    va_end(arg);

    return done;
}

static void print_file_line(const char *file, const uint32_t line, const uint16_t column)
{
    FILE *source_file = fopen(file, "r");
    if (source_file == NULL) {
        printf("   \n   <unknown>\n");
        exit(EXIT_FAILURE);
    }
    char buffer[MAX_LINE_LENGTH];
    for (uint32_t current_line = 1; current_line <= line; current_line++) {
        error_read_again:
        fgets(buffer, sizeof(buffer), source_file);
        if (buffer == NULL || *buffer == EOF) {
            printf("   \n   <unknown>\n");
            exit(EXIT_FAILURE);
        }
        if (strlen(buffer) == MAX_LINE_LENGTH && buffer[strlen(buffer) - 1] != '\n') {
            goto error_read_again;
        }
    }
    fclose(source_file);
    // Trim the initial spaces / tabs.
    uint16_t offset = 0;
    while (buffer[offset] == '\t' || buffer[offset] == ' ') offset++;
    printf("   \n   %s   ", buffer + offset);
    for (uint16_t i = 1; i < column - offset; i++) printf(" ");
    printf("^\n");
}

void Logger::add_entity(const std::string *file, const uint32_t line, const uint16_t column, const std::string msg)
{
    this->entities.push_back({ file, line, column, msg });
}

void Logger::pop_entity()
{
    this->entities.pop_back();
}

int Logger::crash()
{
    if (this->entities.size() == 0) {
        // Show a generic error since no entities exists.
        red_printf("There was an error\n");
        return EXIT_FAILURE;
    }
    // Display the error stack using the entities.
    printf(" ----------------------\n");
    red_printf(" > THERE WAS AN ERROR <\n");
    printf(" ----------------------\n");
    for (size_t i = 0; i < this->entities.size() - 1; i++) {
        printf(
            " > %s - %s, line: %u, column: %u\n",
            this->entities[i].msg.c_str(),
            this->entities[i].file->c_str(),
            this->entities[i].line,
            this->entities[i].column
        );
        print_file_line(this->entities[i].file->c_str(), this->entities[i].line, this->entities[i].column);
    }
    printf(" > ");
    red_printf("%s", this->entities[this->entities.size() - 1].msg.c_str());
    printf(
        " - %s, line: %u, column: %u\n",
        this->entities[this->entities.size() - 1].file->c_str(),
        this->entities[this->entities.size() - 1].line,
        this->entities[this->entities.size() - 1].column
    );
    print_file_line(
        this->entities[this->entities.size() - 1].file->c_str(),
        this->entities[this->entities.size() - 1].line,
        this->entities[this->entities.size() - 1].column
    );

    return EXIT_FAILURE;
}
