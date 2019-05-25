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
#include <filesystem>

#define CURRENT() (*(this->current))
#define PREVIOUS() (*(this->current - 1))
#define CHECK(token) (this->current->type == token)
#define NEXT() (this->current++)
#define IS_AT_END() (CHECK(TOKEN_EOF))
#define LOOKAHEAD(n) (*(this->current + n))
#define LINE() (this->current->line)
#define COL() (this->current->column)
#define PLINE() ((this->current - 1)->line)
#define PCOL() ((this->current - 1)->column)
#define ADD_LOG(msg) logger->add_entity(this->file, LINE(), COL(), msg)
#define ADD_PREV_LOG(msg) logger->add_entity(this->file, PLINE(), PCOL(), msg)
#define ADD_LOG_PAR(line, col, msg) logger->add_entity(this->file, line, col, msg)
#define EXPECT_NEW_LINE() if (!this->match_any({{ TOKEN_NEW_LINE, TOKEN_EOF }})) { \
    ADD_LOG("Expected a new line or EOF but got '" + CURRENT().to_string() + "'."); exit(logger->crash()); }
#define NEW_NODE(type, ...) (std::make_shared<type>(this->file, PLINE(), PCOL(), __VA_ARGS__))

// Stores the parsing file stack, to avoid
// cyclic imports.
static std::vector<std::shared_ptr<const std::string>> file_stack;

#define PREVENT_CYCLIC(file_ptr) \
{ \
    if (std::find(file_stack.begin(), file_stack.end(), file_ptr) != file_stack.end()) { \
        ADD_LOG("Cyclic import detected. Can't use '" + *file_ptr + "'. Cyclic imports are not available in nuua."); \
        exit(logger->crash()); \
    } \
}

// Stores the relation between a file_name and the
// parsed Abstract Syntax Tree and the pointer to the
// original long lived file name string.
// Acts as a temporal cache to avoid re-parsing a file.
static std::unordered_map<
    std::string,
    std::pair<
        std::shared_ptr<std::vector<std::shared_ptr<Statement>>>,
        std::shared_ptr<const std::string>
    >
> parsed_files;

Token *Parser::consume(const TokenType type, const std::string &message)
{
    if (this->current->type == type) return NEXT();
    ADD_LOG(message);
    exit(logger->crash());
}

bool Parser::match(const TokenType token)
{
    if (CHECK(token)) {
        if (token != TOKEN_EOF) NEXT();
        return true;
    }
    return false;
}

