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
    "RULE_UNSIGNED",
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
    "RULE_FUNCTIONVALUE",
    "RULE_CALL",
    "RULE_ACCESS",
    "RULE_RETURN",
    "RULE_IF",
    "RULE_WHILE",
    "RULE_FOR",
    "RULE_CAST",
    "RULE_IMPORT",
    "RULE_EXPORT",
    "RULE_CLASS",
    "RULE_SLICE",
    "RULE_RANGE",
    "RULE_DELETE",
    "RULE_OBJECT",
    "RULE_PROPERTY"
};

void Parser::debug_rule(const Rule rule)
{
    printf("%s\n", RuleNames[rule].c_str());
}

void Parser::debug_rule(const std::shared_ptr<Statement> &statement)
{
    printf("%s\n", RuleNames[statement->rule].c_str());
}

void Parser::debug_rules(const std::vector<Rule> &rules)
{
    for (const Rule rule : rules) printf("%s\n", RuleNames[rule].c_str());
}

void Parser::debug_rules(const std::vector<std::shared_ptr<Statement>> &rules)
{
    for (const std::shared_ptr<Statement> &stmt : rules) Parser::debug_rule(stmt);
}

static void print_spaces(uint16_t ammount)
{
    for (; ammount > 0; ammount--) printf("  ");
}

void Parser::debug_ast(const std::shared_ptr<Expression> &expression, const uint16_t spacer, const bool print_spacer)
{
    if (print_spacer) print_spaces(spacer);
    if (!expression) { printf("[No expression provided]\n"); return; };
    switch (expression->rule) {
        case RULE_INTEGER: { printf("Integer\n"); break; }
        case RULE_UNSIGNED: { printf("Unsigned\n"); break; }
        case RULE_FLOAT: { printf("Float\n"); break; }
        case RULE_BOOLEAN: { printf("Boolean\n"); break; }
        case RULE_STRING: { printf("String\n"); break; }
        case RULE_VARIABLE: { printf("Variable\n"); break; }
        case RULE_LIST: { printf("List\n"); break; }
        case RULE_DICTIONARY: { printf("Dictionary\n"); break; }
        /*
        case RULE_CLOSURE: {
            Closure *closure = static_cast<Closure *>(expression);
            printf("Closure[%s]\n", closure->return_type.c_str());
            Parser::debug_ast(closure->body, spacer + 1);
            break;
        }
        */
        case RULE_BINARY: {
            std::shared_ptr<Binary> binary = std::static_pointer_cast<Binary>(expression);
            printf("Binary[%s]\n", binary->op.to_type_string().c_str());
            Parser::debug_ast(binary->left, spacer + 1);
            Parser::debug_ast(binary->right, spacer + 1);
            break;
        }
        case RULE_UNARY: {
            std::shared_ptr<Unary> unary = std::static_pointer_cast<Unary>(expression);
            printf("Unary[%s]\n", unary->op.to_type_string().c_str());
            Parser::debug_ast(unary->right, spacer + 1);
            break;
        }
        case RULE_GROUP: {
            printf("Group\n");
            Parser::debug_ast(std::static_pointer_cast<Group>(expression)->expression, spacer + 1);
            break;
        }
        case RULE_ASSIGN: {
            std::shared_ptr<Assign> assign = std::static_pointer_cast<Assign>(expression);
            printf("Assign\n");
            Parser::debug_ast(assign->target, spacer + 1);
            Parser::debug_ast(assign->value, spacer + 1);
            break;
        }
        case RULE_LOGICAL: {
            std::shared_ptr<Logical> logical = std::static_pointer_cast<Logical>(expression);
            printf("Logical[%s]\n", logical->op.to_string().c_str());
            Parser::debug_ast(logical->left, spacer + 1);
            Parser::debug_ast(logical->right, spacer + 1);
            break;
        }
        case RULE_CALL: {
            printf("Call\n");
            Parser::debug_ast(std::static_pointer_cast<Call>(expression)->target, spacer + 1);
            break;
        }
        case RULE_ACCESS: {
            std::shared_ptr<Access> access = std::static_pointer_cast<Access>(expression);
            printf("Access\n");
            Parser::debug_ast(access->target, spacer + 1);
            Parser::debug_ast(access->index, spacer + 1);
            break;
        }
        case RULE_CAST: {
            std::shared_ptr<Cast> cast = std::static_pointer_cast<Cast>(expression);
            printf("Cast[%s]\n", cast->type->to_string().c_str());
            Parser::debug_ast(cast->expression, spacer + 1);
            break;
        }
        case RULE_SLICE: {
            std::shared_ptr<Slice> slice = std::static_pointer_cast<Slice>(expression);
            printf("Slice\n");
            Parser::debug_ast(slice->target, spacer + 1);
            Parser::debug_ast(slice->start, spacer + 1);
            Parser::debug_ast(slice->end, spacer + 1);
            Parser::debug_ast(slice->step, spacer + 1);
            break;
        }
        case RULE_RANGE: {
            std::shared_ptr<Range> range = std::static_pointer_cast<Range>(expression);
            printf("Range\n");
            Parser::debug_ast(range->start, spacer + 1);
            Parser::debug_ast(range->end, spacer + 1);
            break;
        }
        case RULE_OBJECT: {
            std::shared_ptr<Object> object = std::static_pointer_cast<Object>(expression);
            printf("Object[%s]\n", object->name.c_str());
            for (const auto &[key, value] : object->arguments) {
                print_spaces(spacer + 1);
                printf("%s:\n", key.c_str());
                Parser::debug_ast(value, spacer + 2);
            }
            break;
        }
        case RULE_PROPERTY: {
            std::shared_ptr<Property> prop = std::static_pointer_cast<Property>(expression);
            printf("Property[%s]\n", prop->name.c_str());
            Parser::debug_ast(prop->object, spacer + 1);
            break;
        }
        default: { break; }
    }
}

