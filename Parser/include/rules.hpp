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

#define NODE_PROPS std::shared_ptr<const std::string> &file, const line_t line, const column_t column

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
    RULE_FUNCTIONVALUE,
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
    RULE_OBJECT,
    RULE_PROPERTY
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
        Node(const Rule r, const std::shared_ptr<const std::string> &f, const line_t l, const column_t c)
            : rule(r), file(f), line(l), column(c) {};
        // ~Node() { printf("Node destroyed: %s:%llu:%llu\n", file->c_str(), line, column); }
};

class Expression : public Node
{
    public:
        explicit Expression(const Node &node)
            : Node(node) {};
};

class Statement : public Node
{
    public:
        explicit Statement(const Node &node)
            : Node(node) {};
};

/* Expressions */
class Integer : public Expression
{
    public:
        int64_t value;
        Integer(NODE_PROPS, const int64_t v)
            : Expression({ RULE_INTEGER, file, line, column }), value(v) {};
};

class Float : public Expression
{
    public:
        double value;
        Float(NODE_PROPS, const double v)
            : Expression({ RULE_FLOAT, file, line, column }), value(v) {};
};

class String : public Expression
{
    public:
        std::string value;
        String(NODE_PROPS, const std::string &v)
            : Expression({ RULE_STRING, file, line, column }), value(v) {};
};

class Boolean : public Expression
{
    public:
        bool value;
        Boolean(NODE_PROPS, const bool v)
            : Expression({ RULE_BOOLEAN, file, line, column }), value(v) {};
};

class List : public Expression
{
    public:
        std::vector<std::shared_ptr<Expression>> value;
        std::shared_ptr<Type> type; // Stores the list type since it's complex to analyze later.
        List(NODE_PROPS, const std::vector<std::shared_ptr<Expression>> &v)
            : Expression({ RULE_LIST, file, line, column }), value(v) {};
};

class Dictionary : public Expression
{
    public:
        std::unordered_map<std::string, std::shared_ptr<Expression>> value;
        std::vector<std::string> key_order;
        std::shared_ptr<Type> type; // Stores the dict type since it's complex to analyze later.
        Dictionary(NODE_PROPS, const std::unordered_map<std::string, std::shared_ptr<Expression>> &v, const std::vector<std::string> &ko)
            : Expression({ RULE_DICTIONARY, file, line, column }), value(std::move(v)), key_order(std::move(ko)) {};
};

class Group : public Expression
{
    public:
        std::shared_ptr<Expression> expression;
        Group(NODE_PROPS, const std::shared_ptr<Expression> &v)
            : Expression({ RULE_GROUP, file, line, column }), expression(std::move(v)) {};
};

class Unary : public Expression
{
    public:
        Token op;
        std::shared_ptr<Expression> right;
        UnaryType type = (UnaryType) NULL; // Determines what type of unary operation will be performed, no need to store a whole Type.
        Unary(NODE_PROPS, const Token &o, const std::shared_ptr<Expression> &r)
            : Expression({ RULE_UNARY, file, line, column }), op(o), right(std::move(r)) {};
};

class Binary : public Expression
{
    public:
        std::shared_ptr<Expression> left;
        Token op;
        std::shared_ptr<Expression> right;
        BinaryType type = (BinaryType) NULL; // Determines what type of binary operation will be performed.
        Binary(NODE_PROPS, const std::shared_ptr<Expression> &l, const Token &o, const std::shared_ptr<Expression> &r)
            : Expression({ RULE_BINARY, file, line, column }), left(std::move(l)), op(o), right(std::move(r)) {};
};

class Variable : public Expression
{
    public:
        std::string name;
        Variable(NODE_PROPS, const std::string &n)
            : Expression({ RULE_VARIABLE, file, line, column }), name(n) {};
};

class Assign : public Expression
{
    public:
        std::shared_ptr<Expression> target;
        std::shared_ptr<Expression> value;
        bool is_access = false;
        Assign(NODE_PROPS, const std::shared_ptr<Expression> &t, const std::shared_ptr<Expression> &v)
            : Expression({ RULE_ASSIGN, file, line, column }), target(std::move(t)), value(std::move(v)) {};
};

