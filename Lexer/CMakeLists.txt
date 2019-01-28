add_library (Lexer src/lexer.cpp src/tokens.cpp)
set_property(TARGET Lexer PROPERTY CXX_STANDARD 17)
set_property(TARGET Lexer PROPERTY CXX_STANDARD_REQUIRED ON)
target_link_libraries(Lexer Logger)
