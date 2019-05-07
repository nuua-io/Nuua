/**
 * |------------|
 * | Nuua Rules |
 * |------------|
 *
 * Copyright 2019 Erik Campobadal <soc@erik.cat>
 * https://nuua.io
 */
#ifndef RULES_HPP
#define RULES_HPP

#include "../../Lexer/include/tokens.hpp"
#include "block.hpp"
// #include "type.hpp"
#include <string>
#include <vector>
#include <unordered_map>

// Forward declaration
class Type;

typedef enum : uint8_t {
    RULE_EXPRESSION,
    RULE_STATEMENT,
    RULE_PRINT,
    RULE_EXPRESSION_STATEMENT,
    RULE_INTEGER,
    RULE_UNSIGNED,
    RULE_FLOAT,
    RULE_STRING,
    RULE_BOOLEAN,
    RULE_LIST,
    RULE_DICTIONARY,
    RULE_GROUP,
    RULE_UNARY,
    RULE_BINARY,
    RULE_DECLARATION,
    RULE_VARIABLE,
    RULE_ASSIGN,
    RULE_LOGICAL,
    RULE_FUNCTION,
    RULE_CALL,
    RULE_ACCESS,
    RULE_RETURN,
    RULE_IF,
    RULE_WHILE,
    RULE_FOR,
    RULE_CAST,
    RULE_USE,
    RULE_EXPORT,
    RULE_CLASS,
    RULE_SLICE,
    RULE_RANGE,
} Rule;

typedef enum : uint8_t {
    CAST_INT_FLOAT, // INT -> FLOAT
    CAST_INT_BOOL, // INT -> BOOL
    CAST_INT_STRING, // INT -> STRING
    CAST_FLOAT_INT, // FLOAT -> INT
    CAST_FLOAT_BOOL, // FLOAT -> BOOL
    CAST_FLOAT_STRING, // FLOAT -> STRING
    CAST_BOOL_INT, // BOOL -> INT
    CAST_BOOL_FLOAT, // BOOL -> FLOAT
    CAST_BOOL_STRING, // BOOL -> STRING
    CAST_LIST_STRING, // LIST -> STRING
    CAST_LIST_BOOL, // LIST -> BOOL
    CAST_DICT_STRING, // DICT -> STRING
    CAST_DICT_BOOL, // DICT -> BOOL
    CAST_STRING_BOOL, // STRING -> BOOL
} CastType;

typedef enum : uint8_t {
    // Negation
    UNARY_NEG_BOOL, // ! BOOL -> BOOL
    // Minus operations
    UNARY_MINUS_INT, // - INT -> INT
    UNARY_MINUS_FLOAT, // - FLOAT -> FLOAT
    UNARY_MINUS_BOOL, // - BOOL -> INT
    // Plus operations
    UNARY_PLUS_INT, // + INT -> INT
    UNARY_PLUS_FLOAT, // + FLOAT -> FLOAT
    UNARY_PLUS_BOOL, // + BOOL -> INT
} UnaryType;