class Logical : public Expression
{
    public:
        std::shared_ptr<Expression> left;
        Token op;
        std::shared_ptr<Expression> right;
        Logical(NODE_PROPS, const std::shared_ptr<Expression> &l, const Token &o, const std::shared_ptr<Expression> &r)
            : Expression({ RULE_LOGICAL, file, line, column }), left(std::move(l)),  op(o), right(std::move(r)) {};
};

class Call : public Expression
{
    public:
        std::shared_ptr<Expression> target;
        std::vector<std::shared_ptr<Expression>> arguments;
        bool has_return = false; // Determines if the call target returns a value or not.
        Call(NODE_PROPS, const std::shared_ptr<Expression> &t, const std::vector<std::shared_ptr<Expression>> &a)
            : Expression({ RULE_CALL, file, line, column }), target(std::move(t)), arguments(a) {};
};

class Access : public Expression
{
    public:
        std::shared_ptr<Expression> target;
        std::shared_ptr<Expression> index;
        AccessType type = (AccessType) NULL;
        Access(NODE_PROPS, const std::shared_ptr<Expression> &t, const std::shared_ptr<Expression> &i)
            : Expression({ RULE_ACCESS, file, line, column }), target(std::move(t)), index(std::move(i)) {};
};

class Cast : public Expression
{
    public:
        std::shared_ptr<Expression> expression;
        std::shared_ptr<Type> type;
        CastType cast_type = (CastType) NULL;
        Cast(NODE_PROPS, const std::shared_ptr<Expression> &e, std::shared_ptr<Type> &t)
            : Expression({ RULE_CAST, file, line, column }), expression(std::move(e)), type(std::move(t)) {}
};

/*
class Closure : public Expression
{
    public:
        std::vector<Statement *> parameters;
        std::string return_type;
        std::vector<Statement *> body;
        Block block;
        Closure(NODE_PROPS, std::vector<Statement *> parameters, std::string return_type, std::vector<Statement *> body)
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
        bool is_list = false; // Determines if it's a list or a string, used by Analyzer.
        Slice(NODE_PROPS, const std::shared_ptr<Expression> &t, const std::shared_ptr<Expression> &s, const std::shared_ptr<Expression> &e, const std::shared_ptr<Expression> &st)
            : Expression({ RULE_SLICE, file, line, column }), target(std::move(t)), start(std::move(s)), end(std::move(e)), step(std::move(st)) {}
};

class Range : public Expression
{
    public:
        std::shared_ptr<Expression> start;
        std::shared_ptr<Expression> end;
        bool inclusive;
        Range(NODE_PROPS, const std::shared_ptr<Expression> &s, const std::shared_ptr<Expression> &e, const bool i)
            : Expression({ RULE_RANGE, file, line, column }), start(std::move(s)), end(std::move(e)), inclusive(i) {}
};

class Delete : public Expression
{
    public:
        std::shared_ptr<Expression> target;
        Delete(NODE_PROPS, const std::shared_ptr<Expression> &t)
            : Expression({ RULE_DELETE, file, line, column }), target(std::move(t)) {}
};

// Forward declare
class Declaration;

class FunctionValue : public Expression
{
    public:
        std::string name;
        std::vector<std::shared_ptr<Declaration>> parameters;
        std::shared_ptr<Type> return_type;
        std::vector<std::shared_ptr<Statement>> body;
        std::shared_ptr<Block> block;
        FunctionValue(NODE_PROPS, const std::string &n, const std::vector<std::shared_ptr<Declaration>> &p, std::shared_ptr<Type> &rt, const std::vector<std::shared_ptr<Statement>> &b)
            : Expression({ RULE_FUNCTION, file, line, column }), name(n), parameters(p), return_type(std::move(rt)), body(b) {}
};

class Object : public Expression
{
    public:
        std::string name;
        std::unordered_map<std::string, std::shared_ptr<Expression>> arguments;
        Object(NODE_PROPS, const std::string &n, const std::unordered_map<std::string, std::shared_ptr<Expression>> &a)
            : Expression({ RULE_OBJECT, file, line, column }), name(n), arguments(a) {}
};

class Property : public Expression
{
    public:
        std::shared_ptr<Expression> object;
        std::string name;
        Property(NODE_PROPS, const std::shared_ptr<Expression> &o, const std::string &n)
            : Expression({ RULE_PROPERTY, file, line, column }), object(o), name(n) {}
};

/* Statements */
class Print : public Statement
{
    public:
        std::shared_ptr<Expression> expression;
        Print(NODE_PROPS, const std::shared_ptr<Expression> &e)
            : Statement({ RULE_PRINT, file, line, column }), expression(std::move(e)) {}
};

