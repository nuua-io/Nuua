cmake_minimum_required (VERSION 2.8)
project (Nuua)

if (NOT CMAKE_BUILD_TYPE)
  set (CMAKE_BUILD_TYPE Release)
endif ()

if (CMAKE_COMPILER_IS_GNUCXX)
    set (CMAKE_CXX_FLAGS "-Wall -Wextra")
    set (CMAKE_CXX_FLAGS_DEBUG "-g")
    set (CMAKE_CXX_FLAGS_RELEASE "-Ofast")
endif (CMAKE_COMPILER_IS_GNUCXX)

set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory (Logger)
add_subdirectory (Lexer)
add_subdirectory (Parser)
add_subdirectory (Analyzer)
add_subdirectory (Compiler)
add_subdirectory (Virtual-Machine)
add_subdirectory (Application)

add_executable (nuua nuua.cpp)
set_property (TARGET nuua PROPERTY CXX_STANDARD 17)
set_property (TARGET nuua PROPERTY CXX_STANDARD_REQUIRED ON)
target_link_libraries (nuua Application)
