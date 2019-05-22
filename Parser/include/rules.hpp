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
    RULE_DELETE,
    RULE_LENGTH,
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
    CAST_LIST_INT, // LIST -> INT
    CAST_DICT_STRING, // DICT -> STRING
    CAST_DICT_BOOL, // DICT -> BOOL
    CAST_DICT_INT, // DICT -> INT
    CAST_STRING_BOOL, // STRING -> BOOL
    CAST_STRING_INT, // STRING -> INT
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

typedef enum : uint8_t {
    ACCESS_STRING,
    ACCESS_LIST,
    ACCESS_DICT,
} AccessType;

class Node
{
    public:
        const Rule rule;
        std::shared_ptr<const std::string> file;
        line_t line;
        column_t column;
        Node(const Rule rule, std::shared_ptr<const std::string> &file, const line_t line, const column_t column)
            : rule(rule), file(file), line(line), column(column) {};
        // ~Node() { printf("Node destroyed: %s:%llu:%llu\n", file->c_str(), line, column); }
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
        Integer(std::shared_ptr<const std::string> &file, const line_t line, const column_t column, int64_t value)
            : Expression({ RULE_INTEGER, file, line, column }), value(value) {};
};

class Float : public Expression
{
    public:
        double value;
        Float(std::shared_ptr<const std::string> &file, const line_t line, const column_t column, double value)
            : Expression({ RULE_FLOAT, file, line, column }), value(value) {};
};

class String : public Expression
{
    public:
        std::string value;
        String(std::shared_ptr<const std::string> &file, const line_t line, const column_t column, const std::string &value)
            : Expression({ RULE_STRING, file, line, column }), value(value) {};
};

class Boolean : public Expression
{
    public:
        bool value;
        Boolean(std::shared_ptr<const std::string> &file, const line_t line, const column_t column, bool value)
            : Expression({ RULE_BOOLEAN, file, line, column }), value(value) {};
};

class List : public Expression
{
    public:
        std::vector<std::shared_ptr<Expression>> value;
        std::shared_ptr<Type> type; // Stores the list type since it's complex to analyze later.
        List(std::shared_ptr<const std::string> &file, const line_t line, const column_t column, const std::vector<std::shared_ptr<Expression>> &value)
            : Expression({ RULE_LIST, file, line, column }), value(value) {};
};

class Dictionary : public Expression
{
    public:
        std::unordered_map<std::string, std::shared_ptr<Expression>> value;
        std::vector<std::string> key_order;
        std::shared_ptr<Type> type; // Stores the dict type since it's complex to analyze later.
        Dictionary(std::shared_ptr<const std::string> &file, const line_t line, const column_t column, const std::unordered_map<std::string, std::shared_ptr<Expression>> &value, const std::vector<std::string> &key_order)
            : Expression({ RULE_DICTIONARY, file, line, column }), value(std::move(value)), key_order(std::move(key_order)) {};
};

class Group : public Expression
{
    public:
        std::shared_ptr<Expression> expression;
        Group(std::shared_ptr<const std::string> &file, const line_t line, const column_t column, const std::shared_ptr<Expression> &value)
            : Expression({ RULE_GROUP, file, line, column }), expression(std::move(value)) {};
};

class Unary : public Expression
{
    public:
        Token op;
        std::shared_ptr<Expression> right;
        UnaryType type = (UnaryType) NULL; // Determines what type of unary operation will be performed, no need to store a whole Type.
        Unary(std::shared_ptr<const std::string> &file, const line_t line, const column_t column, Token op, const std::shared_ptr<Expression> &right)
            : Expression({ RULE_UNARY, file, line, column }), op(op), right(std::move(right)) {};
};

class Binary : public Expression
{
    public:
        std::shared_ptr<Expression> left;
        Token op;
        std::shared_ptr<Expression> right;
        BinaryType type = (BinaryType) NULL; // Determines what type of binary operation will be performed.
        Binary(std::shared_ptr<const std::string> &file, const line_t line, const column_t column, const std::shared_ptr<Expression> &left, Token op, const std::shared_ptr<Expression> &right)
            : Expression({ RULE_BINARY, file, line, column }), left(std::move(left)), op(op), right(std::move(right)) {};
};

class Variable : public Expression
{
    public:
        std::string name;
        Variable(std::shared_ptr<const std::string> &file, const line_t line, const column_t column, const std::string &name)
            : Expression({ RULE_VARIABLE, file, line, column }), name(name) {};
};

class Assign : public Expression
{
    public:
        std::shared_ptr<Expression> target;
        std::shared_ptr<Expression> value;
        bool is_access;
        Assign(std::shared_ptr<const std::string> &file, const line_t line, const column_t column, const std::shared_ptr<Expression> &target, const std::shared_ptr<Expression> &value)
            : Expression({ RULE_ASSIGN, file, line, column }), target(std::move(target)), value(std::move(value)) {};
};

