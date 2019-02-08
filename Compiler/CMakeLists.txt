add_library (Compiler SHARED src/compiler.cpp src/program.cpp src/value.cpp)
target_link_libraries (Compiler Analyzer Logger)
