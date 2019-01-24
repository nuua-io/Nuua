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
#include <string>
#include <vector>
#include <unordered_map>

typedef enum : uint8_t {
    RULE_EXPRESSION,
    RULE_STATEMENT,
    RULE_PRINT,
    RULE_EXPRESSION_STATEMENT,
    RULE_INTEGER,
    RULE_FLOAT,
    RULE_STRING,
    RULE_BOOLEAN,
    RULE_LIST,
    RULE_DICTIONARY,
    RULE_NONE,
    RULE_GROUP,
    RULE_UNARY,
    RULE_BINARY,
    RULE_DECLARATION,
    RULE_VARIABLE,
    RULE_ASSIGN,
    RULE_ASSIGN_ACCESS,
    RULE_LOGICAL,
    RULE_FUNCTION,
    RULE_CALL,
    RULE_ACCESS,
    RULE_RETURN,
    RULE_IF,
    RULE_WHILE,
    RULE_CAST,
} Rule;

class Expression
{
    public:
        uint32_t line;
        Rule rule;

        Expression(Rule rule = RULE_EXPRESSION)
            : rule(rule) {};
};

class Statement
{
    public:
        uint32_t line;
        Rule rule;

        Statement(Rule rule = RULE_STATEMENT)
            : rule(rule) {};
};

/* Expressions */
class Integer : public Expression
{
    public:
        int64_t value;

        Integer(int64_t value)
            : Expression(RULE_INTEGER), value(value) {};
};

class Float : public Expression
{
    public:
        double value;

        Float(double value)
            : Expression(RULE_FLOAT), value(value) {};
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
        std::vector<Statement *> arguments;
        std::string return_type;
        std::vector<Statement *> body;

        Function(std::vector<Statement *> arguments, std::string return_type, std::vector<Statement *> body)
            : Expression(RULE_FUNCTION), arguments(arguments), return_type(return_type), body(body) {}
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

class Cast : public Expression
{
    public:
        Expression *expression;
        std::string type;

        Cast(Expression *expression, std::string type)
            : Expression(RULE_CAST), expression(expression), type(type) {}
};

/* Statements */
class Print : public Statement
{
    public:
        Expression *expression;

        Print(Expression *expression)
            : Statement(RULE_PRINT), expression(expression) {}
};

class ExpressionStatement : public Statement
{
    public:
        Expression *expression;

        ExpressionStatement(Expression *expression)
            : Statement(RULE_EXPRESSION_STATEMENT), expression(expression) {}
};

class Declaration : public Statement
{
    public:
        std::string name;
        std::string type;
        Expression *initializer;

        Declaration(std::string name, std::string type, Expression *initializer)
            : Statement(RULE_DECLARATION), name(name), type(type), initializer(initializer) {};
};

class Return : public Statement
{
    public:
        Expression *value;

        Return(Expression *value)
            : Statement(RULE_RETURN), value(value) {}
};

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

void debug_rules(std::vector<Rule> rules);
void debug_rules(std::vector<Statement *> rules);

#endif
