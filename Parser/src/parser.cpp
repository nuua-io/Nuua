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

bool Parser::match_any(std::vector<TokenType> &tokens)
{
    for (TokenType &token : tokens) {
        if (CHECK(token)) {
            if (token != TOKEN_EOF) NEXT();
            return true;
        }
    }

    return false;
}

/* GRAMMAR RULES */

Expression *Parser::primary()
{
    if (this->match(TOKEN_FALSE)) return new Boolean(false);
    if (this->match(TOKEN_TRUE)) return new Boolean(true);
    if (this->match(TOKEN_NONE)) return new None();
    if (this->match(TOKEN_INTEGER)) return new Integer(std::stoi(PREVIOUS().to_string()));
    if (this->match(TOKEN_FLOAT)) return new Float(std::stof(PREVIOUS().to_string()));
    if (this->match(TOKEN_STRING)) return new String(PREVIOUS().to_string());
    if (this->match(TOKEN_IDENTIFIER)) return new Variable(PREVIOUS().to_string());
    if (this->match(TOKEN_LEFT_SQUARE)) {
        std::vector<Expression *> values;
        if (this->match(TOKEN_RIGHT_SQUARE)) return new List(values);
        for (;;) {
            if (IS_AT_END()) {
                logger->error("Unfinished list, Expecting ']' after the last list element.", this->current->line);
                exit(EXIT_FAILURE);
            }
            values.push_back(expression());
            if (this->match(TOKEN_RIGHT_SQUARE)) break;
            this->consume(TOKEN_COMMA, "Expected ',' after a list element");
        }
        return new List(values);
    }
    if (this->match(TOKEN_LEFT_BRACE)) {
        std::unordered_map<std::string, Expression *> values;
        std::vector<std::string> keys;
        if (this->match(TOKEN_RIGHT_BRACE)) return new Dictionary(values, keys);
        for (;;) {
            if (IS_AT_END()) {
                logger->error("Unfinished dictionary, Expecting '}' after the last dictionary element.", this->current->line);
                exit(EXIT_FAILURE);
            }
            Expression *key = this->expression();
            if (key->rule != RULE_VARIABLE) {
                logger->error("Expected an identifier as a key", this->current->line);
                exit(EXIT_FAILURE);
            }
            this->consume(TOKEN_COLON, "Expected ':' after dictionary key");
            std::string name = static_cast<Variable *>(key)->name;
            values[name] = this->expression();
            keys.push_back(name);
            if (this->match(TOKEN_RIGHT_BRACE)) break;
            this->consume(TOKEN_COMMA, "Expected ',' after dictionary element");
        }
        return new Dictionary(values, keys);
    }
    if (this->match(TOKEN_LEFT_PAREN)) {
        Expression *value = this->expression();
        this->consume(TOKEN_RIGHT_PAREN, "Expected ')' after a group expression");
        return new Group(value);
    }
    logger->error("Expected an expression", LINE());
    exit(EXIT_FAILURE);
}

/*
unary_postfix -> primary ("[" primary "]" | "(" arguments? ")")*;
arguments -> expression ("," expression)*;
*/
Expression *Parser::unary_postfix()
{
    Expression *result = this->primary();
    while (this->match_any({{ TOKEN_LEFT_SQUARE, TOKEN_LEFT_PAREN }})) {
        Token op = PREVIOUS();
        switch (op.type) {
            case TOKEN_LEFT_SQUARE: {
                Expression *expr = this->primary();
                this->consume(TOKEN_RIGHT_SQUARE, "Expected ']' after the access index");
                result = new Access(result, expr);
                break;
            }
            case TOKEN_LEFT_PAREN: {
                std::vector<Expression *> arguments = this->arguments();
                this->consume(TOKEN_RIGHT_SQUARE, "Expected ')' after the access index");
                result = new Call(result, arguments);
                break;
            }
            default: { logger->error("Invalid unary postfix operator", LINE()); exit(EXIT_FAILURE); };
        }
    }
    return result;
}

/*
unary_prefix -> ("!" | "-") unary_prefix
    | unary_postfix;
*/
Expression *Parser::unary_prefix()
{
    if (this->match_any({{ TOKEN_BANG, TOKEN_MINUS }})) {
        return new Unary(PREVIOUS(), this->unary_prefix());
    }
    return this->unary_postfix();
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
assignment -> IDENTIFIER "=" assignment
    | logical_or;
*/
Expression *Parser::assignment()
{
    Expression *result = this->logical_or();
    if (this->match(TOKEN_EQUAL)) {
        return new Assign(result, this->assignment());
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
    std::string type = this->type();
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

std::vector<Statement *> Parser::parameters()
{

}

std::vector<Expression *> Parser::arguments()
{

}

std::string Parser::type()
{
    if (this->match(TOKEN_LEFT_SQUARE)) {
        // List type.
        return ("[" + this->type()) + this->consume(TOKEN_RIGHT_SQUARE, "A list type needs to end with ']'")->to_string();
    } else if (this->match(TOKEN_LEFT_BRACE)) {
        // Dict type.
        return ("{" + this->type()) + this->consume(TOKEN_RIGHT_BRACE, "A list type needs to end with '}'")->to_string();
    } else if (this->match(TOKEN_LEFT_PAREN)) {
        // Fun type.
    } else if (CHECK(TOKEN_IDENTIFIER)) {
        // Other types (native + custom).
        return this->consume(TOKEN_IDENTIFIER, "Expected an identifier as a type.")->to_string();
    } else if (CHECK(TOKEN_NEW_LINE) || CHECK(TOKEN_EQUAL)) return "";

    logger->error("Unknown type token expected.", LINE());
    exit(EXIT_FAILURE);
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