typedef enum : uint8_t {
    // Addition
    BINARY_ADD_INT, // INT + INT -> INT
    BINARY_ADD_FLOAT, // FLOAT + FLOAT -> FLOAT
    BINARY_ADD_STRING, // STRING + STRING -> STRING
    BINARY_ADD_BOOL, // BOOL + BOOL -> INT
    BINARY_ADD_LIST, // LIST + LIST -> LIST
    BINARY_ADD_DICT, // DICT + DICT -> DICT
    // Substraction
    BINARY_SUB_INT, // INT - INT -> INT
    BINARY_SUB_FLOAT, // FLOAT - FLOAT -> FLOAT
    BINARY_SUB_BOOL, // BOOL - BOOL -> INT
    // Multiplication
    BINARY_MUL_INT, // INT * INT -> INT
    BINARY_MUL_FLOAT, // FLOAT * FLOAT -> FLOAT
    BINARY_MUL_BOOL, // BOOL * BOOL -> INT
    BINARY_MUL_INT_STRING, // INT * STRING -> STRING
    BINARY_MUL_STRING_INT, // STRING * INT -> STRING
    BINARY_MUL_INT_LIST, // INT * LIST -> LIST
    BINARY_MUL_LIST_INT, // LIST * INT -> LIST
    // Division
    BINARY_DIV_INT, // INT / INT -> FLOAT
    BINARY_DIV_FLOAT, // FLOAT / FLOAT -> FLOAT
    BINARY_DIV_STRING_INT, // STRING / INT -> LIST
    BINARY_DIV_LIST_INT, // LIST / INT -> LIST[LIST]
    // Equality
    BINARY_EQ_INT, // INT == INT -> BOOL
    BINARY_EQ_FLOAT, // FLOAT == FLOAT -> BOOL
    BINARY_EQ_STRING, // STRING == STRING -> BOOL
    BINARY_EQ_BOOL, // BOOL == BOOL -> BOOL
    BINARY_EQ_LIST, // LIST == LIST -> BOOL
    BINARY_EQ_DICT, // DICT == DICT -> BOOL
    // Not Equality
    BINARY_NEQ_INT, // INT != INT -> BOOL
    BINARY_NEQ_FLOAT, // FLOAT != FLOAT -> BOOL
    BINARY_NEQ_STRING, // STRING != STRING -> BOOL
    BINARY_NEQ_BOOL, // BOOL != BOOL -> BOOL
    BINARY_NEQ_LIST, // LIST != LIST -> BOOL
    BINARY_NEQ_DICT, // DICT != DICT -> BOOL
    // Higher than
    BINARY_HT_INT, // INT > INT -> BOOL
    BINARY_HT_FLOAT, // FLOAT > FLOAT -> BOOL
    BINARY_HT_STRING, // STRING > STRING -> BOOL
    BINARY_HT_BOOL, // BOOL > BOOL -> BOOL
    // Higher than or equal to
    BINARY_HTE_INT, // INT >= INT -> BOOL
    BINARY_HTE_FLOAT, // FLOAT >= FLOAT -> BOOL
    BINARY_HTE_STRING, // STRING >= STRING -> BOOL
    BINARY_HTE_BOOL, // BOOL >= BOOL -> BOOL
    // Lower than
    BINARY_LT_INT, // INT < INT -> BOOL
    BINARY_LT_FLOAT, // FLOAT < FLOAT -> BOOL
    BINARY_LT_STRING, // STRING < STRING -> BOOL
    BINARY_LT_BOOL, // BOOL < BOOL -> BOOL
    // Lower than or equal to
    BINARY_LTE_INT, // INT <= INT -> BOOL
    BINARY_LTE_FLOAT, // FLOAT <= FLOAT -> BOOL
    BINARY_LTE_STRING, // STRING <= STRING -> BOOL
    BINARY_LTE_BOOL, // BOOL <= BOOL -> BOOL
} BinaryType;

class Node
{
    public:
        const Rule rule;
        const std::string *file;
        line_t line;
        column_t column;
        Node(const Rule rule, const std::string *file, const line_t line, const uint32_t column)
            : rule(rule), file(file), line(line), column(column) {};
};

class Expression : public Node
{
    public:
        Expression(Node node)
            : Node(node) {};
};

class Statement : public Node
{
    public:
        Statement(Node node)
            : Node(node) {};
};

/* Expressions */
class Integer : public Expression
{
    public:
        int64_t value;
        Integer(const std::string *file, const line_t line, const uint32_t column, int64_t value)
            : Expression({ RULE_INTEGER, file, line, column }), value(value) {};
};

class Float : public Expression
{
    public:
        double value;
        Float(const std::string *file, const line_t line, const uint32_t column, double value)
            : Expression({ RULE_FLOAT, file, line, column }), value(value) {};
};

class String : public Expression
{
    public:
        std::string value;

        String(const std::string *file, const line_t line, const uint32_t column, std::string value)
            : Expression({ RULE_STRING, file, line, column }), value(value) {};
};

class Boolean : public Expression
{
    public:
        bool value;
        Boolean(const std::string *file, const line_t line, const uint32_t column, bool value)
            : Expression({ RULE_BOOLEAN, file, line, column }), value(value) {};
};