void Parser::debug_ast(const std::shared_ptr<Statement> &statement, const uint16_t spacer)
{
    print_spaces(spacer);
    if (!statement) { printf("-\n"); return; };
    switch (statement->rule) {
        case RULE_EXPRESSION_STATEMENT: {
            Parser::debug_ast(std::static_pointer_cast<ExpressionStatement>(statement)->expression, spacer, false);
            break;
        }
        case RULE_PRINT: {
            printf("Print\n");
            Parser::debug_ast(std::static_pointer_cast<Print>(statement)->expression, spacer + 1);
            break;
        }
        case RULE_DECLARATION: {
            std::shared_ptr<Declaration> dec = std::static_pointer_cast<Declaration>(statement);
            printf("Declaration[%s: %s]\n", dec->name.c_str(), dec->type ? dec->type->to_string().c_str() : "<from-initializer>");
            Parser::debug_ast(dec->initializer, spacer + 1);
            break;
        }
        case RULE_FUNCTION: {
            std::shared_ptr<FunctionValue> function = std::static_pointer_cast<Function>(statement)->value;
            printf("Function[%s: %s]\n", function->name.c_str(), function->return_type ? function->return_type->to_string().c_str() : "<no-return>");
            Parser::debug_ast(function->body, spacer + 1);
            break;
        }
        case RULE_RETURN: {
            printf("Return\n");
            Parser::debug_ast(std::static_pointer_cast<Return>(statement)->value, spacer + 1);
            break;
        }
        case RULE_IF: {
            std::shared_ptr<If> ifs = std::static_pointer_cast<If>(statement);
            printf("If\n");
            print_spaces(spacer + 1);
            printf("[Condition]\n");
            Parser::debug_ast(ifs->condition, spacer + 2);
            print_spaces(spacer + 1);
            printf("[Then branch]\n");
            Parser::debug_ast(ifs->then_branch, spacer + 2);
            if (ifs->else_branch.size() > 0) {
                print_spaces(spacer + 1);
                printf("[Else branch]\n");
                Parser::debug_ast(ifs->else_branch, spacer + 2);
            }
            break;
        }
        case RULE_WHILE: {
            std::shared_ptr<While> whiles = std::static_pointer_cast<While>(statement);
            printf("While\n");
            print_spaces(spacer + 1);
            printf("[Condition]\n");
            Parser::debug_ast(whiles->condition, spacer + 2);
            print_spaces(spacer + 1);
            printf("[Body]\n");
            Parser::debug_ast(whiles->body, spacer + 2);
            break;
        }
        case RULE_FOR: {
            std::shared_ptr<For> fors = std::static_pointer_cast<For>(statement);
            printf("For[%s, %s]\n", fors->variable.c_str(), fors->index.c_str());
            print_spaces(spacer + 1);
            printf("[Iterator]\n");
            Parser::debug_ast(fors->iterator, spacer + 2);
            print_spaces(spacer + 1);
            printf("[Body]\n");
            Parser::debug_ast(fors->body, spacer + 2);
            break;
        }
        case RULE_USE: {
            std::shared_ptr<Use> use = std::static_pointer_cast<Use>(statement);
            printf("Use[%s]\n", use->module->c_str());
            print_spaces(spacer + 1);
            printf("[Targets]\n");
            for (std::string &identifier : use->targets) {
                print_spaces(spacer + 2);
                printf("%s\n", identifier.c_str());
            }
            print_spaces(spacer + 1);
            printf("[Code]\n");
            Parser::debug_ast(*use->code, spacer + 2);
            break;
        }
        case RULE_EXPORT: {
            printf("Export\n");
            Parser::debug_ast(std::static_pointer_cast<Export>(statement)->statement, spacer + 1);
            break;
        }
        case RULE_CLASS: {
            std::shared_ptr<Class> c = std::static_pointer_cast<Class>(statement);
            printf("Class[%s]\n", c->name.c_str());
            Parser::debug_ast(c->body, spacer + 1);
            break;
        }
        default: { break; }
    }
}

void Parser::debug_ast(const std::vector<std::shared_ptr<Statement>> &statements, const uint16_t spacer)
{
    for (const std::shared_ptr<Statement> &stmt : statements) Parser::debug_ast(stmt, spacer);
}
