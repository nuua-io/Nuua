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
    "RULE_CAST",
    "RULE_IMPORT"
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

static void print_spaces(uint16_t ammount)
{
    for (; ammount > 0; ammount--) printf("  ");
}

void Parser::debug_ast(Expression *expression, uint16_t spacer)
{
    print_spaces(spacer);
    switch (expression->rule) {
        case RULE_INTEGER: { printf("Integer"); break; }
        case RULE_FLOAT: { printf("Float"); break; }
        case RULE_BOOLEAN: { printf("Boolean"); break; }
        case RULE_STRING: { printf("String"); break; }
        case RULE_VARIABLE: { printf("Variable"); break; }
        case RULE_LIST: { printf("List"); break; }
        case RULE_DICTIONARY: { printf("Dictionary"); break; }
        case RULE_BINARY: {
            Binary *binary = static_cast<Binary *>(expression);
            printf("Binary[%s]\n", binary->op.to_string().c_str());
            Parser::debug_ast(binary->left, spacer + 1);
            printf("\n");
            Parser::debug_ast(binary->right, spacer + 1);
            break;
        }
        case RULE_UNARY: {
            Unary *unary = static_cast<Unary *>(expression);
            printf("Unary[%s]\n", unary->op.to_string().c_str());
            Parser::debug_ast(unary->right, spacer + 1);
            break;
        }
        case RULE_GROUP: {
            printf("Group\n");
            Parser::debug_ast(static_cast<Group *>(expression)->expression, spacer + 1);
            break;
        }
        case RULE_ASSIGN: {
            Assign *assign= static_cast<Assign *>(expression);
            printf("Assign\n");
            Parser::debug_ast(assign->target, spacer + 1);
            printf("\n");
            Parser::debug_ast(assign->value, spacer + 1);
            break;
        }
        case RULE_LOGICAL: {
            Logical *logical= static_cast<Logical *>(expression);
            printf("Logical[%s]\n", logical->op.to_string().c_str());
            Parser::debug_ast(logical->left, spacer + 1);
            printf("\n");
            Parser::debug_ast(logical->right, spacer + 1);
            break;
        }
        case RULE_CALL: {
            Call *call= static_cast<Call *>(expression);
            printf("Call\n");
            Parser::debug_ast(call->target, spacer + 1);
            break;
        }
        case RULE_ACCESS: {
            Access *access= static_cast<Access *>(expression);
            printf("Access\n");
            Parser::debug_ast(access->target, spacer + 1);
            printf("\n");
            Parser::debug_ast(access->index, spacer + 1);
            break;
        }
        case RULE_CAST: {
            Cast *cast = static_cast<Cast *>(expression);
            printf("Cast[%s]\n", cast->type.c_str());
            Parser::debug_ast(cast->expression, spacer + 1);
            break;
        }
        default: { break; }
    }
}

void Parser::debug_ast(Statement *statement, uint16_t spacer)
{
    print_spaces(spacer);
    switch (statement->rule) {
        case RULE_EXPRESSION_STATEMENT: {
            Parser::debug_ast(static_cast<ExpressionStatement *>(statement)->expression, spacer);
            break;
        }
        case RULE_PRINT: {
            printf("Print(\n");
            Parser::debug_ast(static_cast<Print *>(statement)->expression, spacer + 1);
            break;
        }
        case RULE_DECLARATION: {
            Declaration *dec = static_cast<Declaration *>(statement);
            printf("Declaration[%s: %s]\n", dec->name.c_str(), dec->type.c_str());
            if (dec->initializer) Parser::debug_ast(dec->initializer, spacer + 1);
            break;
        }
        case RULE_FUNCTION: {
            Function *function= static_cast<Function *>(statement);
            printf("Function[%s: ]\n", function->name.c_str(), function->return_type.c_str());
            Parser::debug_ast(function->body, spacer + 1);
            break;
        }
        case RULE_RETURN: {
            printf("Return\n");
            Parser::debug_ast(static_cast<Return *>(statement)->value, spacer + 1);
            break;
        }
        case RULE_IF: {
            If *ifs = static_cast<If *>(statement);
            printf("If\n");
            Parser::debug_ast(ifs->condition, spacer + 1);
            printf("\n");
            print_spaces(spacer + 1);
            printf("-----\n");
            Parser::debug_ast(ifs->then_branch, spacer + 1);
            if (ifs->else_branch.size() > 0) {
                printf("\n");
                print_spaces(spacer + 1);
                printf("-----\n");
                Parser::debug_ast(ifs->else_branch, spacer + 1);
            }
            break;
        }
        case RULE_WHILE: {
            While *whiles = static_cast<While *>(statement);
            printf("While\n");
            Parser::debug_ast(whiles->condition, spacer + 1);
            printf("\n");
            print_spaces(spacer + 1);
            printf("-----\n");
            Parser::debug_ast(whiles->body, spacer + 1);
            break;
        }
        case RULE_IMPORT: {
            Import *import = static_cast<Import *>(statement);
            printf("Import[%s from %s]", import->target.c_str(), import->module.c_str());
            break;
        }
        default: { break; }
    }
}

void Parser::debug_ast(std::vector<Statement *> &statements, uint16_t spacer)
{
    for (Statement *stmt : statements) Parser::debug_ast(stmt, spacer);
}
