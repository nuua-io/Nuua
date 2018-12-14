/**
 * |-------------|
 * | Nuua Parser |
 * |-------------|
 *
 * Copyright 2018 Erik Campobadal <soc@erik.cat>
 * https://nuua.io
 */
#include "../include/parser.hpp"
#include "../../Lexer/include/lexer.hpp"
#include "../../Logger/include/logger.hpp"

#define CURRENT() (*(this->current))
#define PREVIOUS() (*(this->current - 1))
#define CHECK(token) (CURRENT().is(token))
#define NEXT() (*(this->current++))
#define IS_AT_END() (this->current->is(TOKEN_EOF))
#define LOOKAHEAD(n) (*(this->current + n))

Token Parser::consume(TokenType type, const char* message)
{
    if (this->current->is(type)) return NEXT();

    logger->error(std::string(message), this->current->line);
    exit(EXIT_FAILURE);
}

bool Parser::match(TokenType token)
{
    if (CHECK(token)) {
        if (token != TOKEN_EOF) NEXT();
        return true;
    }

    return false;
}

bool Parser::match_any(std::vector<TokenType> tokens)
{
    for (auto token : tokens) {
        if (CHECK(token)) {
            if (token != TOKEN_EOF) NEXT();
            return true;
        }
    }

    return false;
}

std::vector<Statement *> Parser::get_block_body()
{
    std::vector<Statement *> body;
    while (!CHECK(TOKEN_RIGHT_BRACE) && !IS_AT_END()) {
        body.push_back(this->statement());
    }
    return body;
}

bool Parser::is_function()
{
    uint32_t current = 0;
    for (uint32_t skip = 0; skip == 0 && !LOOKAHEAD(current).is(TOKEN_RIGHT_PAREN); current++) {
        if (LOOKAHEAD(current).is(TOKEN_EOF)) {
            logger->error("Unterminated grouping or function. Did you close the parenthesis?", this->current->line);
            exit(EXIT_FAILURE);
        }
        else if (LOOKAHEAD(current).is(TOKEN_LEFT_PAREN)) skip++;
        else if (LOOKAHEAD(current).is(TOKEN_RIGHT_PAREN)) skip--;
    }
    if (LOOKAHEAD(current + 1).is(TOKEN_EOF)) return false;

    return LOOKAHEAD(current + 1).is(TOKEN_LEFT_BRACE)
        || LOOKAHEAD(current + 1).is(TOKEN_RIGHT_ARROW)
        || LOOKAHEAD(current + 1).is(TOKEN_BIG_RIGHT_ARROW);
}

Expression *Parser::function()
{
    std::vector<Expression *> arguments;
    std::vector<Statement *> body;

    // Get the function arguments
    if (!CHECK(TOKEN_RIGHT_PAREN)) {
        do {
            Expression *argument = this->expression();

            // Check if it's a correct argument type
            // Checked already when compiling :)
            /*
            switch (argument->type) {
                case RULE_VARIABLE: { break; }
                default: { logger->error("Invalid argument type when defining the function", this->current->line); exit(EXIT_FAILURE); }
            }
            */

            arguments.push_back(argument);
        } while (this->match(TOKEN_COMMA));
    }

    this->consume(TOKEN_RIGHT_PAREN, "Expected ')' after function arguments");

    // Get the function body
    if (this->match(TOKEN_LEFT_BRACE)) {
        this->consume(TOKEN_NEW_LINE, "Expected a new line after the '{'");
        body = this->get_block_body();
        this->consume(TOKEN_RIGHT_BRACE, "Unterminated block. Expected '}'");
        // This is checked already since it's an expression statement
        // this->consume(TOKEN_NEW_LINE, "Expected a new line after the '}'");
    } else {
        logger->error("Unknown function body found. You need to use '{' or '=>' or '->' after the arguments to define the function's body.", this->current->line);
        exit(EXIT_FAILURE);
    }

    return new Function(arguments, body);
}

Expression *Parser::list()
{
    std::vector<Expression *> values;
    if (this->match(TOKEN_RIGHT_SQUARE)) return new List(values);
    for (;;) {
        if (IS_AT_END()) {
            logger->error("Unfinished list, Expecting ']' after the last list element.", this->current->line);
            exit(EXIT_FAILURE);
        }
        values.push_back(expression());
        if (this->match(TOKEN_RIGHT_SQUARE)) break;
        this->consume(TOKEN_COMMA, "Expected ',' after list element");
    }

    return new List(values);
}

