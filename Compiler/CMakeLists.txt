add_library (Compiler src/compiler.cpp src/program.cpp src/value.cpp)
set_property(TARGET Compiler PROPERTY CXX_STANDARD 17)
set_property(TARGET Compiler PROPERTY CXX_STANDARD_REQUIRED ON)
target_link_libraries (Compiler Analyzer Logger)