class Logical : public Expression
{
    public:
        std::shared_ptr<Expression> left;
        Token op;
        std::shared_ptr<Expression> right;
        Logical(std::shared_ptr<const std::string> &file, const line_t line, const column_t column, const std::shared_ptr<Expression> &left, Token op, const std::shared_ptr<Expression> &right)
            : Expression({ RULE_LOGICAL, file, line, column }), left(std::move(left)),  op(op), right(std::move(right)) {};
};

class Call : public Expression
{
    public:
        std::shared_ptr<Expression> target;
        std::vector<std::shared_ptr<Expression>> arguments;
        Call(std::shared_ptr<const std::string> &file, const line_t line, const column_t column, const std::shared_ptr<Expression> &target, const std::vector<std::shared_ptr<Expression>> &arguments)
            : Expression({ RULE_CALL, file, line, column }), target(std::move(target)), arguments(arguments) {};
};

class Access : public Expression
{
    public:
        std::shared_ptr<Expression> target;
        std::shared_ptr<Expression> index;
        AccessType type;
        Access(std::shared_ptr<const std::string> &file, const line_t line, const column_t column, const std::shared_ptr<Expression> &target, const std::shared_ptr<Expression> &index)
            : Expression({ RULE_ACCESS, file, line, column }), target(std::move(target)), index(std::move(index)) {};
};

class Cast : public Expression
{
    public:
        std::shared_ptr<Expression> expression;
        std::shared_ptr<Type> type;
        CastType cast_type = (CastType) NULL;
        Cast(std::shared_ptr<const std::string> &file, const line_t line, const column_t column, const std::shared_ptr<Expression> &expression, std::shared_ptr<Type> &type)
            : Expression({ RULE_CAST, file, line, column }), expression(std::move(expression)), type(std::move(type)) {}
};

/*
class Closure : public Expression
{
    public:
        std::vector<Statement *> parameters;
        std::string return_type;
        std::vector<Statement *> body;
        Block block;
        Closure(std::shared_ptr<const std::string> &file, const line_t line, const column_t column, std::vector<Statement *> parameters, std::string return_type, std::vector<Statement *> body)
            : Expression(RULE_CLOSURE, file, line, column), parameters(parameters), return_type(return_type), body(body) {}
};
*/

class Slice : public Expression
{
    public:
        std::shared_ptr<Expression> target;
        std::shared_ptr<Expression> start;
        std::shared_ptr<Expression> end;
        std::shared_ptr<Expression> step;
        bool is_list = (bool) NULL; // Determines if it's a list or a string, used by Analyzer.
        Slice(std::shared_ptr<const std::string> &file, const line_t line, const column_t column, const std::shared_ptr<Expression> &target, const std::shared_ptr<Expression> &start, const std::shared_ptr<Expression> &end, const std::shared_ptr<Expression> &step)
            : Expression({ RULE_SLICE, file, line, column }), target(std::move(target)), start(std::move(start)), end(std::move(end)), step(std::move(step)) {}
};

class Range : public Expression
{
    public:
        std::shared_ptr<Expression> start;
        std::shared_ptr<Expression> end;
        bool inclusive;
        Range(std::shared_ptr<const std::string> &file, const line_t line, const column_t column, const std::shared_ptr<Expression> &start, const std::shared_ptr<Expression> &end, bool inclusive)
            : Expression({ RULE_RANGE, file, line, column }), start(std::move(start)), end(std::move(end)), inclusive(inclusive) {}
};

class Delete : public Expression
{
    public:
        std::shared_ptr<Expression> target;
        Delete(std::shared_ptr<const std::string> &file, const line_t line, const column_t column, const std::shared_ptr<Expression> &target)
            : Expression({ RULE_DELETE, file, line, column }), target(std::move(target)) {}
};

class Length : public Expression
{
    public:
        std::shared_ptr<Expression> target;
        Length(std::shared_ptr<const std::string> &file, const line_t line, const column_t column, const std::shared_ptr<Expression> &target)
            : Expression({ RULE_LENGTH, file, line, column }), target(std::move(target)) {}
};

/* Statements */
class Print : public Statement
{
    public:
        std::shared_ptr<Expression> expression;
        Print(std::shared_ptr<const std::string> &file, const line_t line, const column_t column, const std::shared_ptr<Expression> &expression)
            : Statement({ RULE_PRINT, file, line, column }), expression(std::move(expression)) {}
};

