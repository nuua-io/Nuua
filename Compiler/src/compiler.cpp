#include "../include/compiler.hpp"
#include "../../Parser/include/parser.hpp"
#include "../../Logger/include/logger.hpp"

void Compiler::add_opcode(OpCode opcode)
{
    switch (this->current_memory) {
        case PROGRAM_MEMORY: {
            this->program.program.code.push_back(opcode);
            this->program.program.lines.push_back(this->current_line);
            break;
        }
        case FUNCTIONS_MEMORY: {
            this->program.functions.code.push_back(opcode);
            this->program.functions.lines.push_back(this->current_line);
            break;
        }
        case CLASSES_MEMORY: {
            this->program.classes.code.push_back(opcode);
            this->program.classes.lines.push_back(this->current_line);
            break;
        }
    }
}

Program Compiler::compile(const char *source)
{
    auto parser = new Parser;
    auto structure = parser->parse(source);
    delete parser;

    logger->info("Started compiling...");

    for (auto node : structure) this->compile(node);
    this->add_opcode(OP_EXIT);

    logger->success("Compiling completed");

    return this->program;
}

void Compiler::compile(Statement *rule)
{
    switch (rule->rule) {
        case RULE_EXPRESSION:
        case RULE_STATEMENT: {
            logger->error("Invalid rule to compile. Found Statement or expression without propper format.");
            exit(EXIT_FAILURE);
            break; // I won't remove this line to keep it understandable
        }
        case RULE_EXPRESSION_STATEMENT: {
            this->compile(static_cast<ExpressionStatement *>(rule)->expression);
            break;
        }
        case RULE_IF: {
            break;
        }
        case RULE_WHILE: {
            break;
        }
        default: {
            logger->error("Invalid statemetn to compile.");
            exit(EXIT_FAILURE);
        }
    }
}

void Compiler::compile(Expression *rule)
{
    switch (rule->rule) {
        case RULE_NUMBER: {
            this->add_constant(new Value(static_cast<Number *>(rule)->value));
            break;
        }
        case RULE_STRING: {
            this->add_constant(new Value(static_cast<String *>(rule)->value));
            break;
        }
        case RULE_BOOLEAN: {
            this->add_constant(new Value(static_cast<Boolean *>(rule)->value));
            break;
        }
        case RULE_LIST: {
            auto list = static_cast<List *>(rule);
            for (int i = list->value.size() - 1; i >= 0; i--) this->compile(list->value.at(i));
            this->add_constant(new Value(static_cast<double>(list->value.size())));
            break;
        }
        case RULE_DICTIONARY: {
            auto dictionary = static_cast<Dictionary *>(rule);
            for (int i = dictionary->key_order.size() - 1; i >= 0; i--) {
                this->add_constant(new Value(dictionary->key_order[i]));
                this->compile(dictionary->value.at(dictionary->key_order[i]));
            }
            this->add_constant(new Value(static_cast<double>(dictionary->value.size())));
            break;
        }
        case RULE_NONE: {
            this->add_constant(new Value());
            break;
        }
        case RULE_GROUP: {
            this->compile(static_cast<Group *>(rule)->expression);
            break;
        }
        case RULE_UNARY: {
            auto unary = static_cast<Unary *>(rule);
            this->compile(unary->right);
            this->compile(unary->op);
            break;
        }
        case RULE_BINARY: {
            auto binary = static_cast<Binary *>(rule);
            this->compile(binary->left);
            this->compile(binary->right);
            this->compile(binary->op);
            break;
        }
        case RULE_VARIABLE: {
            this->add_opcode(OP_LOAD);
            this->add_constant_only(new Value(static_cast<Variable*>(rule)->name));
            break;
        }
        case RULE_ASSIGN: {
            auto assign = static_cast<Assign *>(rule);
            this->compile(assign->value);
            this->add_opcode(OP_STORE);
            this->add_constant_only(new Value(assign->name));
            break;
        }
        case RULE_ASSIGN_ACCESS: {
            auto assign_access = static_cast<AssignAccess *>(rule);
            this->compile(assign_access->value);
            this->compile(assign_access->index);
            this->add_opcode(OP_STORE_ACCESS);
            this->add_constant_only(new Value(assign_access->name));
            break;
        }
        case RULE_LOGICAL: {
            break;
        }
        case RULE_FUNCTION: {
            break;
        }
        case RULE_CALL: {
            break;
        }
        case RULE_ACCESS: {
            break;
        }
        default: {
            logger->error("Invalid expression to compile.");
            exit(EXIT_FAILURE);
        }
    }
}

void Compiler::compile(Token op, bool unary)
{
    switch (op.type) {
        case TOKEN_PLUS: { this->add_opcode(OP_ADD); break; }
        case TOKEN_MINUS: { unary ? this->add_opcode(OP_MINUS) : this->add_opcode(OP_SUB); break; }
        case TOKEN_STAR: { this->add_opcode(OP_MUL); break; }
        case TOKEN_SLASH: { this->add_opcode(OP_DIV); break; }
        case TOKEN_BANG: { this->add_opcode(OP_NOT); break; }
        case TOKEN_EQUAL: { this->add_opcode(OP_STORE); break; }
        case TOKEN_EQUAL_EQUAL: { this->add_opcode(OP_EQ); break; }
        case TOKEN_BANG_EQUAL: { this->add_opcode(OP_NEQ); break; }
        case TOKEN_LOWER: { this->add_opcode(OP_LT); break; }
        case TOKEN_LOWER_EQUAL: { this->add_opcode(OP_LTE); break; }
        case TOKEN_HIGHER: { this->add_opcode(OP_HT); break; }
        case TOKEN_HIGHER_EQUAL: { this->add_opcode(OP_HTE); break; }
        default: {
            logger->error("Unknown operation token in binary instruction");
            exit(EXIT_FAILURE);
        }
    }
}
