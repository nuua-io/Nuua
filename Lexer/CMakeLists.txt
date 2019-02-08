add_library (Lexer SHARED src/lexer.cpp src/tokens.cpp)
target_link_libraries (Lexer Logger)
