/**
 * |---------------------------|
 * | Nuua Programming Language |
 * |---------------------------|
 *
 * Copyright (c) 2019 Erik Campobadal <soc@erik.cat>
 * https://nuua.io
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "Application/include/application.hpp"

int main(int argc, char *argv[])
{
    // This creates a new nuua application, given the command line attributes
    auto application = new Application(argc, argv);

    // Run the application. This will automatically determine the application
    // type (For example, if it is a prompt application or a file application)
    application->start();

    // Delete the allocated space of the application
    delete application;

    // Returns a success exit
    return EXIT_SUCCESS;
}
