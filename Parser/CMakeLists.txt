add_library (Parser src/parser.cpp src/rules.cpp)
set_property(TARGET Parser PROPERTY CXX_STANDARD 17)
set_property(TARGET Parser PROPERTY CXX_STANDARD_REQUIRED ON)
target_link_libraries (Parser Lexer Logger)
