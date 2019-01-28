add_library (Analyzer src/analyzer.cpp src/block.cpp src/type.cpp)
set_property(TARGET Analyzer PROPERTY CXX_STANDARD 17)
set_property(TARGET Analyzer PROPERTY CXX_STANDARD_REQUIRED ON)
target_link_libraries (Analyzer Parser Logger)
