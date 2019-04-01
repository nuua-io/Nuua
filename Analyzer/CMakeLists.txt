add_library (Analyzer src/analyzer.cpp src/type.cpp)
target_link_libraries (Analyzer Parser Logger)
