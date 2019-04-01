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
#include <string>
#include <vector>
#include <unordered_map>

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
    RULE_CLOSURE,
} Rule;

class Expression
{
    public:
        const Rule rule;
        const std::string *file;
        const uint32_t line;
        Expression(const Rule rule, const std::string *file, const uint32_t line)
            : rule(rule), file(file), line(line) {};
};

class Statement
{
    public:
        const Rule rule;
        const std::string *file;
        const uint32_t line;
        Statement(const Rule rule, const std::string *file, const uint32_t line)
            : rule(rule), file(file), line(line) {};
};

/* Expressions */
class Integer : public Expression
{
    public:
        int64_t value;
        Integer(const std::string *file, const uint32_t line, int64_t value)
            : Expression(RULE_INTEGER, file, line), value(value) {};
};

class Unsigned : public Expression
{
    public:
        uint64_t value;
        Unsigned(const std::string *file, const uint32_t line, uint64_t value)
            : Expression(RULE_UNSIGNED, file, line), value(value) {};
};

class Float : public Expression
{
    public:
        double value;
        Float(const std::string *file, const uint32_t line, double value)
            : Expression(RULE_FLOAT, file, line), value(value) {};
};

class String : public Expression
{
    public:
        std::string value;

        String(const std::string *file, const uint32_t line, std::string value)
            : Expression(RULE_STRING, file, line), value(value) {};
};

class Boolean : public Expression
{
    public:
        bool value;
        Boolean(const std::string *file, const uint32_t line, bool value)
            : Expression(RULE_BOOLEAN, file, line), value(value) {};
};

class List : public Expression
{
    public:
        std::vector<Expression *> value;
        List(const std::string *file, const uint32_t line, std::vector<Expression *> value)
            : Expression(RULE_LIST, file, line), value(value) {};
};

class Dictionary : public Expression
{
    public:
        std::unordered_map<std::string, Expression *> value;
        std::vector<std::string> key_order;
        Dictionary(const std::string *file, const uint32_t line, std::unordered_map<std::string, Expression *> value, std::vector<std::string> key_order)
            : Expression(RULE_DICTIONARY, file, line), value(value), key_order(key_order) {};
};

class Group : public Expression
{
    public:
        Expression *expression;
        Group(const std::string *file, const uint32_t line, Expression *value)
            : Expression(RULE_GROUP, file, line), expression(value) {};
};

class Unary : public Expression
{
    public:
        Token op;
        Expression *right;
        Unary(const std::string *file, const uint32_t line, Token op, Expression *right)
            : Expression(RULE_UNARY, file, line), op(op), right(right) {};
};

class Binary : public Expression
{
    public:
        Expression* left;
        Token op;
        Expression* right;
        Binary(const std::string *file, const uint32_t line, Expression *left, Token op, Expression *right)
            : Expression(RULE_BINARY, file, line), left(left), op(op), right(right) {};
};

class Variable : public Expression
{
    public:
        std::string name;
        Variable(const std::string *file, const uint32_t line, std::string name)
            : Expression(RULE_VARIABLE, file, line), name(name) {};
};

class Assign : public Expression
{
    public:
        Expression *target;
        Expression *value;
        Assign(const std::string *file, const uint32_t line, Expression *target, Expression *value)
            : Expression(RULE_ASSIGN, file, line), target(target), value(value) {};
};

class Logical : public Expression
{
    public:
        Expression *left;
        Token op;
        Expression *right;
        Logical(const std::string *file, const uint32_t line, Expression *left, Token op, Expression *right)
            : Expression(RULE_LOGICAL, file, line), left(left),  op(op), right(right) {};
};

class Call : public Expression
{
    public:
        Expression *target;
        std::vector<Expression *> arguments;
        Call(const std::string *file, const uint32_t line, Expression *target, std::vector<Expression *> arguments)
            : Expression(RULE_CALL, file, line), target(target), arguments(arguments) {};
};

