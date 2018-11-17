CC = g++
CFLAGS = -std=c++17 -Wall -Wextra -flto -Ofast

ifeq ($(OS),Windows_NT)
EXECUTABLE	:= nuua.exe
else
EXECUTABLE	:= nuua
endif

nuua: build/logger.o build/tokens.o build/lexer.o
	$(CC) $(CFLAGS) nuua.cpp -o bin/$(EXECUTABLE) $^
build/logger.o: Logger/src/logger.cpp
	$(CC) $(CFLAGS) -c $^ -o $@
build/lexer.o: Lexer/src/lexer.cpp
	$(CC) $(CFLAGS) -c $^ -o $@
build/tokens.o: Lexer/src/tokens.cpp
	$(CC) $(CFLAGS) -c $^ -o $@

.PHONY: clean
clean:
	rm build/*.o

.PHONY: push
push:
	git push
	git subtree push --prefix=Lexer Lexer master
	git subtree push --prefix=Logger Logger master
