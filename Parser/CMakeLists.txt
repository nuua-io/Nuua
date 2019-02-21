add_library (Parser SHARED src/parser.cpp src/rules.cpp src/block.cpp)
target_link_libraries (Parser Lexer Logger)
