add_library (Virtual-Machine SHARED src/virtual_machine.cpp)
target_link_libraries (Virtual-Machine Compiler Logger)