class Access : public Expression
{
    public:
        Expression *target;
        Expression *index;
        bool integer_index; // Determines if it needs an integer or string to access.
        Access(const std::string *file, const uint32_t line, Expression *target, Expression *index)
            : Expression(RULE_ACCESS, file, line), target(target), index(index) {};
};

class Cast : public Expression
{
    public:
        Expression *expression;
        std::string type;
        Cast(const std::string *file, const uint32_t line, Expression *expression, std::string type)
            : Expression(RULE_CAST, file, line), expression(expression), type(type) {}
};

class Closure : public Expression
{
    public:
        std::vector<Statement *> parameters;
        std::string return_type;
        std::vector<Statement *> body;
        Block block;
        Closure(const std::string *file, const uint32_t line, std::vector<Statement *> parameters, std::string return_type, std::vector<Statement *> body)
            : Expression(RULE_CLOSURE, file, line), parameters(parameters), return_type(return_type), body(body) {}
};

/* Statements */
class Print : public Statement
{
    public:
        Expression *expression;
        Print(const std::string *file, const uint32_t line, Expression *expression)
            : Statement(RULE_PRINT, file, line), expression(expression) {}
};

class ExpressionStatement : public Statement
{
    public:
        Expression *expression;
        ExpressionStatement(const std::string *file, const uint32_t line, Expression *expression)
            : Statement(RULE_EXPRESSION_STATEMENT, file, line), expression(expression) {}
};

class Declaration : public Statement
{
    public:
        std::string name;
        std::string type;
        Expression *initializer;
        Declaration(const std::string *file, const uint32_t line, std::string name, std::string type, Expression *initializer)
            : Statement(RULE_DECLARATION, file, line), name(name), type(type), initializer(initializer) {};
};

class Return : public Statement
{
    public:
        Expression *value;
        Return(const std::string *file, const uint32_t line, Expression *value)
            : Statement(RULE_RETURN, file, line), value(value) {}
};

class If : public Statement
{
    public:
        Expression *condition;
        std::vector<Statement *> then_branch;
        std::vector<Statement *> else_branch;
        Block then_block, else_block;
        If(const std::string *file, const uint32_t line, Expression *condition, std::vector<Statement *> then_branch, std::vector<Statement *> else_branch)
            : Statement(RULE_IF, file, line), condition(condition), then_branch(then_branch), else_branch(else_branch) {};
};

class While : public Statement
{
    public:
        Expression *condition;
        std::vector<Statement *> body;
        Block block;
        While(const std::string *file, const uint32_t line, Expression *condition, std::vector<Statement *> body)
            : Statement(RULE_WHILE, file, line), condition(condition), body(body) {};
};

class For : public Statement
{
    public:
        std::string variable;
        std::string index;
        Expression *iterator;
        std::vector<Statement *> body;
        For(const std::string *file, const uint32_t line, std::string variable, std::string index, Expression *iterator, std::vector<Statement *> body)
            : Statement(RULE_FOR, file, line), variable(variable), index(index), iterator(iterator), body(body) {}
};

class Function : public Statement
{
    public:
        std::string name;
        std::vector<Statement *> parameters;
        std::string return_type;
        std::vector<Statement *> body;
        Block block;
        Function(const std::string *file, const uint32_t line, std::string name, std::vector<Statement *> parameters, std::string return_type, std::vector<Statement *> body)
            : Statement(RULE_FUNCTION, file, line), name(name), parameters(parameters), return_type(return_type), body(body) {}
};

class Use : public Statement
{
    public:
        std::vector<std::string> targets;
        const std::string *module;
        std::vector<Statement *> *code;
        Use(const std::string *file, const uint32_t line, std::vector<std::string> targets, const std::string *module)
            : Statement(RULE_USE, file, line), targets(targets), module(module) {};
};

class Export : public Statement
{
    public:
        Statement *statement;
        Export(const std::string *file, const uint32_t line, Statement *statement)
            : Statement(RULE_EXPORT, file, line), statement(statement) {}
};

#endif