Expression *Parser::dictionary()
{
    std::unordered_map<std::string, Expression *> values;
    std::vector<std::string> keys;
    if (this->match(TOKEN_RIGHT_BRACE)) return new Dictionary(values, keys);
    for (;;) {
        if (IS_AT_END()) {
            logger->error("Unfinished dictionary, Expecting '}' after the last dictionary element.", this->current->line);
            exit(EXIT_FAILURE);
        }
        auto key = this->expression();
        if (key->rule != RULE_VARIABLE) {
            logger->error("Expected an identifier as a key", this->current->line);
            exit(EXIT_FAILURE);
        }
        this->consume(TOKEN_COLON, "Expected ':' after dictionary key");
        auto name = static_cast<Variable *>(key)->name;
        values[name] = this->expression();
        keys.push_back(name);
        if (this->match(TOKEN_RIGHT_BRACE)) break;
        this->consume(TOKEN_COMMA, "Expected ',' after dictionary element");
    }

    /*
    printf("-\n");
    for (auto a : values) printf("%s\n", a.first.c_str());
    printf("-\n");
    */

    return new Dictionary(values, keys);
}

Expression *Parser::primary()
{
    // debug_token(CURRENT());
    if (this->match(TOKEN_FALSE)) return new Boolean(false);
    if (this->match(TOKEN_TRUE)) return new Boolean(true);
    if (this->match(TOKEN_NONE)) return new None();
    if (this->match(TOKEN_INTEGER)) return new Integer(std::stoi(PREVIOUS().to_string()));
    if (this->match(TOKEN_FLOAT)) return new Float(std::stof(PREVIOUS().to_string()));
    if (this->match(TOKEN_STRING)) return new String(PREVIOUS().to_string());
    if (this->match(TOKEN_IDENTIFIER)) return new Variable(PREVIOUS().to_string());
    if (this->match(TOKEN_LEFT_SQUARE)) return this->list();
    if (this->match(TOKEN_LEFT_BRACE)) return this->dictionary();
    if (this->match(TOKEN_LEFT_PAREN)) {
        if (this->is_function()) return this->function();
        auto value = this->expression();
        this->consume(TOKEN_RIGHT_PAREN, "Expected ')' after a group expression");

        return new Group(value);
    }
    // printf("FAILED AT: ");
    // debug_token(CURRENT());
    logger->error("Expected an expression", this->current->line);
    exit(EXIT_FAILURE);
}

Expression *Parser::finish_call(Expression *callee)
{
    if (callee->rule != RULE_VARIABLE) {
        logger->error("Expected an identifier as the function name.", this->current->line);
        exit(EXIT_FAILURE);
    }
    std::vector<Expression *> arguments;
    if (!CHECK(TOKEN_RIGHT_PAREN)) {
        arguments.push_back(this->expression());
        while (this->match(TOKEN_COMMA) && !IS_AT_END()) arguments.push_back(this->expression());
    }

    this->consume(TOKEN_RIGHT_PAREN, "Expected ')' after call arguments");

    return new Call(static_cast<Variable *>(callee)->name, arguments);
}

Expression *Parser::finish_access(Expression *item)
{
    if (item->rule != RULE_VARIABLE) {
        logger->error("Expected an identifier as the access variable.", this->current->line);
        exit(EXIT_FAILURE);
    }
    auto index = this->expression();
    this->consume(TOKEN_RIGHT_SQUARE, "Expected ']' after index access");

    return new Access(static_cast<Variable *>(item)->name, index);
}

Expression *Parser::call()
{
    auto result = this->primary();
    for (;;) {
        if (this->match(TOKEN_LEFT_PAREN)) {
            result = this->finish_call(result);
        } else if (this->match(TOKEN_LEFT_SQUARE)) {
            result = this->finish_access(result);
        } else {
            break;
        }
    }

    return result;
}

Expression *Parser::unary()
{
    if (this->match_any(std::vector<TokenType>({ TOKEN_BANG, TOKEN_MINUS }))) {
        auto op = PREVIOUS();
        return new Unary(op, this->unary());
    }

    return this->call();
}

Expression *Parser::mul_div_mod()
{
    auto result = this->unary();
    while (this->match_any(std::vector<TokenType>({ TOKEN_SLASH, TOKEN_STAR, TOKEN_PERCENT }))) {
        auto op = PREVIOUS();
        result = new Binary(result, op, this->unary());
    }

    return result;
}

Expression *Parser::addition()
{
    auto result = this->mul_div_mod();
    while (this->match_any(std::vector<TokenType>({ TOKEN_MINUS, TOKEN_PLUS }))) {
        auto op = PREVIOUS();
        result = new Binary(result, op, this->mul_div_mod());
    }

    return result;
}

Expression *Parser::comparison()
{
    auto result = this->addition();
    while (this->match_any(std::vector<TokenType>({ TOKEN_HIGHER, TOKEN_HIGHER_EQUAL, TOKEN_LOWER, TOKEN_LOWER_EQUAL }))) {
        auto op = PREVIOUS();
        result = new Binary(result, op, this->addition());
    }

    return result;
}

Expression *Parser::equality()
{
    auto result = this->comparison();
    while (this->match_any(std::vector<TokenType>({ TOKEN_BANG_EQUAL, TOKEN_EQUAL_EQUAL }))) {
        auto op = PREVIOUS();
        result = new Binary(result, op, this->comparison());
    }

    return result;
}

