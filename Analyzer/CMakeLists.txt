add_library (Analyzer SHARED src/analyzer.cpp src/module.cpp)
target_link_libraries (Analyzer Parser Logger)
