# Configuration
CXX = g++
# CXXFLAGS = -m64 -std=c++17 -flto -Ofast -D DEBUG=false -fno-signed-zeros -fno-trapping-math -funroll-loops -fopenmp -D_GLIBCXX_PARALLEL
CXXFLAGS = -m64 -std=c++17 -Wall -Wextra -D DEBUG=true
BIN = bin
BUILD = build

# Dependency list for each layered tier
MODULES = Logger Lexer Parser Analyzer Compiler Virtual-Machine Application

# Objects required to build nuua
OBJS = $(BUILD)/nuua.o $(foreach module,$(MODULES),$(patsubst $(module)/src/%.cpp,$(BUILD)/$(module)/src/%.o,$(wildcard $(module)/src/*.cpp)))
DEPS = $(patsubst %.o,%.d,$(OBJS))

# Helpers
define \n


endef
GENERATE_DEPENDENCY = @printf " -> Generating dependencies: %s\n" $1${\n}@$(CXX) $(CXXFLAGS) -MM $1 -MT $(BUILD)/$(patsubst %.cpp,%.o,$1) -MF $(BUILD)/$(patsubst %.cpp,%.d,$1)

ifeq ($(OS),Windows_NT)
EXECUTABLE	:= nuua.exe
else
EXECUTABLE	:= nuua
endif

ifeq ($(CXX),clang++)
CXXFLAGS += -Xclang -flto-visibility-public-std
endif

# Main entry point
all:
	@printf "\n"
	@printf "   _   _ _   _ _   _   _     \n"
	@printf "  | \ | | | | | | | | / \    \n"
	@printf "  |  \| | | | | | | |/ _ \   \n"
	@printf "  | |\  | |_| | |_| / ___ \  \n"
	@printf "  |_| \_|\___/ \___/_/   \_\ \n"
	@printf "                             \n"
	@printf "\n"
	@printf " -------------------------------------------\n"
	@printf " |                                         |\n"
	@printf " |        NUUA PROGRAMMING LANGUAGE        |\n"
	@printf " |          Erik Campobadal Fores          |\n"
	@printf " |             https://nuua.io             |\n"
	@printf " |                                         |\n"
	@printf " -------------------------------------------\n"
	@printf "\n"
	@printf "\n"
	@printf " -------------------------------------------\n"
	@printf " |> Building Nuua                           \n"
	@printf " -------------------------------------------\n"
	@printf "\n"
	@$(MAKE) --no-print-directory nuua
	@printf "\n"
	@printf " -------------------------------------------\n"
	@printf " |> Cleaning:                               \n"
	@printf " -------------------------------------------\n"
	@printf "\n"
	@$(MAKE) --no-print-directory clean_deps
	@printf "\n"
	@printf " -------------------------------------------\n"
	@printf " |> Complete: $(BIN)/$(EXECUTABLE)\n"
	@printf " -------------------------------------------\n"
	@printf "\n"

.PHONY: nuua
nuua: $(BIN)/$(EXECUTABLE)

# Build the nuua programming language
$(BIN)/$(EXECUTABLE): $(OBJS)
	@printf "\n"
	@printf " -------------------------------------------\n"
	@printf " |> Linking Nuua                            \n"
	@printf " -------------------------------------------\n"
	@printf "\n"
	@printf " -> Linking Nuua:\n"
	@printf "    { %s }\n" $^
	@$(CXX) $(CXXFLAGS) -o $@ $^

$(DEPS):
	$(call GENERATE_DEPENDENCY, $(subst build/,,$(patsubst %.d,%.cpp,$@)))
$(OBJS):
	@printf " -> Compiling %s\n" $<
	@$(CXX) $(CXXFLAGS) -c $< -o $@

-include $(DEPS)

.PHONY: clean
clean:
	@printf " -> Cleaning Nuua\n"
	@rm -f build/*.o
	$(foreach module,$(MODULES),@printf " -> Cleaning %s\n" $(module)${\n}@rm -f build/$(module)/src/*.o build/$(module)/src/*.d${\n})

.PHONY: clean_deps
clean_deps:
	$(foreach module,$(MODULES),@printf " -> Cleaning dependencies: %s\n" "build/$(module)/src/*.d"${\n}@rm -f build/$(module)/src/*.d${\n})

.PHONY: push
push:
	@printf " -> Pushing Nuua\n"
	@git push
	$(foreach module,$(MODULES),@printf " -> Pushing %s\n" $(module)${\n}@git subtree push --prefix=$(module) $(module) master${\n})