Expression *Parser::and_operator()
{
    auto result = this->equality();
    while (this->match(TOKEN_AND)) {
        auto op = PREVIOUS();
        result = new Logical(result, op, this->equality());
    }

    return result;
}

Expression *Parser::or_operator()
{
    auto result = this->and_operator();
    while (this->match(TOKEN_AND)) {
        auto op = PREVIOUS();
        result = new Logical(result, op, this->and_operator());
    }

    return result;
}

Expression *Parser::assignment()
{
    auto result = this->or_operator();
    if (this->match(TOKEN_EQUAL)) {
        switch (result->rule) {
            case RULE_VARIABLE: { return new Assign(static_cast<Variable *>(result)->name, this->expression()); }
            case RULE_ACCESS: {
                printf("Found assign access.\n");
                auto res = static_cast<Access *>(result);
                return new AssignAccess(res->name, res->index, this->expression());
            }
            default: { logger->error("Invalid assignment target", this->current->line); exit(EXIT_FAILURE); };
        }
    }
    return result;
}

Expression *Parser::expression()
{
    return this->assignment();
}

Statement *Parser::expression_statement()
{
    auto expr = this->expression();
    if (!this->match_any(std::vector<TokenType>({ TOKEN_NEW_LINE, TOKEN_EOF }))) {
        logger->error("Expected a new line or EOF after expression statement", this->current->line);
        exit(EXIT_FAILURE);
    }
    return new ExpressionStatement(expr);
}

Statement *Parser::declaration_statement()
{
    auto variable = this->consume(TOKEN_IDENTIFIER, "Expected an identifier in a declaration statement");
    this->consume(TOKEN_COLON, "Expected ':' after identifier in a declaration statement");
    auto type = this->consume(TOKEN_IDENTIFIER, "Expected a type after the ':' in a declaration statement");
    Expression *initializer = nullptr;

    if (this->match(TOKEN_EQUAL)) initializer = this->expression();

    if (!this->match_any(std::vector<TokenType>({ TOKEN_NEW_LINE, TOKEN_EOF }))) {
        logger->error("Expected a new line after the '}'", this->current->line);
        exit(EXIT_FAILURE);
    }

    return new Declaration(variable.to_string(), type.to_string(), initializer);
}

Statement *Parser::if_statement()
{
    this->consume(TOKEN_LEFT_PAREN, "Expected a '(' after 'if'");
    auto condition = this->expression();
    this->consume(TOKEN_RIGHT_PAREN, "Expected a ')' after 'if' condition");
    this->consume(TOKEN_LEFT_BRACE, "Expected a '{' after the ')'");
    this->consume(TOKEN_NEW_LINE, "Expected a new line after the '{'");
    auto thenBranch = this->get_block_body();
    this->consume(TOKEN_RIGHT_BRACE, "Unterminated block. Expected '}'");

    if (!this->match_any(std::vector<TokenType>({ TOKEN_NEW_LINE, TOKEN_EOF }))) {
        logger->error("Expected a new line after the '}'", this->current->line);
        exit(EXIT_FAILURE);
    }

    return new If(condition, thenBranch, std::vector<Statement *>({}));
}

Statement *Parser::while_statement()
{
    this->consume(TOKEN_LEFT_PAREN, "Expected '(' after 'while'");
    auto condition = this->expression();
    this->consume(TOKEN_RIGHT_PAREN, "Expected ')' after 'while' condition");
    this->consume(TOKEN_LEFT_BRACE, "Expected a '{' after the ')'");
    this->consume(TOKEN_NEW_LINE, "Expected a new line after the '{'");
    auto body = this->get_block_body();
    this->consume(TOKEN_RIGHT_BRACE, "Unterminated block. Expected '}'");

    if (!this->match_any(std::vector<TokenType>({ TOKEN_NEW_LINE, TOKEN_EOF }))) {
        logger->error("Expected a new line after the '}'", this->current->line);
        exit(EXIT_FAILURE);
    }

    return new While(condition, body);
}

Statement *Parser::statement()
{
    if (CHECK(TOKEN_IDENTIFIER) && LOOKAHEAD(1).is(TOKEN_COLON)) return this->declaration_statement();
    else if (this->match(TOKEN_IF)) return this->if_statement();
    else if (this->match(TOKEN_WHILE)) return this->while_statement();

    return this->expression_statement();
}

std::vector<Statement *> Parser::parse(const char *source)
{
    auto lexer = new Lexer;

    std::vector<Token> tokens = lexer->scan(source);

    delete lexer;

    logger->info("Started parsing...");

    this->current = &tokens.front();

    std::vector<Statement *> code;

    while (!IS_AT_END()) {
        // printf("Looking for statement... Starting at ");
        // CURRENT().debug_token();
        code.push_back(this->statement());
    }

    #if DEBUG
        Parser::debug_rules(code);
    #endif

    logger->success("Parsing completed");

    return code;
}

#undef CURRENT
#undef PREVIOUS
#undef CHECK
#undef NEXT
#undef IS_AT_END
#undef LOOKAHEAD