bool Parser::match_any(const std::vector<TokenType> &tokens)
{
    for (const TokenType &token : tokens) {
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
std::shared_ptr<Expression> Parser::primary()
{
    if (this->match(TOKEN_FALSE)) return NEW_NODE(Boolean, false);
    if (this->match(TOKEN_TRUE)) return NEW_NODE(Boolean, true);
    if (this->match(TOKEN_INTEGER)) return NEW_NODE(Integer, std::stoll(PREVIOUS().to_string()));
    if (this->match(TOKEN_FLOAT)) return NEW_NODE(Float, std::stof(PREVIOUS().to_string()));
    if (this->match(TOKEN_STRING)) return NEW_NODE(String, PREVIOUS().to_string());
    if (this->match(TOKEN_IDENTIFIER)) return NEW_NODE(Variable, PREVIOUS().to_string());
    if (this->match(TOKEN_LEFT_SQUARE)) {
        std::vector<std::shared_ptr<Expression> > values;
        if (this->match(TOKEN_RIGHT_SQUARE)) return NEW_NODE(List, values);
        for (;;) {
            if (IS_AT_END()) {
                ADD_LOG("Unfinished list, Expecting ']' after the last list element.");
                exit(logger->crash());
            }
            values.push_back(std::move(expression()));
            if (this->match(TOKEN_RIGHT_SQUARE)) break;
            this->consume(TOKEN_COMMA, "Expected ',' after a list element");
        }
        return NEW_NODE(List, values);
    }
    if (this->match(TOKEN_LEFT_BRACE)) {
        std::unordered_map<std::string, std::shared_ptr<Expression>> values;
        std::vector<std::string> keys;
        if (this->match(TOKEN_RIGHT_BRACE)) return NEW_NODE(Dictionary, values, keys);
        for (;;) {
            if (IS_AT_END()) {
                ADD_LOG("Unfinished dictionary, Expecting '}' after the last dictionary element.");
                exit(logger->crash());
            }
            std::shared_ptr<Expression> key = this->expression();
            if (key->rule != RULE_VARIABLE) {
                ADD_LOG("Expected an identifier as a key");
                exit(logger->crash());
            }
            this->consume(TOKEN_COLON, "Expected ':' after dictionary key");
            std::string name = std::static_pointer_cast<Variable>(key)->name;
            values[name] = this->expression();
            keys.push_back(name);
            if (this->match(TOKEN_RIGHT_BRACE)) break;
            this->consume(TOKEN_COMMA, "Expected ',' after dictionary element");
        }
        return NEW_NODE(Dictionary, values, keys);
    }
    if (this->match(TOKEN_LEFT_PAREN)) {
        std::shared_ptr<Expression> value = this->expression();
        this->consume(TOKEN_RIGHT_PAREN, "Expected ')' after a group expression");
        return NEW_NODE(Group, value);
    }
    /*
    if (this->match(TOKEN_STICK)) {
        std::vector<std::shared_ptr<Statement> > parameters = this->parameters();
        if (!this->match(TOKEN_STICK)) {
            parameters = this->parameters();
            this->consume(TOKEN_STICK, "Expected '|' after the closure parameters");
        }
        std::string return_type;
        if (this->match(TOKEN_COLON)) return_type = this->type(false);
        std::vector<std::shared_ptr<Statement> > body;
        if (this->match(TOKEN_RIGHT_ARROW)) {
            body.push_back(NEW_NODE(Return, this->expression()));
        } else if (this->match(TOKEN_BIG_RIGHT_ARROW)) {
            body.push_back(this->statement(false));
        } else if (this->match(TOKEN_LEFT_BRACE)) {
            EXPECT_NEW_LINE();
            body = this->body();
            this->consume(TOKEN_RIGHT_BRACE, "Expected '}' after closure body.");
        } else {
            ADD_LOG("Unknown token found after closure. Expected '->', '=>' or '{'.");
            exit(logger->crash());
        }
        return NEW_NODE(Closure, parameters, return_type, body);
    }
    */
    ADD_LOG("Expected an expression but got '" + CURRENT().to_string() + "'");
    exit(logger->crash());
}

/*
unary_postfix -> primary ("[" expression "]" | slice | "(" arguments? ")")*;
slice -> "[" expression? ":" expression? (":" expression?)? "]"
arguments -> expression ("," expression)*;
*/
std::shared_ptr<Expression> Parser::unary_postfix()
{
    std::shared_ptr<Expression> result = this->primary();
    while (this->match_any({{ TOKEN_LEFT_SQUARE, TOKEN_LEFT_PAREN }})) {
        Token op = PREVIOUS();
        switch (op.type) {
            case TOKEN_LEFT_SQUARE: {
                std::shared_ptr<Expression> start = std::shared_ptr<Expression>();
                std::shared_ptr<Expression> end = std::shared_ptr<Expression>();
                std::shared_ptr<Expression> step = std::shared_ptr<Expression>();
                if (this->match(TOKEN_COLON)) goto parser_is_slice1;
                start = this->expression();
                if (this->match(TOKEN_COLON)) {
                    // It's a Slice, not an access and the start index is already calculated.
                    parser_is_slice1:
                    if (this->match(TOKEN_RIGHT_SQUARE)) goto parser_finish_slice;
                    if (this->match(TOKEN_COLON)) goto parser_get_slice_step;
                    end = this->expression();
                    if (this->match(TOKEN_RIGHT_SQUARE)) goto parser_finish_slice;
                    this->consume(TOKEN_COLON, "Expected ':' or ']' after slice end index");
                    parser_get_slice_step:
                    if (this->match(TOKEN_RIGHT_SQUARE)) goto parser_finish_slice;
                    step = this->expression();
                    this->consume(TOKEN_RIGHT_SQUARE, "Expected ']' after a slice step");
                    parser_finish_slice:
                    result = NEW_NODE(Slice, result, start, end, step);
                    break;
                }
                this->consume(TOKEN_RIGHT_SQUARE, "Expected ']' after the access index");
                result = NEW_NODE(Access, result, start);
                break;
            }
            case TOKEN_LEFT_PAREN: {
                std::vector<std::shared_ptr<Expression> > arguments = this->arguments();
                this->consume(TOKEN_RIGHT_PAREN, "Expected ')' after function arguments");
                result = NEW_NODE(Call, result, arguments);
                break;
            }
            default: {
                ADD_LOG("Invalid unary postfix operator");
                exit(logger->crash());
            };
        }
    }
    return result;
}

/*
unary_prefix -> ("!" | "+" | "-") unary_prefix
    | unary_postfix;
*/
std::shared_ptr<Expression> Parser::unary_prefix()
{
    if (this->match_any({{ TOKEN_BANG, TOKEN_PLUS, TOKEN_MINUS }})) {
        Token op = PREVIOUS();
        std::shared_ptr<Expression> expr = this->unary_prefix();
        return NEW_NODE(Unary, op, expr);
    }
    return this->unary_postfix();
}

/*
cast -> unary_prefix ("as" type)*;
*/
std::shared_ptr<Expression> Parser::cast()
{
    std::shared_ptr<Expression> result = this->unary_prefix();
    while (this->match(TOKEN_AS)) {
        std::shared_ptr<Type> type = this->type();
        result = NEW_NODE(Cast, result, type);
    }
    return result;
}

/*
multiplication -> cast (("/" | "*") cast)*;
*/
std::shared_ptr<Expression> Parser::multiplication()
{
    std::shared_ptr<Expression> result = this->cast();
    while (this->match_any({{ TOKEN_SLASH, TOKEN_STAR, TOKEN_PERCENT }})) {
        Token op = PREVIOUS();
        std::shared_ptr<Expression> expr = this->cast();
        result = NEW_NODE(Binary, result, op, expr);
    }
    return result;
}

/*
addition -> multiplication (("-" | "+") multiplication)*;
*/
std::shared_ptr<Expression> Parser::addition()
{
    std::shared_ptr<Expression> result = this->multiplication();
    while (this->match_any({{ TOKEN_MINUS, TOKEN_PLUS }})) {
        Token op = PREVIOUS();
        std::shared_ptr<Expression> expr = this->multiplication();
        result = NEW_NODE(Binary, result, op, expr);
    }
    return result;
}

/*
comparison -> addition ((">" | ">=" | "<" | "<=") addition)*;
*/
std::shared_ptr<Expression> Parser::comparison()
{
    std::shared_ptr<Expression> result = this->addition();
    while (this->match_any({{ TOKEN_HIGHER, TOKEN_HIGHER_EQUAL, TOKEN_LOWER, TOKEN_LOWER_EQUAL }})) {
        Token op = PREVIOUS();
        std::shared_ptr<Expression> expr = this->addition();
        result = NEW_NODE(Binary, result, op, expr);
    }
    return result;
}

/*
equality -> comparison (("!=" | "==") comparison)*;
*/
std::shared_ptr<Expression> Parser::equality()
{
    std::shared_ptr<Expression> result = this->comparison();
    while (this->match_any({{ TOKEN_BANG_EQUAL, TOKEN_EQUAL_EQUAL }})) {
        Token op = PREVIOUS();
        std::shared_ptr<Expression> expr = this->comparison();
        result = NEW_NODE(Binary, result, op, expr);
    }
    return result;
}

/*
logical_and -> equality ("and" equality)*;
*/
std::shared_ptr<Expression> Parser::logical_and()
{
    std::shared_ptr<Expression> result = this->equality();
    while (this->match(TOKEN_AND)) {
        Token op = PREVIOUS();
        std::shared_ptr<Expression> expr = this->equality();
        result = NEW_NODE(Logical, result, op, expr);
    }
    return result;
}

/*
logical_or -> logical_and ("or" logical_and)*;
*/
std::shared_ptr<Expression> Parser::logical_or()
{
    std::shared_ptr<Expression> result = this->logical_and();
    while (this->match(TOKEN_AND)) {
        Token op = PREVIOUS();
        std::shared_ptr<Expression> expr = this->logical_and();
        result = NEW_NODE(Logical, result, op, expr);
    }
    return result;
}

/*
range -> logical_or ((".." | "...") logical_or)*;
*/
std::shared_ptr<Expression> Parser::range()
{
    std::shared_ptr<Expression> result = this->logical_or();
    while (this->match_any({{ TOKEN_DOUBLE_DOT, TOKEN_TRIPLE_DOT }})) {
        std::shared_ptr<Expression> right = this->expression();
        result = NEW_NODE(Range, result, right, PREVIOUS().type == TOKEN_DOUBLE_DOT ? false : true);
    }
    return result;
}

/*
assignment -> range ("=" range)*;
*/
std::shared_ptr<Expression> Parser::assignment()
{
    std::shared_ptr<Expression> result = this->range();
    while (this->match(TOKEN_EQUAL)) {
        std::shared_ptr<Expression> expr = this->range();
        result = NEW_NODE(Assign, result, expr);
    }
    return result;
}

/*
expression -> assignment;
*/
std::shared_ptr<Expression> Parser::expression()
{
    return this->assignment();
}

/*
variable_declaration -> IDENTIFIER ":" ("=" expression)?;
*/
std::shared_ptr<Statement> Parser::variable_declaration()
{
    std::string variable = this->consume(TOKEN_IDENTIFIER, "Expected an identifier in a declaration statement")->to_string();
    this->consume(TOKEN_COLON, "Expected ':' after identifier in a declaration statement");
    std::shared_ptr<Type> type = this->type();
    std::shared_ptr<Expression> initializer;
    if (this->match(TOKEN_EQUAL)) initializer = this->expression();
    return NEW_NODE(Declaration, variable, type, initializer);
}

/*
expression_statement -> expression;
*/
std::shared_ptr<Statement> Parser::expression_statement()
{
    std::shared_ptr<Expression> expr = this->expression();
    return NEW_NODE(ExpressionStatement, expr);
}

/*
use_declaration -> "use" STRING
    | "use" IDENTIFIER ("," IDENTIFIER)* "from" STRING;
*/
std::shared_ptr<Statement> Parser::use_declaration()
{
    std::vector<std::string> targets;
    std::string module;
    if (CHECK(TOKEN_IDENTIFIER)) {
        targets.push_back(this->consume(TOKEN_IDENTIFIER, "Expected an identifier after 'use'")->to_string()); // The message is redundant.
        while (this->match(TOKEN_COMMA)) targets.push_back(this->consume(TOKEN_IDENTIFIER, "Expected an identifier after ','")->to_string());
        this->consume(TOKEN_FROM, "Expected 'from' after the import target");
        module = this->consume(TOKEN_STRING, "Expected an identifier after 'from'")->to_string();
    } else {
        module = this->consume(TOKEN_STRING, "Expected an identifier or 'string' after 'use'")->to_string();
    }
    printf("Hey, the module: %s\n", module.c_str());
    Parser::format_path(module, this->file);
    std::shared_ptr<Use> use;
    // Parse the contents of the target.
    if (parsed_files.find(module) == parsed_files.end()) {
        use = NEW_NODE(Use, targets, std::make_shared<std::string>(module));
        PREVENT_CYCLIC(use->module);
        use->code = std::make_shared<std::vector<std::shared_ptr<Statement>>>();
        Parser(use->module).parse(use->code);
    } else {
        use = NEW_NODE(Use, targets, parsed_files[module].second);
        PREVENT_CYCLIC(use->module);
        use->code = parsed_files[module].first;
    }
    return use;
}

/*
export_declaration -> "export" top_level_declaration
*/
std::shared_ptr<Statement> Parser::export_declaration()
{
    std::shared_ptr<Statement> stmt = this->top_level_declaration();
    if (stmt->rule == RULE_EXPORT) {
        ADD_LOG("Can't export an export. Does that even make sense?.");
        exit(logger->crash());
    }
    return NEW_NODE(Export, stmt);
}

/*
fun_declaration -> "fun" IDENTIFIER "(" parameters? ")" (":" type)? ("->" expression "\n" | "=>" statement | "{" "\n" statement* "}" "\n");
parameters -> variable_declaration ("," variable_declaration)*;
*/
std::shared_ptr<Statement> Parser::fun_declaration()
{
    std::string name = this->consume(TOKEN_IDENTIFIER, "Expected an identifier (function name) after 'fun'.")->to_string();
    this->consume(TOKEN_LEFT_PAREN, "Expected '(' after the function name.");
    std::vector<std::shared_ptr<Declaration>> parameters;
    if (!this->match(TOKEN_RIGHT_PAREN)) {
        this->parameters(&parameters);
        this->consume(TOKEN_RIGHT_PAREN, "Expected ')' after the function parameters");
    }
    std::shared_ptr<Type> return_type;
    if (this->match(TOKEN_COLON)) return_type = this->type(false);
    std::vector<std::shared_ptr<Statement> > body;
    if (this->match(TOKEN_RIGHT_ARROW)) {
        body.push_back(std::move(NEW_NODE(Return, this->expression())));
    } else if (this->match(TOKEN_BIG_RIGHT_ARROW)) {
        body.push_back(std::move(this->statement(false)));
    } else if (this->match(TOKEN_LEFT_BRACE)) {
        EXPECT_NEW_LINE();
        body = this->body();
        this->consume(TOKEN_RIGHT_BRACE, "Expected '}' after function body.");
    } else {
        ADD_LOG("Unknown token found after function. Expected '->', '=>' or '{'.");
        exit(logger->crash());
    }
    return NEW_NODE(Function, name, parameters, return_type, body);
}

/*
print_statement -> "print" expression;
*/
std::shared_ptr<Statement> Parser::print_statement()
{
    std::shared_ptr<Expression> expr = this->expression();
    return NEW_NODE(Print, expr);
}

/*
if_statement -> "if" expression ("=>" statement | "{" "\n" statement* "}");
*/
std::shared_ptr<Statement> Parser::if_statement()
{
    std::shared_ptr<Expression> condition = this->expression();
    std::vector<std::shared_ptr<Statement> > then_branch, else_branch;
    // Then branch
    if (this->match(TOKEN_LEFT_BRACE)) {
        EXPECT_NEW_LINE();
        then_branch = this->body();
        this->consume(TOKEN_RIGHT_BRACE, "Expected '}' after 'if' body.");
    } else if (this->match(TOKEN_BIG_RIGHT_ARROW)) {
        then_branch.push_back(std::move(this->statement(false)));
    } else {
        ADD_LOG("Expected '{' or '=>' after 'if' condition.");
        exit(logger->crash());
    }
    // Else branch
    if (this->match(TOKEN_ELIF)) {
        else_branch.push_back(std::move(this->if_statement()));
    } else if (this->match(TOKEN_ELSE)) {
        if (this->match(TOKEN_LEFT_BRACE)) {
            EXPECT_NEW_LINE();
            else_branch = this->body();
            this->consume(TOKEN_RIGHT_BRACE, "Expected '}' after 'else' body.");
        } else if (this->match(TOKEN_BIG_RIGHT_ARROW)) {
            else_branch.push_back(std::move(this->statement(false)));
        } else {
            ADD_LOG("Expected '{' or '=>' after 'else'.");
            exit(logger->crash());
        }
    }
    return NEW_NODE(If, condition, then_branch, else_branch);
}

std::shared_ptr<Statement> Parser::while_statement()
{
    std::shared_ptr<Expression> condition = this->expression();
    std::vector<std::shared_ptr<Statement> > body;
    if (this->match(TOKEN_LEFT_BRACE)) {
        EXPECT_NEW_LINE();
        body = this->body();
        this->consume(TOKEN_RIGHT_BRACE, "Expected '}' after 'while' body.");
    } else if (this->match(TOKEN_BIG_RIGHT_ARROW)) {
        body.push_back(std::move(this->statement(false)));
    } else {
        ADD_LOG("Expected '{' or '=>' after 'while' condition.");
        exit(logger->crash());
    }
    return NEW_NODE(While, condition, body);
}

/*
for_statement -> "for" IDENTIFIER ("," IDENTIFIER)? "in" expression ("=>" statement | "{" "\n" statement* "}" "\n");
*/
std::shared_ptr<Statement> Parser::for_statement()
{
    std::string index, variable = this->consume(TOKEN_IDENTIFIER, "Expected identifier after 'for'")->to_string();
    if (this->match(TOKEN_COMMA)) {
        index = this->consume(TOKEN_IDENTIFIER, "Expected identifier as the optional second identifier in 'for'")->to_string();
    }
    this->consume(TOKEN_IN, "Expected 'in' after 'for' identifier/s.");
    std::shared_ptr<Expression> iterator = this->expression();
    std::vector<std::shared_ptr<Statement> > body;
    if (this->match(TOKEN_LEFT_BRACE)) {
        EXPECT_NEW_LINE();
        body = this->body();
        this->consume(TOKEN_RIGHT_BRACE, "Expected '}' after 'for' body.");
    } else if (this->match(TOKEN_BIG_RIGHT_ARROW)) {
        body.push_back(std::move(this->statement(false)));
    } else {
        ADD_LOG("Expected '{' or '=>' after 'for' iterator.");
        exit(logger->crash());
    }
    return NEW_NODE(For, variable, index, iterator, body);
}

/*
return_statement -> "return" expression?;
*/
std::shared_ptr<Statement> Parser::return_statement()
{
    if (CHECK(TOKEN_NEW_LINE) || CHECK(TOKEN_EOF)) {
        return NEW_NODE(Return, nullptr);
    }
    std::shared_ptr<Expression> expr = this->expression();
    return NEW_NODE(Return, expr);
}

std::shared_ptr<Statement> Parser::class_statement()
{
    std::string name = this->consume(TOKEN_IDENTIFIER, "Expected identifier after 'class'.")->to_string();
    this->consume(TOKEN_LEFT_BRACE, "Expected '{' after 'class' name.");
    std::vector<std::shared_ptr<Statement> > body = this->class_body();
    this->consume(TOKEN_RIGHT_BRACE, "Expected '}' after 'class' body.");
    return NEW_NODE(Class, name, body);
}

/*
statement -> variable_declaration "\n"?
    | if_statement "\n"
    | while_statement "\n"
    | for_statement "\n"
    | return_statement "\n"
    | print_statement "\n"
    | expression_statement "\n"?;
*/
std::shared_ptr<Statement> Parser::statement(bool new_line)
{
    std::shared_ptr<Statement> result;
    // Remove blank lines
    while (this->match(TOKEN_NEW_LINE));

    // Save the current line / column.
    line_t line = LINE();
    column_t column = COL();

    // Check what type of statement we're parsing.
    if (CHECK(TOKEN_IDENTIFIER) && LOOKAHEAD(1).type == TOKEN_COLON) {
        ADD_LOG("Parsing variable declaration");
        result = this->variable_declaration();
    } else if (this->match(TOKEN_IF)) {
        ADD_PREV_LOG("Parsing if declaration");
        result = this->if_statement();
    } else if (this->match(TOKEN_WHILE)) {
        ADD_PREV_LOG("Parsing while statement");
        result = this->while_statement();
    } else if (this->match(TOKEN_FOR)) {
        ADD_PREV_LOG("Parsing for statement");
        result = this->for_statement();
    } else if (this->match(TOKEN_RETURN)) {
        ADD_PREV_LOG("Parsing return statement");
        result = this->return_statement();
    } else if (this->match(TOKEN_PRINT)) {
        ADD_PREV_LOG("Parsing print statement");
        result = this->print_statement();
    } else {
        ADD_LOG("Parsing expression");
        result = this->expression_statement();
    }
    if (new_line) EXPECT_NEW_LINE();
    logger->pop_entity();

    // Set the line / column to overwrite the ones in the node.
    result->line = line;
    result->column = column;

    return result;
}

/*
top_level_declaration -> use_declaration "\n"
    | export_declaration "\n"
    | class_declaration "\n"
    | fun_declaration "\n";
*/
std::shared_ptr<Statement> Parser::top_level_declaration()
{
    std::shared_ptr<Statement> result;
    // Remove blank lines
    while (this->match(TOKEN_NEW_LINE));

    // Save the current line / column.
    line_t line = LINE();
    column_t column = COL();

    if (this->match(TOKEN_USE)) {
        ADD_PREV_LOG("Parsing 'use' declaration");
        result = this->use_declaration();
    } else if (this->match(TOKEN_EXPORT)) {
        ADD_PREV_LOG("Parsing 'export' declaration");
        result = this->export_declaration();
    } else if (this->match(TOKEN_CLASS)) {
        ADD_PREV_LOG("Parsing 'class' declaration");
        result = this->class_statement();
    } else if (this->match(TOKEN_FUN)) {
        ADD_PREV_LOG("Parsing 'fun' declaration");
        result = this->fun_declaration();
    } else {
        ADD_LOG("Unknown top level declaration. Expected 'use', 'export', 'class' or 'fun'. But got '" + CURRENT().to_string() + "'");
        exit(logger->crash());
    }
    EXPECT_NEW_LINE();
    logger->pop_entity();

    // Set the line / column to overwrite the ones in the node.
    result->line = line;
    result->column = column;

    return result;
}

std::shared_ptr<Statement> Parser::class_body_declaration()
{
    std::shared_ptr<Statement> result = nullptr;
    // Remove blank lines
    while (this->match(TOKEN_NEW_LINE));

    // Save the current line / column.
    line_t line = LINE();
    column_t column = COL();

    if (CHECK(TOKEN_IDENTIFIER) && LOOKAHEAD(1).type == TOKEN_COLON) {
        ADD_LOG("Parsing variable declaration");
    } else if (this->match(TOKEN_FUN)) {
        ADD_PREV_LOG("Parsing 'fun' declaration");
        result = this->fun_declaration();
    } else {
        ADD_LOG("Invalid class block declaration. Expected 'fun' or variable declaration. But got '" + CURRENT().to_string() + "'");
        exit(logger->crash());
    }
    EXPECT_NEW_LINE();
    logger->pop_entity();

    // Set the line / column to overwrite the ones in the node.
    result->line = line;
    result->column = column;

    return result;
}

void Parser::parameters(std::vector<std::shared_ptr<Declaration>> *dest)
{
    if (!CHECK(TOKEN_RIGHT_PAREN)) {
        do {
            std::shared_ptr<Statement> parameter = this->statement(false);
            if (parameter->rule != RULE_DECLARATION) {
                ADD_LOG_PAR(parameter->line, parameter->column, "Invalid argument when defining the function. Expected a variable declaration.");
                exit(logger->crash());
            }
            dest->push_back(std::move(std::static_pointer_cast<Declaration>(parameter)));
        } while (this->match(TOKEN_COMMA));
    }
}

std::vector<std::shared_ptr<Expression>> Parser::arguments()
{
    std::vector<std::shared_ptr<Expression>> arguments;
    if (!CHECK(TOKEN_RIGHT_PAREN)) {
        do arguments.push_back(std::move(this->expression()));
        while (this->match(TOKEN_COMMA));
    }
    return arguments;
}

std::vector<std::shared_ptr<Statement>> Parser::body()
{
    std::vector<std::shared_ptr<Statement>> body;
    while (!IS_AT_END()) {
        // Remove blank lines.
        while (this->match(TOKEN_NEW_LINE));
        // Check if the body ended.
        if (CHECK(TOKEN_RIGHT_BRACE)) break;
        // Push a new statement into the body.
        body.push_back(std::move(this->statement()));
    }
    return body;
}

std::vector<std::shared_ptr<Statement> > Parser::class_body()
{
    std::vector<std::shared_ptr<Statement> > body;
    while (!IS_AT_END() && !CHECK(TOKEN_RIGHT_BRACE)) {
        body.push_back(std::move(this->class_body_declaration()));
    }
    return body;
}

/*
type -> "[" type "]"
    | "{" type "}"
    | "(" type ("," type)* ("->" type)? ")"
    | IDENTIFIER;
*/
std::shared_ptr<Type> Parser::type(bool optional)
{
    if (this->match(TOKEN_LEFT_SQUARE)) {
        // List type.
        std::shared_ptr<Type> inner_type = this->type();
        std::shared_ptr<Type> type = std::make_shared<Type>(VALUE_LIST, inner_type);
        this->consume(TOKEN_RIGHT_SQUARE, "A list type needs to end with ']'");
        return type;
    } else if (this->match(TOKEN_LEFT_BRACE)) {
        // Dict type.
        std::shared_ptr<Type> inner_type = this->type();
        std::shared_ptr<Type> type = std::make_shared<Type>(VALUE_DICT, inner_type);
        this->consume(TOKEN_RIGHT_BRACE, "A dictionary type needs to end with '}'");
        return type;
    } else if (this->match(TOKEN_LEFT_PAREN)) {
        // Function type.
        if (this->match(TOKEN_RIGHT_PAREN)) {
            // Empty function (no paramenters or return type)
            return std::make_shared<Type>(VALUE_FUN);
        } else if (this->match(TOKEN_RIGHT_ARROW)) {
            // Function without arguments and only a return type.
            std::shared_ptr<Type> type = this->type();
            this->consume(TOKEN_RIGHT_PAREN, "A function type needs to end with ')'");
            return std::make_shared<Type>(VALUE_FUN, type);
        }
        // The function have arguments (+ return)?
        std::shared_ptr<Type> type = std::make_shared<Type>(VALUE_FUN);
        // Add the parameters.
        do type->parameters.push_back(std::move(this->type()));
        while (this->match(TOKEN_COMMA));
        // Add the return type if needed.
        if (this->match(TOKEN_RIGHT_ARROW)) type->inner_type = this->type();
        this->consume(TOKEN_RIGHT_PAREN, "A function type needs to end with ')'");
        return type;
    } else if (CHECK(TOKEN_IDENTIFIER)) {
        // Other types (native + custom).
        std::string type = this->consume(TOKEN_IDENTIFIER, "Expected an identifier as a type.")->to_string();
        return std::make_shared<Type>(type);
    } else if (optional && (CHECK(TOKEN_NEW_LINE) || CHECK(TOKEN_EQUAL))) return nullptr;

    ADD_LOG("Unknown type token expected.");
    exit(logger->crash());
}

/*
program -> top_level_declaration*;
*/
void Parser::parse(std::shared_ptr<std::vector<std::shared_ptr<Statement>>> &code)
{
    printf("----> Parser\n");
    // Add the file we are going to parse to the file_stack.
    file_stack.push_back(this->file);
    // Prepare the token list.
    std::unique_ptr<std::vector<Token>> tokens = std::make_unique<std::vector<Token>>();
    Lexer lexer = Lexer(this->file);
    // Scan the tokens.
    lexer.scan(tokens);
    Token::debug_tokens(*tokens);
    this->current = &tokens->front();
    while (!IS_AT_END()) code->push_back(std::move(this->top_level_declaration()));
    // Check the code size to avoid empty files.
    if (code->size() == 0) {
        logger->add_entity(this->file, 0, 0, "Empty file detected, you might need to check the file or make sure it's not empty.");
        exit(logger->crash());
    }
    parsed_files[*this->file] = std::make_pair(code, this->file);
    printf("----> !Parser\n");
}

Parser::Parser(const char *file)
{
    std::string source = std::string(file);
    Parser::format_path(source);
    this->file = std::move(std::make_shared<const std::string>(std::string(source)));
}

void Parser::format_path(std::string &path, const std::shared_ptr<const std::string> &parent)
{
    // Add the final .nu if needed.
    if (path.length() <= 3 || path.substr(path.length() - 3) != ".nu") path.append(".nu");
    // Join the parent if needed.
    std::filesystem::path f;

    // Path if local file
    if (parent) f = std::filesystem::path(*parent).remove_filename().append(path);
    else f = path;

    // Cehck if path exists
    if (!std::filesystem::exists(std::filesystem::absolute(f))) {
        // Check if path exists on std lib.
        f = std::filesystem::path(logger->executable_path).remove_filename().append("Lib").append(path);
        printf("STD PATH IS: %s\n", f.string().c_str());
        if (!std::filesystem::exists(std::filesystem::absolute(f))) {
            // Well... No module exists with that name.
            logger->add_entity(std::shared_ptr<const std::string>(), 0, 0, "Module " + path + " not found in any path.");
            exit(logger->crash());
        }
    }

    path = std::filesystem::absolute(f).string();
}

#undef CURRENT
#undef PREVIOUS
#undef CHECK
#undef NEXT
#undef IS_AT_END
#undef LOOKAHEAD
#undef LINE
#undef COL
#undef PLINE
#undef PCOL
#undef ADD_LOG
#undef ADD_LOG_PAR
#undef ADD_PREV_LOG
#undef EXPECT_NEW_LINE
#undef NEW_NODE