class ExpressionStatement : public Statement
{
    public:
        std::shared_ptr<Expression> expression;
        ExpressionStatement(std::shared_ptr<const std::string> &file, const line_t line, const column_t column, const std::shared_ptr<Expression> &expression)
            : Statement({ RULE_EXPRESSION_STATEMENT, file, line, column }), expression(std::move(expression)) {}
};

class Declaration : public Statement
{
    public:
        std::string name;
        std::shared_ptr<Type> type;
        std::shared_ptr<Expression> initializer;
        Declaration(std::shared_ptr<const std::string> &file, const line_t line, const column_t column, const std::string &name, std::shared_ptr<Type> &type, const std::shared_ptr<Expression> &initializer)
            : Statement({ RULE_DECLARATION, file, line, column }), name(name), type(std::move(type)), initializer(std::move(initializer)) {};
};

class Return : public Statement
{
    public:
        std::shared_ptr<Expression> value;
        Return(std::shared_ptr<const std::string> &file, const line_t line, const column_t column, const std::shared_ptr<Expression> &value = std::shared_ptr<Expression>())
            : Statement({ RULE_RETURN, file, line, column }), value(std::move(value)) {}
};

class If : public Statement
{
    public:
        std::shared_ptr<Expression> condition;
        std::vector<std::shared_ptr<Statement>> then_branch;
        std::vector<std::shared_ptr<Statement>> else_branch;
        std::shared_ptr<Block> then_block, else_block;
        If(std::shared_ptr<const std::string> &file, const line_t line, const column_t column, const std::shared_ptr<Expression> &condition, const std::vector<std::shared_ptr<Statement>> &then_branch, const std::vector<std::shared_ptr<Statement>> &else_branch)
            : Statement({ RULE_IF, file, line, column }), condition(std::move(condition)), then_branch(then_branch), else_branch(else_branch) {};
};

class While : public Statement
{
    public:
        std::shared_ptr<Expression> condition;
        std::vector<std::shared_ptr<Statement>> body;
        std::shared_ptr<Block> block;
        While(std::shared_ptr<const std::string> &file, const line_t line, const column_t column, const std::shared_ptr<Expression> &condition, const std::vector<std::shared_ptr<Statement>> &body)
            : Statement({ RULE_WHILE, file, line, column }), condition(std::move(condition)), body(body) {};
};

class For : public Statement
{
    public:
        std::string variable;
        std::string index;
        std::shared_ptr<Expression> iterator;
        std::vector<std::shared_ptr<Statement>> body;
        std::shared_ptr<Block> block;
        std::shared_ptr<Type> type; // Stores the type of the iterator.
        For(std::shared_ptr<const std::string> &file, const line_t line, const column_t column, const std::string &variable, const std::string &index, const std::shared_ptr<Expression> &iterator, const std::vector<std::shared_ptr<Statement>> &body)
            : Statement({ RULE_FOR, file, line, column }), variable(variable), index(index), iterator(std::move(iterator)), body(body) {}
};

class Function : public Statement
{
    public:
        std::string name;
        std::vector<std::shared_ptr<Declaration>> parameters;
        std::shared_ptr<Type> return_type;
        std::vector<std::shared_ptr<Statement>> body;
        std::shared_ptr<Block> block;
        Function(std::shared_ptr<const std::string> &file, const line_t line, const column_t column, const std::string &name, const std::vector<std::shared_ptr<Declaration>> &parameters, std::shared_ptr<Type> &return_type, const std::vector<std::shared_ptr<Statement>> &body)
            : Statement({ RULE_FUNCTION, file, line, column }), name(name), parameters(parameters), return_type(std::move(return_type)), body(body) {}
};

class Use : public Statement
{
    public:
        std::vector<std::string> targets;
        std::shared_ptr<const std::string> module;
        std::shared_ptr<std::vector<std::shared_ptr<Statement>>> code;
        std::shared_ptr<Block> block;
        Use(std::shared_ptr<const std::string> &file, const line_t line, const column_t column, const std::vector<std::string> &targets, const std::shared_ptr<const std::string> &module)
            : Statement({ RULE_USE, file, line, column }), targets(targets), module(std::move(module)) {};
};

class Export : public Statement
{
    public:
        std::shared_ptr<Statement> statement;
        Export(std::shared_ptr<const std::string> &file, const line_t line, const column_t column, const std::shared_ptr<Statement> &statement)
            : Statement({ RULE_EXPORT, file, line, column }), statement(std::move(statement)) {}
};

class Class : public Statement
{
    public:
        std::string name;
        std::vector<std::shared_ptr<Statement>> body;
        Class(std::shared_ptr<const std::string> &file, const line_t line, const column_t column, const std::string &name, const std::vector<std::shared_ptr<Statement>> &body)
            : Statement({ RULE_CLASS, file, line, column }), name(name), body(body) {}
};

#endif
