# Configuration
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -flto -Ofast
BIN = bin

# Dependency list for each layered tier
DEP_LOGGER = Logger/src/logger.o
DEP_LEXER = Lexer/src/tokens.o Lexer/src/lexer.o

# Nuua dependencies
DEPENDENCIES = nuua.o $(DEP_LOGGER) $(DEP_LEXER)

ifeq ($(OS),Windows_NT)
EXECUTABLE	:= nuua.exe
else
EXECUTABLE	:= nuua
endif

# Main entry point
all: $(BIN)/$(EXECUTABLE)

# Build the nuua programming language
$(BIN)/$(EXECUTABLE): $(DEPENDENCIES)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm build/*.o

.PHONY: push
push:
	git push
	git subtree push --prefix=Lexer Lexer master
	git subtree push --prefix=Logger Logger master
