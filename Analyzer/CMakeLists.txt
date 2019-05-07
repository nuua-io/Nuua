add_library (Analyzer src/analyzer.cpp src/module.cpp)
target_link_libraries (Analyzer Parser Logger)
