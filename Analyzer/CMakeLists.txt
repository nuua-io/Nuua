add_library (Analyzer SHARED src/analyzer.cpp src/block.cpp src/type.cpp)
target_link_libraries (Analyzer Parser Logger)
