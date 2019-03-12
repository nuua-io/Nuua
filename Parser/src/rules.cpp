/**
 * |------------|
 * | Nuua Rules |
 * |------------|
 *
 * Copyright 2019 Erik Campobadal <soc@erik.cat>
 * https://nuua.io
 */
#include "../include/parser.hpp"

static std::vector<std::string> RuleNames = {
    "RULE_EXPRESSION",
    "RULE_STATEMENT",
    "RULE_PRINT",
    "RULE_EXPRESSION_STATEMENT",
    "RULE_INTEGER",
    "RULE_FLOAT",
    "RULE_STRING",
    "RULE_BOOLEAN",
    "RULE_LIST",
    "RULE_DICTIONARY",
    "RULE_NONE",
    "RULE_GROUP",
    "RULE_UNARY",
    "RULE_BINARY",
    "RULE_DECLARATION",
    "RULE_VARIABLE",
    "RULE_ASSIGN",
    "RULE_LOGICAL",
    "RULE_FUNCTION",
    "RULE_CALL",
    "RULE_ACCESS",
    "RULE_RETURN",
    "RULE_IF",
    "RULE_WHILE",
    "RULE_CAST"
};

void Parser::debug_rule(Rule rule)
{
    printf("%s\n", RuleNames[rule].c_str());
}

void Parser::debug_rule(Statement *statement)
{
    printf("%s\n", RuleNames[statement->rule].c_str());
}

void Parser::debug_rules(std::vector<Rule> &rules)
{
    for (Rule rule : rules) printf("%s\n", RuleNames[rule].c_str());
}

void Parser::debug_rules(std::vector<Statement *> &rules)
{
    for (Statement *stmt : rules) Parser::debug_rule(stmt);
}

void Parser::debug_ast(Expression *expression)
{
    switch (expression->rule) {
        case RULE_INTEGER: { printf("Integer(%lld)", static_cast<Integer *>(expression)->value); break; }
        case RULE_FLOAT: { printf("Float(%f)", static_cast<Float *>(expression)->value); break; }
        case RULE_BOOLEAN: { printf("Boolean(%s)", static_cast<Boolean *>(expression)->value ? "true" : "false"); break; }
        case RULE_STRING: { printf("String(%s)", static_cast<String *>(expression)->value); break; }
        case RULE_VARIABLE: { printf("Variable(%s)", static_cast<Variable *>(expression)->name.c_str()); break; }
        case RULE_BINARY: {
            Binary *binary = static_cast<Binary *>(expression);
            printf("Binary(");
            Parser::debug_ast(binary->left);
            printf(", %s, ", binary->op.to_string().c_str());
            Parser::debug_ast(binary->right);
            printf(")");
            break;
        }
        case RULE_UNARY: {
            Unary *unary = static_cast<Unary *>(expression);
            printf("Unary(%s, ", unary->op.to_string().c_str());
            Parser::debug_ast(unary->right);
            printf(")");
            break;
        }
        case RULE_GROUP: {
            printf("Group(");
            Parser::debug_ast(static_cast<Group *>(expression)->expression);
            printf(")");
            break;
        }
    }
}

void Parser::debug_ast(Statement *statement)
{
    switch (statement->rule) {
        case RULE_EXPRESSION_STATEMENT: {
            Parser::debug_ast(static_cast<ExpressionStatement *>(statement)->expression);
            break;
        }
        case RULE_PRINT: {
            printf("Print(");
            Parser::debug_ast(static_cast<Print *>(statement)->expression);
            printf(")");
            break;
        }
    }
}

void Parser::debug_ast(std::vector<Statement *> &statements)
{
    for (Statement *stmt : statements) Parser::debug_ast(stmt);
}
