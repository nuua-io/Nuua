# Configuration
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -flto -Ofast
BIN = bin
BUILD = build

# Dependency list for each layered tier
MODULES = Logger Lexer Parser

# Objects required to build nuua
OBJS = $(BUILD)/nuua.o $(foreach module,$(MODULES),$(patsubst $(module)/src/%.cpp,$(BUILD)/$(module)/%.o,$(wildcard $(module)/src/*.cpp)))

ifeq ($(OS),Windows_NT)
EXECUTABLE	:= nuua.exe
else
EXECUTABLE	:= nuua
endif

# Main entry point
all: $(BIN)/$(EXECUTABLE)

# Build the nuua programming language
$(BIN)/$(EXECUTABLE): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^
build/nuua.o: nuua.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
build/Logger/%.o: Logger/src/%.cpp Logger/include/%.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
build/Lexer/%.o: Lexer/src/%.cpp Lexer/include/%.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
build/Parser/%.o: Parser/src/%.cpp Parser/include/%.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
build/Compiler/%.o: Compiler/src/%.cpp Compiler/include/%.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
build/Virtual-Machine/%.o: Virtual-Machine/src/%.cpp Virtual-Machine/include/%.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm build/*.o
	rm build/Lexer/*.o
	rm build/Logger/*.o
	rm build/Parser/*.o

.PHONY: push
push:
	git push
	git subtree push --prefix=Lexer Lexer master
	git subtree push --prefix=Logger Logger master
	git subtree push --prefix=Parser Parser master
