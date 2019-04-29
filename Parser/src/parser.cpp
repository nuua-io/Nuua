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
#define NEW_NODE(type, ...) (new type(this->file, PLINE(), PCOL(), __VA_ARGS__))

// Stores the parsing file stack, to avoid
// cyclic imports.
static std::vector<const std::string *> file_stack;

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
static std::unordered_map<std::string, std::pair<std::vector<Statement *> *, const std::string *>> parsed_files;

Token *Parser::consume(TokenType type, const char *message)
{
    if (this->current->type == type) return NEXT();
    ADD_LOG(std::string(message));
    exit(logger->crash());
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
    if (this->match(TOKEN_FALSE)) return NEW_NODE(Boolean, false);
    if (this->match(TOKEN_TRUE)) return NEW_NODE(Boolean, true);
    if (this->match(TOKEN_INTEGER)) return NEW_NODE(Integer, std::stoll(PREVIOUS().to_string()));
    if (this->match(TOKEN_FLOAT)) return NEW_NODE(Float, std::stof(PREVIOUS().to_string()));
    if (this->match(TOKEN_STRING)) return NEW_NODE(String, PREVIOUS().to_string());
    if (this->match(TOKEN_IDENTIFIER)) return NEW_NODE(Variable, PREVIOUS().to_string());
    if (this->match(TOKEN_LEFT_SQUARE)) {
        std::vector<Expression *> values;
        if (this->match(TOKEN_RIGHT_SQUARE)) return NEW_NODE(List, values);
        for (;;) {
            if (IS_AT_END()) {
                ADD_LOG("Unfinished list, Expecting ']' after the last list element.");
                exit(logger->crash());
            }
            values.push_back(expression());
            if (this->match(TOKEN_RIGHT_SQUARE)) break;
            this->consume(TOKEN_COMMA, "Expected ',' after a list element");
        }
        return NEW_NODE(List, values);
    }
    if (this->match(TOKEN_LEFT_BRACE)) {
        std::unordered_map<std::string, Expression *> values;
        std::vector<std::string> keys;
        if (this->match(TOKEN_RIGHT_BRACE)) return NEW_NODE(Dictionary, values, keys);
        for (;;) {
            if (IS_AT_END()) {
                ADD_LOG("Unfinished dictionary, Expecting '}' after the last dictionary element.");
                exit(logger->crash());
            }
            Expression *key = this->expression();
            if (key->rule != RULE_VARIABLE) {
                ADD_LOG("Expected an identifier as a key");
                exit(logger->crash());
            }
            this->consume(TOKEN_COLON, "Expected ':' after dictionary key");
            std::string name = static_cast<Variable *>(key)->name;
            values[name] = this->expression();
            keys.push_back(name);
            if (this->match(TOKEN_RIGHT_BRACE)) break;
            this->consume(TOKEN_COMMA, "Expected ',' after dictionary element");
        }
        return NEW_NODE(Dictionary, values, keys);
    }
    if (this->match(TOKEN_LEFT_PAREN)) {
        Expression *value = this->expression();
        this->consume(TOKEN_RIGHT_PAREN, "Expected ')' after a group expression");
        return NEW_NODE(Group, value);
    }
    /*
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
Expression *Parser::unary_postfix()
{
    Expression *result = this->primary();
    while (this->match_any({{ TOKEN_LEFT_SQUARE, TOKEN_LEFT_PAREN }})) {
        Token op = PREVIOUS();
        switch (op.type) {
            case TOKEN_LEFT_SQUARE: {
                Expression *start = nullptr;
                Expression *end = nullptr;
                Expression *step = nullptr;
                if (this->match(TOKEN_COLON)) goto parser_is_slice1;
                start = this->expression();
                if (this->match(TOKEN_COLON)) {
                    // It's a Slice, not an access
                    parser_is_slice1:
                    if (this->match(TOKEN_RIGHT_SQUARE)) goto parser_finish_slice;
                    if (this->match(TOKEN_COLON)) goto parser_is_slice2;
                    end = this->expression();
                    if (this->match(TOKEN_COLON)) goto parser_is_slice3;
                    parser_is_slice2:
                    if (this->match(TOKEN_RIGHT_SQUARE)) goto parser_finish_slice;
                    step = this->expression();
                    parser_is_slice3:
                    this->consume(TOKEN_RIGHT_SQUARE, "Expected ']' after the slice access");
                    parser_finish_slice:
                    result = NEW_NODE(Slice, result, start, end, step);
                    break;
                }
                this->consume(TOKEN_RIGHT_SQUARE, "Expected ']' after the access index");
                result = NEW_NODE(Access, result, start);
                break;
            }
            case TOKEN_LEFT_PAREN: {
                std::vector<Expression *> arguments = this->arguments();
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
Expression *Parser::unary_prefix()
{
    if (this->match_any({{ TOKEN_BANG, TOKEN_PLUS, TOKEN_MINUS }})) {
        Token op = PREVIOUS();
        Expression *expr = this->unary_prefix();
        return NEW_NODE(Unary, op, expr);
    }
    return this->unary_postfix();
}

/*
cast -> unary_prefix ("as" type)*;
*/
Expression *Parser::cast()
{
    Expression *result = this->unary_prefix();
    while (this->match(TOKEN_AS)) {
        Type *type = this->type();
        result = NEW_NODE(Cast, result, type);
    }
    return result;
}

/*
multiplication -> cast (("/" | "*") cast)*;
*/
Expression *Parser::multiplication()
{
    Expression *result = this->cast();
    while (this->match_any({{ TOKEN_SLASH, TOKEN_STAR, TOKEN_PERCENT }})) {
        Token op = PREVIOUS();
        Expression *expr = this->cast();
        result = NEW_NODE(Binary, result, op, expr);
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
        Token op = PREVIOUS();
        Expression *expr = this->multiplication();
        result = NEW_NODE(Binary, result, op, expr);
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
        Token op = PREVIOUS();
        Expression *expr = this->addition();
        result = NEW_NODE(Binary, result, op, expr);
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
        Token op = PREVIOUS();
        Expression *expr = this->comparison();
        result = NEW_NODE(Binary, result, op, expr);
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
        Token op = PREVIOUS();
        Expression *expr = this->equality();
        result = NEW_NODE(Logical, result, op, expr);
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
        Token op = PREVIOUS();
        Expression *expr = this->logical_and();
        result = NEW_NODE(Logical, result, op, expr);
    }
    return result;
}

/*
range -> logical_or ((".." | "...") logical_or)*;
*/
Expression *Parser::range()
{
    Expression *result = this->logical_or();
    while (this->match_any({{ TOKEN_DOUBLE_DOT, TOKEN_TRIPLE_DOT }})) {
        Expression *right = this->expression();
        result = NEW_NODE(Range, result, right, PREVIOUS().type == TOKEN_DOUBLE_DOT ? false : true);
    }
    return result;
}

/*
assignment -> range ("=" range)*;
*/
Expression *Parser::assignment()
{
    Expression *result = this->range();
    while (this->match(TOKEN_EQUAL)) {
        Expression *expr = this->range();
        result = NEW_NODE(Assign, result, expr);
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
    Type *type = this->type();
    Expression *initializer = nullptr;
    if (this->match(TOKEN_EQUAL)) initializer = this->expression();
    return NEW_NODE(Declaration, variable, type, initializer);
}

/*
expression_statement -> expression;
*/
Statement *Parser::expression_statement()
{
    Expression *expr = this->expression();
    return NEW_NODE(ExpressionStatement, expr);
}

/*
use_declaration -> "use" IDENTIFIER ("," IDENTIFIER)* "from" STRING;
*/
Statement *Parser::use_declaration()
{
    std::vector<std::string> targets;
    targets.push_back(this->consume(TOKEN_IDENTIFIER, "Expected an identifier after 'use'")->to_string());
    while (this->match(TOKEN_COMMA)) targets.push_back(this->consume(TOKEN_IDENTIFIER, "Expected an identifier after ','")->to_string());
    this->consume(TOKEN_FROM, "Expected 'from' after the import target");
    std::string module = this->consume(TOKEN_STRING, "Expected an identifier after 'from'")->to_string();
    Parser::format_path(&module, this->file);
    Use *use;
    // Parse the contents of the target.
    if (parsed_files.find(module) == parsed_files.end()) {
        use = NEW_NODE(Use, targets, new std::string(module));
        PREVENT_CYCLIC(use->module);
        use->code = new std::vector<Statement *>;
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
Statement *Parser::export_declaration()
{
    Statement *stmt = this->top_level_declaration();
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
Statement *Parser::fun_declaration()
{
    std::string name = this->consume(TOKEN_IDENTIFIER, "Expected an identifier (function name) after 'fun'.")->to_string();
    this->consume(TOKEN_LEFT_PAREN, "Expected '(' after the function name.");
    std::vector<Declaration *> parameters;
    if (!this->match(TOKEN_RIGHT_PAREN)) {
        this->parameters(&parameters);
        this->consume(TOKEN_RIGHT_PAREN, "Expected ')' after the function parameters");
    }
    Type *return_type = nullptr;
    if (this->match(TOKEN_COLON)) return_type = this->type(false);
    std::vector<Statement *> body;
    if (this->match(TOKEN_RIGHT_ARROW)) {
        body.push_back(NEW_NODE(Return, this->expression()));
    } else if (this->match(TOKEN_BIG_RIGHT_ARROW)) {
        body.push_back(this->statement(false));
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
Statement *Parser::print_statement()
{
    Expression *expr = this->expression();
    return NEW_NODE(Print, expr);
}

/*
if_statement -> "if" expression ("=>" statement | "{" "\n" statement* "}");
*/
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
        ADD_LOG("Expected '{' or '=>' after 'if' condition.");
        exit(logger->crash());
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
            ADD_LOG("Expected '{' or '=>' after 'else'.");
            exit(logger->crash());
        }
    }
    return NEW_NODE(If, condition, then_branch, else_branch);
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
        ADD_LOG("Expected '{' or '=>' after 'while' condition.");
        exit(logger->crash());
    }
    return NEW_NODE(While, condition, body);
}

/*
for_statement -> "for" IDENTIFIER ("," IDENTIFIER)? "in" expression ("=>" statement | "{" "\n" statement* "}" "\n");
*/
Statement *Parser::for_statement()
{
    std::string index, variable = this->consume(TOKEN_IDENTIFIER, "Expected identifier after 'for'")->to_string();
    if (this->match(TOKEN_COMMA)) {
        index = this->consume(TOKEN_IDENTIFIER, "Expected identifier as the optional second identifier in 'for'")->to_string();
    }
    this->consume(TOKEN_IN, "Expected 'in' after 'for' identifier/s.");
    Expression *iterator = this->expression();
    std::vector<Statement *> body;
    if (this->match(TOKEN_LEFT_BRACE)) {
        EXPECT_NEW_LINE();
        body = this->body();
        this->consume(TOKEN_RIGHT_BRACE, "Expected '}' after 'for' body.");
    } else if (this->match(TOKEN_BIG_RIGHT_ARROW)) {
        body.push_back(this->statement(false));
    } else {
        ADD_LOG("Expected '{' or '=>' after 'for' iterator.");
        exit(logger->crash());
    }
    return NEW_NODE(For, variable, index, iterator, body);
}

/*
return_statement -> "return" expression?;
*/
Statement *Parser::return_statement()
{
    if (this->match_any({{ TOKEN_NEW_LINE, TOKEN_EOF }})) {
        return NEW_NODE(Return, nullptr);
    }
    Expression *expr = this->expression();
    return NEW_NODE(Return, expr);
}

Statement *Parser::class_statement()
{
    std::string name = this->consume(TOKEN_IDENTIFIER, "Expected identifier after 'class'.")->to_string();
    this->consume(TOKEN_LEFT_BRACE, "Expected '{' after 'class' name.");
    std::vector<Statement *> body = this->class_body();
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
Statement *Parser::statement(bool new_line)
{
    Statement *result;
    // Remove blank lines
    while (this->match(TOKEN_NEW_LINE));

    // Save the current line / column.
    uint32_t line = LINE();
    uint16_t column = COL();

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
Statement *Parser::top_level_declaration()
{
    Statement *result;
    // Remove blank lines
    while (this->match(TOKEN_NEW_LINE));

    // Save the current line / column.
    uint32_t line = LINE();
    uint16_t column = COL();

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

Statement *Parser::class_body_declaration()
{
    Statement *result;
    // Remove blank lines
    while (this->match(TOKEN_NEW_LINE));

    // Save the current line / column.
    uint32_t line = LINE();
    uint16_t column = COL();

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

void Parser::parameters(std::vector<Declaration *> *dest)
{
    if (!CHECK(TOKEN_RIGHT_PAREN)) {
        do {
            Statement *parameter = this->statement(false);
            if (parameter->rule != RULE_DECLARATION) {
                ADD_LOG_PAR(parameter->line, parameter->column, "Invalid argument when defining the function. Expected a variable declaration.");
                exit(logger->crash());
            }
            dest->push_back(static_cast<Declaration *>(parameter));
        } while (this->match(TOKEN_COMMA));
    }
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

std::vector<Statement *> Parser::class_body()
{
    std::vector<Statement *> body;
    while (!IS_AT_END() && !CHECK(TOKEN_RIGHT_BRACE)) {
        body.push_back(this->class_body_declaration());
    }
    return body;
}

/*
type -> "[" type "]"
    | "{" type "}"
    | "(" type ("," type)* ("->" type)? ")"
    | IDENTIFIER;
*/
Type *Parser::type(bool optional)
{
    Type *type;
    if (this->match(TOKEN_LEFT_SQUARE)) {
        // List type.
        type = new Type(VALUE_LIST, this->type());
        this->consume(TOKEN_RIGHT_SQUARE, "A list type needs to end with ']'");
        return type;
    } else if (this->match(TOKEN_LEFT_BRACE)) {
        // Dict type.
        type = new Type(VALUE_DICT, this->type());
        this->consume(TOKEN_RIGHT_BRACE, "A dictionary type needs to end with '}'");
        return type;
    } else if (this->match(TOKEN_LEFT_PAREN)) {
        // Function type.
        if (this->match(TOKEN_RIGHT_PAREN)) {
            // Empty function (no paramenters or return type)
            return new Type(VALUE_FUN);
        } else if (this->match(TOKEN_RIGHT_ARROW)) {
            // Function without arguments and only a return type.
            Type *t = this->type();
            this->consume(TOKEN_RIGHT_PAREN, "A function type needs to end with ')'");
            return new Type(VALUE_FUN, t);
        }
        // The function have arguments (+ return)?
        type = new Type(VALUE_FUN);
        // Add the parameters.
        do type->parameters.push_back(this->type());
        while (this->match(TOKEN_COMMA));
        // Add the return type if needed.
        if (this->match(TOKEN_RIGHT_ARROW)) type->inner_type = this->type();
        this->consume(TOKEN_RIGHT_PAREN, "A function type needs to end with ')'");
        return type;
    } else if (CHECK(TOKEN_IDENTIFIER)) {
        // Other types (native + custom).
        return new Type(this->consume(TOKEN_IDENTIFIER, "Expected an identifier as a type.")->to_string());
    } else if (optional && (CHECK(TOKEN_NEW_LINE) || CHECK(TOKEN_EQUAL))) return nullptr;

    ADD_LOG("Unknown type token expected.");
    exit(logger->crash());
}

/*
program -> top_level_declaration*;
*/
void Parser::parse(std::vector<Statement *> *code)
{
    std::vector<Token> *tokens = new std::vector<Token>;
    Lexer *lexer = new Lexer(this->file);
    // Scan the tokens.
    lexer->scan(tokens);
    // Token::debug_tokens(*tokens);
    this->current = &(*tokens)[0];
    while (!IS_AT_END()) code->push_back(this->top_level_declaration());
    // Check the code size to avoid empty files.
    if (code->size() == 0) {
        logger->add_entity(this->file, 0, 0, "Empty file detected, you might need to check the file or make sure it's not empty.");
        exit(logger->crash());
    }
    parsed_files[*this->file] = std::make_pair(code, this->file);
    delete lexer;
}

Parser::Parser(const char *file)
{
    std::string source = std::string(file);
    Parser::format_path(&source);
    this->file = new std::string(source);
}

void Parser::format_path(std::string *path, const std::string *parent)
{
    // Add the final .nu if needed.
    if (path->substr(path->length() - 3) != ".nu") path->append(".nu");
    // Join the parent if needed.
    std::filesystem::path f;
    if (parent) f = std::filesystem::path(*parent).remove_filename().string() + *path;
    else f = *path;
    /*
    printf(
        "Path: %s\nRelative: %ls\nAbsolute: %ls\nNoFilename: %ls\n",
        path->c_str(),
        f.c_str(),
        std::filesystem::absolute(f).c_str(),
        std::filesystem::absolute(f).remove_filename().c_str()
    );
    */
   *path = std::filesystem::absolute(f).string();
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
