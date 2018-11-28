/**
 * |------------|
 * | Nuua Rules |
 * |------------|
 *
 * Copyright 2018 Erik Campobadal <soc@erik.cat>
 * https://nuua.io
 */
#include "../include/parser.hpp"

static std::vector<std::string> RuleNames = {
    "RULE_EXPRESSION",
    "RULE_STATEMENT",
    "RULE_EXPRESSION_STATEMENT",
    "RULE_NUMBER",
    "RULE_STRING",
    "RULE_BOOLEAN",
    "RULE_LIST",
    "RULE_DICTIONARY",
    "RULE_NONE",
    "RULE_GROUP",
    "RULE_UNARY",
    "RULE_BINARY",
    "RULE_VARIABLE",
    "RULE_ASSIGN",
    "RULE_ASSIGN_ACCESS",
    "RULE_LOGICAL",
    "RULE_FUNCTION",
    "RULE_CALL",
    "RULE_ACCESS",
    "RULE_IF",
    "RULE_WHILE",
    "RULE_UNROLL",
};

void Parser::debug_rules(std::vector<Rule> rules)
{
    for (auto rule : rules) printf("%s\n", RuleNames[rule].c_str());
}

void Parser::debug_rules(std::vector<Statement *> rules)
{
    for (auto stmt : rules) printf("%s\n", RuleNames[stmt->rule].c_str());
}
