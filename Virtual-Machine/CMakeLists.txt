add_library (Virtual-Machine src/virtual_machine.cpp)
set_property(TARGET Virtual-Machine PROPERTY CXX_STANDARD 17)
set_property(TARGET Virtual-Machine PROPERTY CXX_STANDARD_REQUIRED ON)
target_link_libraries (Virtual-Machine Compiler Logger)
