/**
 * |-------------|
 * | Nuua Parser |
 * |-------------|
 *
 * Copyright 2019 Erik Campobadal <soc@erik.cat>
 * https://nuua.io
 */
#include "../include/parser.hpp"
#include "../../Lexer/include/lexer.hpp"
#include "../../Logger/include/logger.hpp"

#define CURRENT() (*(this->current))
#define PREVIOUS() (*(this->current - 1))
#define CHECK(token) (this->current->type == token)
#define NEXT() (this->current++)
#define IS_AT_END() (CHECK(TOKEN_EOF))
#define LOOKAHEAD(n) (*(this->current + n))
#define LINE() (this->current->line)
#define EXPECT_NEW_LINE() if (this->match_any({{TOKEN_NEW_LINE, TOKEN_EOF}})) { \
    logger->error("Expected a new line or EOF.", LINE()); exit(EXIT_FAILURE); }

Token *Parser::consume(TokenType type, const char *message)
{
    if (this->current->type == type) return NEXT();

    logger->error(std::string(message), LINE());
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
    for (TokenType &token : tokens) {
        if (CHECK(token)) {
            if (token != TOKEN_EOF) NEXT();
            return true;
        }
    }

    return false;
}

std::string Parser::get_type()
{
    if (this->match(TOKEN_LEFT_SQUARE)) {
        // List type.
        return ("[" + this->get_type()) + this->consume(TOKEN_RIGHT_SQUARE, "A list type needs to end with ']'")->to_string();
    } else if (this->match(TOKEN_LEFT_BRACE)) {
        // Dict type.
        return ("{" + this->get_type()) + this->consume(TOKEN_RIGHT_BRACE, "A list type needs to end with '}'")->to_string();
    } else if (this->match(TOKEN_LEFT_PAREN)) {
        // Fun type.
    } else if (CHECK(TOKEN_IDENTIFIER)) {
        // Other types (native + custom).
        return this->consume(TOKEN_IDENTIFIER, "Expected an identifier as a type.")->to_string();
    } else if (CHECK(TOKEN_NEW_LINE) || CHECK(TOKEN_EQUAL)) return "";

    logger->error("Unknown type token expected.", LINE());
    exit(EXIT_FAILURE);
}

/* GRAMMAR RULES */

/*
unary_prefix -> ("!" | "-") unary_prefix
    | call;
*/
Expression *Parser::unary_prefix()
{
    if (this->match_any({{ TOKEN_BANG, TOKEN_MINUS }})) {
        return new Unary(PREVIOUS(), this->unary_prefix());
    }
    return this->call();
}

/*
multiplication -> unary_prefix (("/" | "*") unary_prefix)*;
*/
Expression *Parser::multiplication()
{
    Expression *result = this->unary_prefix();
    while (this->match_any({{ TOKEN_SLASH, TOKEN_STAR, TOKEN_PERCENT }})) {
        result = new Binary(result, PREVIOUS(), this->unary_prefix());
    }
    return result;
}

/*
addition -> multiplication (("-" | "+") multiplication)*;
*/
Expression *Parser::addition()
{
    Expression *result = this->multiplication();
    while (this->match_any({{ TOKEN_MINUS, TOKEN_PLUS }})) {
        result = new Binary(result, PREVIOUS(), this->multiplication());
    }
    return result;
}

/*
comparison -> addition ((">" | ">=" | "<" | "<=") addition)*;
*/
Expression *Parser::comparison()
{
    Expression *result = this->addition();
    while (this->match_any({{ TOKEN_HIGHER, TOKEN_HIGHER_EQUAL, TOKEN_LOWER, TOKEN_LOWER_EQUAL }})) {
        result = new Binary(result, PREVIOUS(), this->addition());
    }
    return result;
}

/*
equality -> comparison (("!=" | "==") comparison)*;
*/
Expression *Parser::equality()
{
    Expression *result = this->comparison();
    while (this->match_any({{ TOKEN_BANG_EQUAL, TOKEN_EQUAL_EQUAL }})) {
        result = new Binary(result, PREVIOUS(), this->comparison());
    }
    return result;
}

/*
logical_and -> equality ("and" equality)*;
*/
Expression *Parser::logical_and()
{
    Expression *result = this->equality();
    while (this->match(TOKEN_AND)) {
        result = new Logical(result, PREVIOUS(), this->equality());
    }
    return result;
}

/*
logical_or -> logical_and ("or" logical_and)*;
*/
Expression *Parser::logical_or()
{
    Expression *result = this->logical_and();
    while (this->match(TOKEN_AND)) {
        result = new Logical(result, PREVIOUS(), this->logical_and());
    }
    return result;
}

/*
assignment -> IDENTIFIER "=" expression
    | logical_or;
*/
Expression *Parser::assignment()
{
    Expression *result = this->logical_or();
    if (this->match(TOKEN_EQUAL)) {
        switch (result->rule) {
            case RULE_VARIABLE: { return new Assign(static_cast<Variable *>(result)->name, this->expression()); }
            case RULE_ACCESS: {
                Access *res = static_cast<Access *>(result);
                return new AssignAccess(res->name, res->index, this->expression());
            }
            default: { logger->error("Invalid assignment target", this->current->line); exit(EXIT_FAILURE); };
        }
    }
    return result;
}

/*
expression -> assignment;
*/
Expression *Parser::expression()
{
    return this->assignment();
}

/*
variable_declaration -> IDENTIFIER ":" ("=" expression)? '\n';
*/
Statement *Parser::variable_declaration()
{
    std::string variable = this->consume(TOKEN_IDENTIFIER, "Expected an identifier in a declaration statement")->to_string();
    this->consume(TOKEN_COLON, "Expected ':' after identifier in a declaration statement");
    // We need to get the variable type. It may be empty if's an inferred type.
    std::string type = this->get_type();
    // Function initializer
    Expression *initializer = nullptr;
    if (this->match(TOKEN_EQUAL)) {
        // Initializer detected.
        initializer = this->expression();
    }
    EXPECT_NEW_LINE();
    return new Declaration(variable, type, initializer);
}

/*
expression_statement -> expression '\n';
*/
Statement *Parser::expression_statement()
{
    return new ExpressionStatement(this->expression());
}

/*
statement -> class_declaration
    | fun_declaration
    | variable_declaration
    | if_statement
    | while_statement
    | for_statement
    | return_statement
    | print_statement
    | expression_statement;
*/
Statement *Parser::statement()
{
    // Remove blank lines
    while (this->match(TOKEN_NEW_LINE));

    // Check what type of statement we're parsing.
    if (this->match(TOKEN_CLASS));
    else if (this->match(TOKEN_FUN));
    else if (CHECK(TOKEN_IDENTIFIER) && LOOKAHEAD(1).type == TOKEN_COLON) return this->variable_declaration();
    else if (this->match(TOKEN_IF));
    else if (this->match(TOKEN_WHILE));
    else if (this->match(TOKEN_FOR));
    else if (this->match(TOKEN_RETURN));
    else if (this->match(TOKEN_PRINT));

    return this->expression_statement();
}

std::vector<Statement *> Parser::parse(const char *source)
{
    std::vector<Token> tokens = Lexer().scan(source);

    logger->info("Started parsing...");
    this->current = &tokens[0];
    std::vector<Statement *> code;

    while (!IS_AT_END()) code.push_back(this->statement());

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
#undef LINE
