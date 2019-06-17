# Required project information.
cmake_minimum_required (VERSION 3.9)
project (Nuua)

# IPO support
include (CheckIPOSupported)

# Set the output directory
set (OUTPUT_DIR ${CMAKE_BINARY_DIR}/bin)
set (LIBRARY_DIR ${CMAKE_BINARY_DIR}/lib)
set (STANDARD_LIRBARY_DIR ${CMAKE_SOURCE_DIR}/Lib)

# Set the default build mode to release if not set.
if (NOT CMAKE_BUILD_TYPE)
  set (CMAKE_BUILD_TYPE Release)
endif ()

# Set the C++ standard to use.
set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable interprocedural optimizations
check_ipo_supported (RESULT ipo_supported)
if (ipo_supported)
    set (CMAKE_INTERPROCEDURAL_OPTIMIZATION true)
endif ()

# Set the C++ general flags and flags for debug and release.
if (CMAKE_COMPILER_IS_GNUCXX)
    set (CMAKE_CXX_FLAGS "-Wall -Wextra -pipe")
    set (CMAKE_CXX_FLAGS_DEBUG "-g")
    set (CMAKE_CXX_FLAGS_RELEASE "-Ofast -s -flto")
endif (CMAKE_COMPILER_IS_GNUCXX)

# Set the output directories.
set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${LIBRARY_DIR}/$<0:>)
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${LIBRARY_DIR}/$<0:>)
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}/$<0:>)

# Add the subdirectories where to look.
add_subdirectory (Logger)
add_subdirectory (Lexer)
add_subdirectory (Parser)
add_subdirectory (Analyzer)
add_subdirectory (Compiler)
add_subdirectory (Virtual-Machine)
add_subdirectory (Application)

# Setup the final executable.
add_executable (nuua nuua.cpp resources/icon.rc)
target_link_libraries (nuua Application)

# Copy the standard library to the exetuable path.
add_custom_command (TARGET nuua POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${STANDARD_LIRBARY_DIR} ${OUTPUT_DIR}/Lib)