class List : public Expression
{
    public:
        std::vector<Expression *> value;
        Type *type = nullptr; // Stores the list type since it's complex to analyze later.
        List(const std::string *file, const line_t line, const uint32_t column, std::vector<Expression *> value)
            : Expression({ RULE_LIST, file, line, column }), value(value) {};
};

class Dictionary : public Expression
{
    public:
        std::unordered_map<std::string, Expression *> value;
        std::vector<std::string> key_order;
        Type *type = nullptr; // Stores the dict type since it's complex to analyze later.
        Dictionary(const std::string *file, const line_t line, const uint32_t column, std::unordered_map<std::string, Expression *> value, std::vector<std::string> key_order)
            : Expression({ RULE_DICTIONARY, file, line, column }), value(value), key_order(key_order) {};
};

class Group : public Expression
{
    public:
        Expression *expression;
        Group(const std::string *file, const line_t line, const uint32_t column, Expression *value)
            : Expression({ RULE_GROUP, file, line, column }), expression(value) {};
};

class Unary : public Expression
{
    public:
        Token op;
        Expression *right = nullptr;
        UnaryType type; // Determines what type of unary operation will be performed, no need to store a whole Type.
        Unary(const std::string *file, const line_t line, const uint32_t column, Token op, Expression *right)
            : Expression({ RULE_UNARY, file, line, column }), op(op), right(right) {};
};

class Binary : public Expression
{
    public:
        Expression* left = nullptr;
        Token op;
        Expression* right = nullptr;
        BinaryType type; // Determines what type of binary operation will be performed.
        Binary(const std::string *file, const line_t line, const uint32_t column, Expression *left, Token op, Expression *right)
            : Expression({ RULE_BINARY, file, line, column }), left(left), op(op), right(right) {};
};

class Variable : public Expression
{
    public:
        std::string name;
        Variable(const std::string *file, const line_t line, const uint32_t column, std::string name)
            : Expression({ RULE_VARIABLE, file, line, column }), name(name) {};
};

class Assign : public Expression
{
    public:
        Expression *target;
        Expression *value;
        Assign(const std::string *file, const line_t line, const uint32_t column, Expression *target, Expression *value)
            : Expression({ RULE_ASSIGN, file, line, column }), target(target), value(value) {};
};

class Logical : public Expression
{
    public:
        Expression *left;
        Token op;
        Expression *right;
        Logical(const std::string *file, const line_t line, const uint32_t column, Expression *left, Token op, Expression *right)
            : Expression({ RULE_LOGICAL, file, line, column }), left(left),  op(op), right(right) {};
};

class Call : public Expression
{
    public:
        Expression *target;
        std::vector<Expression *> arguments;
        Call(const std::string *file, const line_t line, const uint32_t column, Expression *target, std::vector<Expression *> arguments)
            : Expression({ RULE_CALL, file, line, column }), target(target), arguments(arguments) {};
};

class Access : public Expression
{
    public:
        Expression *target;
        Expression *index;
        bool integer_index; // Determines if it needs an integer or string to access.
        Access(const std::string *file, const line_t line, const uint32_t column, Expression *target, Expression *index)
            : Expression({ RULE_ACCESS, file, line, column }), target(target), index(index) {};
};

class Cast : public Expression
{
    public:
        Expression *expression;
        Type *type;
        CastType cast_type;
        Cast(const std::string *file, const line_t line, const uint32_t column, Expression *expression, Type *type)
            : Expression({ RULE_CAST, file, line, column }), expression(expression), type(type) {}
};

/*
class Closure : public Expression
{
    public:
        std::vector<Statement *> parameters;
        std::string return_type;
        std::vector<Statement *> body;
        Block block;
        Closure(const std::string *file, const line_t line, const uint32_t column, std::vector<Statement *> parameters, std::string return_type, std::vector<Statement *> body)
            : Expression(RULE_CLOSURE, file, line, column), parameters(parameters), return_type(return_type), body(body) {}
};
*/

class Slice : public Expression
{
    public:
        Expression *target;
        Expression *start;
        Expression *end;
        Expression *step;
        bool is_list; // Determines if it's a list or a string, used by Analyzer.
        Slice(const std::string *file, const line_t line, const uint32_t column, Expression *target, Expression *start, Expression *end, Expression *step)
            : Expression({ RULE_SLICE, file, line, column }), target(target), start(start), end(end), step(step) {}
};

