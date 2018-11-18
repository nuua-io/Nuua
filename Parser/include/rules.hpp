/**
 * |------------|
 * | Nuua Rules |
 * |------------|
 *
 * Copyright 2018 Erik Campobadal <soc@erik.cat>
 * https://nuua.io
 */

#ifndef RULES_HPP
#define RULES_HPP

// #include "program.hpp"
#include "../../Lexer/include/tokens.hpp"
#include <string>
#include <vector>
#include <unordered_map>

typedef enum : uint8_t {
    RULE_EXPRESSION,
    RULE_STATEMENT,
    RULE_EXPRESSION_STATEMENT,
    RULE_NUMBER,
    RULE_STRING,
    RULE_BOOLEAN,
    RULE_LIST,
    RULE_DICTIONARY,
    RULE_NONE,
    RULE_GROUP,
    RULE_UNARY,
    RULE_BINARY,
    RULE_VARIABLE,
    RULE_ASSIGN,
    RULE_ASSIGN_ACCESS,
    RULE_LOGICAL,
    RULE_FUNCTION,
    RULE_CALL,
    RULE_ACCESS,
    RULE_IF,
    RULE_WHILE,
    RULE_UNROLL,
} Rule;

class Expression
{
    public:
        uint32_t line;
        Rule rule;

        Expression(Rule rule = RULE_EXPRESSION) : rule(rule) {};
};

class Statement
{
    public:
        uint32_t line;
        Rule rule;

        Statement(Rule rule = RULE_STATEMENT)
            : rule(rule) {};
};

class ExpressionStatement : public Statement
{
    public:
        Expression *expression;

        ExpressionStatement(Expression *expression)
            : Statement(RULE_EXPRESSION_STATEMENT), expression(expression) {}
};

class Number : public Expression
{
    public:
        double value;

        Number(double value)
            : Expression(RULE_NUMBER), value(value) {};
};

class String : public Expression
{
    public:
        std::string value;

        String(std::string value)
            : Expression(RULE_STRING), value(value) {};
};

class Boolean : public Expression
{
    public:
        bool value;

        Boolean(bool value)
            : Expression(RULE_BOOLEAN), value(value) {};
};

class List : public Expression
{
    public:
        std::vector<Expression *> value;

        List(std::vector<Expression *> value)
            : Expression(RULE_LIST), value(value) {};
};

class Dictionary : public Expression
{
    public:
        std::unordered_map<std::string, Expression *> value;
        std::vector<std::string> key_order;

        Dictionary(std::unordered_map<std::string, Expression *> value, std::vector<std::string> key_order)
            : Expression(RULE_DICTIONARY), value(value), key_order(key_order) {};
};

class None : public Expression
{
    public:
        None()
            : Expression(RULE_NONE) {};
};

class Group : public Expression
{
    public:
        Expression *expression;

        Group(Expression *value)
            : Expression(RULE_GROUP), expression(value) {};
};

class Unary : public Expression
{
    public:
        Token op; //! This should be changed to a TOKEN rule
        Expression *right;

        Unary(Token op, Expression *right)
            : Expression(RULE_UNARY), op(op), right(right) {};
};

class Binary : public Expression
{
    public:
        Expression* left;
        Token op;
        Expression* right;

        Binary(Expression *left, Token op, Expression *right)
            : Expression(RULE_BINARY), left(left), op(op), right(right) {};
};

class Variable : public Expression
{
    public:
        std::string name;

        Variable(std::string name)
            : Expression(RULE_VARIABLE), name(name) {};
};

class Assign : public Expression
{
    public:
        std::string name;
        Expression *value;

        Assign(std::string name, Expression *value)
            : Expression(RULE_ASSIGN), name(name), value(value) {};
};

class AssignAccess : public Expression
{
    public:
        std::string name;
        Expression *index;
        Expression *value;

        AssignAccess(std::string name, Expression *index, Expression *value)
            : Expression(RULE_ASSIGN_ACCESS), name(name), index(index), value(value) {};
};

class Logical : public Expression
{
    public:
        Expression *left;
        Token op;
        Expression *right;

        Logical(Expression *left, Token op, Expression *right)
            : Expression(RULE_LOGICAL), left(left),  op(op), right(right) {};
};

class Function : public Expression
{
    public:
        std::vector<Expression *> arguments;
        std::vector<Statement *> body;

        Function(std::vector<Expression *> arguments, std::vector<Statement *> body)
            : Expression(RULE_FUNCTION), arguments(arguments), body(body) {}
};

class Call : public Expression
{
    public:
        std::string callee;
        std::vector<Expression *> arguments;

        Call(std::string callee, std::vector<Expression *> arguments)
            : Expression(RULE_CALL), callee(callee), arguments(arguments) {};
};

class Access : public Expression
{
    public:
        std::string name;
        Expression *index;

        Access(std::string name, Expression *index)
            : Expression(RULE_ACCESS), name(name), index(index) {};
};

/* Statements */
class If : public Statement
{
    public:
        Expression *condition;
        std::vector<Statement *> thenBranch;
        std::vector<Statement *> elseBranch;

        If(Expression *condition, std::vector<Statement *> thenBranch, std::vector<Statement *> elseBranch)
            : Statement(RULE_IF), condition(condition), thenBranch(thenBranch), elseBranch(elseBranch) {};
};

class While : public Statement
{
    public:
        Expression *condition;
        std::vector<Statement *> body;

    While(Expression *condition, std::vector<Statement *> body)
        : Statement(RULE_WHILE), condition(condition), body(body) {};
};

class Unroll : public Statement
{
    public:
        uint32_t iterations;
        uint32_t chunks;
        std::vector<Statement *> body;

    Unroll(uint32_t iterations, uint32_t chunks, std::vector<Statement *> body)
        : Statement(RULE_UNROLL), iterations(iterations), chunks(chunks), body(body) {};
};

void debug_rules(std::vector<Rule> rules);
void debug_rules(std::vector<Statement *> rules);

#endif
