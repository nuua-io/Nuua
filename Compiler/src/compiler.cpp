/**
 * |---------------|
 * | Nuua Compiler |
 * |---------------|
 *
 * Copyright 2019 Erik Campobadal <soc@erik.cat>
 * https://nuua.io
 */
#include "../include/compiler.hpp"
#include "../../Parser/include/parser.hpp"
#include "../../Logger/include/logger.hpp"

Memory *Compiler::get_current_memory()
{
    switch (this->current_memory) {
        case FUNCTIONS_MEMORY: { return &this->program.functions; }
        case CLASSES_MEMORY: { return &this->program.classes; }
        default: { return &this->program.program; }
    }
}

void Compiler::add_opcode(OpCode opcode)
{
    this->get_current_memory()->code.push_back(opcode);
    this->get_current_memory()->lines.push_back(this->current_line);
}

void Compiler::add_constant(Value *value)
{
    this->add_opcode(OP_CONSTANT);
    this->add_constant_only(value);
}

Program Compiler::compile(const char *source)
{
    auto parser = new Parser;
    auto structure = parser->parse(source);
    delete parser;

    logger->info("Started compiling...");

    for (auto node : structure) this->compile(node);
    this->add_opcode(OP_EXIT);

    logger->info("Program memory:");
    this->program.program.dump();
    logger->info("Functions memory:");
    this->program.functions.dump();
    logger->info("Classes memory:");
    this->program.classes.dump();

    logger->success("Compiling completed");

    return this->program;
}

void Compiler::compile(Statement *rule)
{
    this->current_line = rule->line;

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
            //! Still needs to finish what happens if the else branch is present!!
            auto rif = static_cast<If *>(rule);
            if (rif->elseBranch.size() == 0) {
                this->compile(rif->condition);

                this->add_opcode(OP_BRANCH_FALSE);
                auto constant_index = this->add_constant_only(new Value(0.0));
                auto start_index = this->current_code_line();

                for (auto stmt : rif->thenBranch) this->compile(stmt);

                this->modify_constant(constant_index, new Value(static_cast<double>(this->current_code_line() - start_index)));
            }
            break;
        }
        case RULE_WHILE: {
            auto rwhile = static_cast<While *>(rule);
            double initial_index = this->current_code_line();
            this->compile(rwhile->condition);

            this->add_opcode(OP_BRANCH_FALSE);
            auto constant_index = this->add_constant_only(new Value(0.0));
            auto start_index = this->current_code_line();

            for (auto stmt : rwhile->body) this->compile(stmt);

            this->add_opcode(OP_JUMP);
            this->add_constant_only(new Value(initial_index));

            this->modify_constant(constant_index, new Value(static_cast<double>(this->current_code_line() - start_index)));

            break;
        }
        default: {
            logger->error("Invalid statemetn to compile.", rule->line);
            exit(EXIT_FAILURE);
        }
    }
}

void Compiler::compile(Expression *rule)
{
    this->current_line = rule->line;

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
            this->compile(unary->op, true);
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
            auto logical = static_cast<Logical *>(rule);
            this->compile(logical->left);
            this->compile(logical->right);
            this->compile(logical->op);
            break;
        }
        case RULE_FUNCTION: {
            auto function = static_cast<Function *>(rule);
            auto memory = this->current_memory;

            this->current_memory = FUNCTIONS_MEMORY;

            double index = this->current_code_line();

            // Compile the function arguments
            for (auto argument : function->arguments) {
                switch (argument->rule) {
                    case RULE_VARIABLE: {
                        this->add_opcode(OP_STORE);
                        this->add_constant_only(new Value(static_cast<Variable *>(argument)->name));
                        break;
                    }
                    default: {
                        logger->error("Invalid argument when defining the function.", argument->line);
                        exit(EXIT_FAILURE);
                    }
                }
            }

            // Compile the function body
            for (auto stmt : function->body) this->compile(stmt);

            this->current_memory = memory;

            this->add_opcode(OP_FUNCTION);
            this->add_constant_only(new Value(index));

            break;
        }
        case RULE_CALL: {
            auto call = static_cast<Call *>(rule);
            for (auto argument : call->arguments) this->compile(argument);
            this->add_opcode(OP_CALL);
            this->add_constant_only(new Value(call->callee));
            this->add_constant_only(new Value(static_cast<double>(call->arguments.size())));
            break;
        }
        case RULE_ACCESS: {
            auto access = static_cast<Access *>(rule);
            this->compile(access->index);
            this->add_opcode(OP_ACCESS);
            this->add_constant_only(new Value(access->name));
            break;
        }
        default: {
            logger->error("Invalid expression to compile.", rule->line);
            exit(EXIT_FAILURE);
        }
    }
}

void Compiler::compile(Token op, bool unary)
{
    this->current_line = op.line;

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
            logger->error("Unknown operation token in a binary instruction", op.line);
            exit(EXIT_FAILURE);
        }
    }
}

uint64_t Compiler::add_constant_only(Value *value)
{
    this->get_current_memory()->constants.push_back(value);

    uint64_t index = this->get_current_memory()->constants.size() - 1;
    this->get_current_memory()->code.push_back(index);

    return index;
}

void Compiler::modify_constant(uint64_t index, Value *value)
{
    this->get_current_memory()->constants.at(index) = value;
}

uint32_t Compiler::current_code_line()
{
    return this->get_current_memory()->code.size();
}
