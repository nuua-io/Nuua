# Required project information.
cmake_minimum_required (VERSION 2.8)
project (Nuua)

# Set the default build mode to release if not set.
if (NOT CMAKE_BUILD_TYPE)
  set (CMAKE_BUILD_TYPE Release)
endif ()

# Set the C++ general flags and flags for debug and release.
if (CMAKE_COMPILER_IS_GNUCXX)
    set (CMAKE_CXX_FLAGS "-Wall -Wextra")
    set (CMAKE_CXX_FLAGS_DEBUG "-g -D DEBUG=true")
    set (CMAKE_CXX_FLAGS_RELEASE "-Ofast -D DEBUG=false")
endif (CMAKE_COMPILER_IS_GNUCXX)

# Set the C++ standard to use.
set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

# Set the output directories.
set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Add the subdirectories where to look.
add_subdirectory (Logger)
add_subdirectory (Lexer)
add_subdirectory (Parser)
add_subdirectory (Analyzer)
add_subdirectory (Compiler)
add_subdirectory (Virtual-Machine)
add_subdirectory (Application)

# Setup the final executable.
add_executable (nuua nuua.cpp)
target_link_libraries (nuua Application)
