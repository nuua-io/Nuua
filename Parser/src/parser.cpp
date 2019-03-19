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
#define EXPECT_NEW_LINE() if (!this->match_any({{TOKEN_NEW_LINE, TOKEN_EOF}})) { \
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

bool Parser::match_any(std::vector<TokenType>  tokens)
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

/*
primary -> "false"
    | "true"
    | INTEGER
    | FLOAT
    | STRING
    | IDENTIFIER
    | LIST
    | DICTIONARY
    | "(" expression ")"
*/
Expression *Parser::primary()
{
    if (this->match(TOKEN_FALSE)) return new Boolean(false);
    if (this->match(TOKEN_TRUE)) return new Boolean(true);
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
    if (this->match(TOKEN_STICK)) {
        std::vector<Statement *> parameters = this->parameters();
        if (!this->match(TOKEN_STICK)) {
            parameters = this->parameters();
            this->consume(TOKEN_STICK, "Expected '|' after the closure parameters");
        }
        std::string return_type;
        if (this->match(TOKEN_COLON)) return_type = this->type(false);
        std::vector<Statement *> body;
        if (this->match(TOKEN_RIGHT_ARROW)) {
            body.push_back(new Return(this->expression()));
        } else if (this->match(TOKEN_BIG_RIGHT_ARROW)) {
            body.push_back(this->statement(false));
        } else if (this->match(TOKEN_LEFT_BRACE)) {
            EXPECT_NEW_LINE();
            body = this->body();
            this->consume(TOKEN_RIGHT_BRACE, "Expected '}' after closure body.");
        } else {
            logger->error("Unknown token found after closure. Expected '->', '=>' or '{'.", LINE());
            exit(EXIT_FAILURE);
        }
        return new Closure(parameters, return_type, body);
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
                this->consume(TOKEN_RIGHT_PAREN, "Expected ')' after function arguments");
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
variable_declaration -> IDENTIFIER ":" ("=" expression)?;
*/
Statement *Parser::variable_declaration()
{
    std::string variable = this->consume(TOKEN_IDENTIFIER, "Expected an identifier in a declaration statement")->to_string();
    this->consume(TOKEN_COLON, "Expected ':' after identifier in a declaration statement");
    std::string type = this->type();
    Expression *initializer = nullptr;
    if (this->match(TOKEN_EQUAL)) initializer = this->expression();
    return new Declaration(variable, type, initializer);
}

/*
expression_statement -> expression;
*/
Statement *Parser::expression_statement()
{
    return new ExpressionStatement(this->expression());
}

/*
import_declaration -> "import" IDENTIFIER "from" IDENTIFIER;
*/
Statement *Parser::import_declaration()
{
    std::string target = this->consume(TOKEN_IDENTIFIER, "Expected an identifier after 'import'")->to_string();
    this->consume(TOKEN_FROM, "Expected 'from' after the import target");
    std::string module = this->consume(TOKEN_IDENTIFIER, "Expected an identifier after 'from'")->to_string();
    return new Import(target, module);
}

/*
fun_declaration -> "fun" IDENTIFIER "(" parameters? ")" (":" IDENTIFIER)? ("->" expression "\n" | "=>" statement | "{" "\n" statement* "}" "\n");
parameters -> variable_declaration ("," variable_declaration)*;
*/
Statement *Parser::fun_declaration()
{
    std::string name = this->consume(TOKEN_IDENTIFIER, "Expected an identifier (function name) after 'fun'.")->to_string();
    this->consume(TOKEN_LEFT_PAREN, "Expected '(' after the function name.");
    std::vector<Statement *> parameters;
    if (!this->match(TOKEN_RIGHT_PAREN)) {
        parameters = this->parameters();
        this->consume(TOKEN_RIGHT_PAREN, "Expected ')' after the function parameters");
    }
    std::string return_type;
    if (this->match(TOKEN_COLON)) return_type = this->type(false);
    std::vector<Statement *> body;
    if (this->match(TOKEN_RIGHT_ARROW)) {
        body.push_back(new Return(this->expression()));
    } else if (this->match(TOKEN_BIG_RIGHT_ARROW)) {
        body.push_back(this->statement(false));
    } else if (this->match(TOKEN_LEFT_BRACE)) {
        EXPECT_NEW_LINE();
        body = this->body();
        this->consume(TOKEN_RIGHT_BRACE, "Expected '}' after function body.");
    } else {
        logger->error("Unknown token found after function. Expected '->', '=>' or '{'.", LINE());
        exit(EXIT_FAILURE);
    }
    return new Function(name, parameters, return_type, body);
}

/*
print_statement -> "print" expression;
*/
Statement *Parser::print_statement()
{
    return new Print(this->expression());
}

Statement *Parser::if_statement()
{
    Expression *condition = this->expression();
    std::vector<Statement *> then_branch, else_branch;

    // Then branch
    if (this->match(TOKEN_LEFT_BRACE)) {
        EXPECT_NEW_LINE();
        then_branch = this->body();
        this->consume(TOKEN_RIGHT_BRACE, "Expected '}' after 'if' body.");
    } else if (this->match(TOKEN_BIG_RIGHT_ARROW)) {
        then_branch.push_back(this->statement(false));
    } else {
        logger->error("Expected '{' or '=>' after 'if' condition.");
        exit(EXIT_FAILURE);
    }

    // Else branch
    if (this->match(TOKEN_ELIF)) {
        else_branch.push_back(this->if_statement());
    } else if (this->match(TOKEN_ELSE)) {
        if (this->match(TOKEN_LEFT_BRACE)) {
            EXPECT_NEW_LINE();
            else_branch = this->body();
            this->consume(TOKEN_RIGHT_BRACE, "Expected '}' after 'else' body.");
        } else if (this->match(TOKEN_BIG_RIGHT_ARROW)) {
            else_branch.push_back(this->statement(false));
        } else {
            logger->error("Expected '{' or '=>' after 'else'.");
            exit(EXIT_FAILURE);
        }
    }

    return new If(condition, then_branch, else_branch);;
}

Statement *Parser::while_statement()
{
    Expression *condition = this->expression();
    std::vector<Statement *> body;
    if (this->match(TOKEN_LEFT_BRACE)) {
        EXPECT_NEW_LINE();
        body = this->body();
        this->consume(TOKEN_RIGHT_BRACE, "Expected '}' after 'while' body.");
    } else if (this->match(TOKEN_BIG_RIGHT_ARROW)) {
        body.push_back(this->statement(false));
    } else {
        logger->error("Expected '{' or '=>' after 'while' condition.");
        exit(EXIT_FAILURE);
    }
    return new While(condition, body);
}

/*
return_statement -> "return" expression?;
*/
Statement *Parser::return_statement()
{
    if (this->match_any({{ TOKEN_NEW_LINE, TOKEN_EOF }})) {
        return new Return(nullptr);
    }

    return new Return(this->expression());
}

/*
statement -> import_declaration "\n"
    | class_declaration "\n"
    | fun_declaration "\n"
    | variable_declaration "\n"?
    | if_statement "\n"
    | while_statement "\n"
    | for_statement "\n"
    | return_statement "\n"
    | print_statement "\n"
    | expression_statement "\n"?;
*/
Statement *Parser::statement(bool new_line)
{
    Statement *result;
    // Remove blank lines
    while (this->match(TOKEN_NEW_LINE));

    // Check what type of statement we're parsing.
    if (this->match(TOKEN_IMPORT)) result = this->import_declaration();
    else if (this->match(TOKEN_CLASS));
    else if (this->match(TOKEN_FUN)) result = this->fun_declaration();
    else if (CHECK(TOKEN_IDENTIFIER) && LOOKAHEAD(1).type == TOKEN_COLON) result = this->variable_declaration();
    else if (this->match(TOKEN_IF)) result = this->if_statement();
    else if (this->match(TOKEN_WHILE)) result = this->while_statement();
    else if (this->match(TOKEN_FOR));
    else if (this->match(TOKEN_RETURN)) result = this->return_statement();
    else if (this->match(TOKEN_PRINT)) result = this->print_statement();
    else result = this->expression_statement();

    if (new_line) EXPECT_NEW_LINE();

    return result;
}

std::vector<Statement *> Parser::parameters()
{
    std::vector<Statement *> parameters;
    if (!CHECK(TOKEN_RIGHT_PAREN)) {
        do {
            Statement *parameter = this->statement(false);
            if (parameter->rule != RULE_DECLARATION) {
                logger->error("Invalid argument when defining the function. Expected a declaration.", parameter->line);
                exit(EXIT_FAILURE);
            }
            parameters.push_back(parameter);
        } while (this->match(TOKEN_COMMA));
    }
    return parameters;
}

std::vector<Expression *> Parser::arguments()
{
    std::vector<Expression *> arguments;
    if (!CHECK(TOKEN_RIGHT_PAREN)) {
        do arguments.push_back(this->expression());
        while (this->match(TOKEN_COMMA));
    }
    return arguments;
}

std::vector<Statement *> Parser::body()
{
    std::vector<Statement *> body;
    while (!IS_AT_END() && !CHECK(TOKEN_RIGHT_BRACE)) {
        body.push_back(this->statement());
    }
    return body;
}

std::string Parser::type(bool optional)
{
    if (this->match(TOKEN_LEFT_SQUARE)) {
        // List type.
        return ("[" + this->type()) + this->consume(TOKEN_RIGHT_SQUARE, "A list type needs to end with ']'")->to_string();
    } else if (this->match(TOKEN_LEFT_BRACE)) {
        // Dict type.
        return ("{" + this->type()) + this->consume(TOKEN_RIGHT_BRACE, "A list type needs to end with '}'")->to_string();
    } else if (this->match(TOKEN_STICK)) {
        // Closure type.
        std::string type = "|";
        type += this->type();
        while (this->match(TOKEN_COMMA)) type += ", " + this->type();
        if (this->match(TOKEN_COLON)) type += ": " + this->type();
        return type + this->consume(TOKEN_STICK, "A closure type needs to end with '|'")->to_string();
    } else if (CHECK(TOKEN_IDENTIFIER)) {
        // Other types (native + custom).
        return this->consume(TOKEN_IDENTIFIER, "Expected an identifier as a type.")->to_string();
    } else if (optional && (CHECK(TOKEN_NEW_LINE) || CHECK(TOKEN_EQUAL))) return "";

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
        Parser::debug_ast(code);
        printf("\n");
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