class ExpressionStatement : public Statement
{
    public:
        std::shared_ptr<Expression> expression;
        ExpressionStatement(NODE_PROPS, const std::shared_ptr<Expression> &e)
            : Statement({ RULE_EXPRESSION_STATEMENT, file, line, column }), expression(std::move(e)) {}
};

class Declaration : public Statement
{
    public:
        std::string name;
        std::shared_ptr<Type> type;
        std::shared_ptr<Expression> initializer;
        Declaration(NODE_PROPS, const std::string &n, std::shared_ptr<Type> &t, const std::shared_ptr<Expression> &i)
            : Statement({ RULE_DECLARATION, file, line, column }), name(n), type(std::move(t)), initializer(std::move(i)) {};
};

class Return : public Statement
{
    public:
        std::shared_ptr<Expression> value;
        Return(NODE_PROPS, const std::shared_ptr<Expression> &v = std::shared_ptr<Expression>())
            : Statement({ RULE_RETURN, file, line, column }), value(std::move(v)) {}
};

class If : public Statement
{
    public:
        std::shared_ptr<Expression> condition;
        std::vector<std::shared_ptr<Statement>> then_branch;
        std::vector<std::shared_ptr<Statement>> else_branch;
        std::shared_ptr<Block> then_block, else_block;
        If(NODE_PROPS, const std::shared_ptr<Expression> &c, const std::vector<std::shared_ptr<Statement>> &tb, const std::vector<std::shared_ptr<Statement>> &eb)
            : Statement({ RULE_IF, file, line, column }), condition(std::move(c)), then_branch(tb), else_branch(eb) {};
};

class While : public Statement
{
    public:
        std::shared_ptr<Expression> condition;
        std::vector<std::shared_ptr<Statement>> body;
        std::shared_ptr<Block> block;
        While(NODE_PROPS, const std::shared_ptr<Expression> &c, const std::vector<std::shared_ptr<Statement>> &b)
            : Statement({ RULE_WHILE, file, line, column }), condition(std::move(c)), body(b) {};
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
        For(NODE_PROPS, const std::string &v, const std::string &i, const std::shared_ptr<Expression> &it, const std::vector<std::shared_ptr<Statement>> &b)
            : Statement({ RULE_FOR, file, line, column }), variable(v), index(i), iterator(std::move(it)), body(b) {}
};

class Function : public Statement
{
    public:
        std::shared_ptr<FunctionValue> value;
        Function(const std::shared_ptr<FunctionValue> &v)
            : Statement({ RULE_FUNCTION, v->file, v->line, v->column }), value(v) {}
};

class Use : public Statement
{
    public:
        std::vector<std::string> targets;
        std::shared_ptr<const std::string> module;
        std::shared_ptr<std::vector<std::shared_ptr<Statement>>> code;
        std::shared_ptr<Block> block;
        Use(NODE_PROPS, const std::vector<std::string> &t, const std::shared_ptr<const std::string> &m)
            : Statement({ RULE_USE, file, line, column }), targets(t), module(std::move(m)) {};
};

class Export : public Statement
{
    public:
        std::shared_ptr<Statement> statement;
        Export(NODE_PROPS, const std::shared_ptr<Statement> &s)
            : Statement({ RULE_EXPORT, file, line, column }), statement(std::move(s)) {}
};

class Class : public Statement
{
    public:
        std::string name;
        std::vector<std::shared_ptr<Statement>> body;
        std::shared_ptr<Block> block;
        Class(NODE_PROPS, const std::string &n, const std::vector<std::shared_ptr<Statement>> &b)
            : Statement({ RULE_CLASS, file, line, column }), name(n), body(b) {}
};

#endif