class Range : public Expression
{
    public:
        Expression *start;
        Expression *end;
        bool inclusive;
        Range(const std::string *file, const line_t line, const uint32_t column, Expression *start, Expression *end, bool inclusive)
            : Expression({ RULE_RANGE, file, line, column }), start(start), end(end), inclusive(inclusive) {}
};

/* Statements */
class Print : public Statement
{
    public:
        Expression *expression;
        Print(const std::string *file, const line_t line, const uint32_t column, Expression *expression)
            : Statement({ RULE_PRINT, file, line, column }), expression(expression) {}
};

class ExpressionStatement : public Statement
{
    public:
        Expression *expression;
        ExpressionStatement(const std::string *file, const line_t line, const uint32_t column, Expression *expression)
            : Statement({ RULE_EXPRESSION_STATEMENT, file, line, column }), expression(expression) {}
};

class Declaration : public Statement
{
    public:
        std::string name;
        Type *type;
        Expression *initializer;
        Declaration(const std::string *file, const line_t line, const uint32_t column, std::string name, Type *type, Expression *initializer)
            : Statement({ RULE_DECLARATION, file, line, column }), name(name), type(type), initializer(initializer) {};
};

class Return : public Statement
{
    public:
        Expression *value;
        Return(const std::string *file, const line_t line, const uint32_t column, Expression *value = nullptr)
            : Statement({ RULE_RETURN, file, line, column }), value(value) {}
};

class If : public Statement
{
    public:
        Expression *condition;
        std::vector<Statement *> then_branch;
        std::vector<Statement *> else_branch;
        Block then_block, else_block;
        If(const std::string *file, const line_t line, const uint32_t column, Expression *condition, std::vector<Statement *> then_branch, std::vector<Statement *> else_branch)
            : Statement({ RULE_IF, file, line, column }), condition(condition), then_branch(then_branch), else_branch(else_branch) {};
};

class While : public Statement
{
    public:
        Expression *condition;
        std::vector<Statement *> body;
        Block block;
        While(const std::string *file, const line_t line, const uint32_t column, Expression *condition, std::vector<Statement *> body)
            : Statement({ RULE_WHILE, file, line, column }), condition(condition), body(body) {};
};

class For : public Statement
{
    public:
        std::string variable;
        std::string index;
        Expression *iterator;
        std::vector<Statement *> body;
        Block block;
        For(const std::string *file, const line_t line, const uint32_t column, std::string variable, std::string index, Expression *iterator, std::vector<Statement *> body)
            : Statement({ RULE_FOR, file, line, column }), variable(variable), index(index), iterator(iterator), body(body) {}
};

class Function : public Statement
{
    public:
        std::string name;
        std::vector<Declaration *> parameters;
        Type *return_type;
        std::vector<Statement *> body;
        Block block;
        Function(const std::string *file, const line_t line, const uint32_t column, std::string name, std::vector<Declaration *> parameters, Type *return_type, std::vector<Statement *> body)
            : Statement({ RULE_FUNCTION, file, line, column }), name(name), parameters(parameters), return_type(return_type), body(body) {}
};

class Use : public Statement
{
    public:
        std::vector<std::string> targets;
        const std::string *module = nullptr;
        std::vector<Statement *> *code = nullptr;
        Block block;
        Use(const std::string *file, const line_t line, const uint32_t column, std::vector<std::string> targets, const std::string *module)
            : Statement({ RULE_USE, file, line, column }), targets(targets), module(module) {};
};

class Export : public Statement
{
    public:
        Statement *statement;
        Export(const std::string *file, const line_t line, const uint32_t column, Statement *statement)
            : Statement({ RULE_EXPORT, file, line, column }), statement(statement) {}
};

class Class : public Statement
{
    public:
        std::string name;
        std::vector<Statement *> body;
        Class(const std::string *file, const line_t line, const uint32_t column, std::string name, std::vector<Statement *> body)
            : Statement({ RULE_CLASS, file, line, column }), name(name), body(body) {}
};

#endif
