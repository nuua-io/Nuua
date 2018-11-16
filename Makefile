.PHONY: push
push:
	git push
	git subtree push --prefix=Lexer Lexer master
	git subtree push --prefix=Logger Logger master
