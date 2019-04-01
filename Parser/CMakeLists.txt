add_library (Parser src/parser.cpp src/rules.cpp src/block.cpp)
target_link_libraries (Parser Lexer Logger -lstdc++fs)
