add_library (Compiler src/compiler.cpp src/program.cpp src/memory.cpp src/value.cpp)
target_link_libraries (Compiler Analyzer Logger)
