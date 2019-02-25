/**
 * |---------------|
 * | Nuua Compiler |
 * |---------------|
 *
 * Copyright 2019 Erik Campobadal <soc@erik.cat>
 * https://nuua.io
 */
#include "../include/compiler.hpp"
#include "../../Logger/include/logger.hpp"

Memory *Compiler::get_current_memory()
{
    switch (this->current_memory) {
        case FUNCTIONS_MEMORY: { return &this->program.functions; }
        case CLASSES_MEMORY: { return &this->program.classes; }
        default: { return &this->program.program; }
    }
}

void Compiler::add_opcode(uint64_t opcode)
{
    this->get_current_memory()->code.push_back(opcode);
    this->get_current_memory()->lines.push_back(this->current_line);
}

Program Compiler::compile(const char *source)
{
    auto analyzer = Analyzer();
    analyzer.analyze(source);
    analyzer.optimize();
    auto structure = &analyzer.code;

    logger->info("Started compiling...");

    // Compile the code.
    this->blocks.push_back(&analyzer.main_block);
    for (auto node : *structure) this->compile(node);
    this->blocks.pop_back();

    // Add the exit opcode.
    this->add_opcode(OP_EXIT);

    #if DEBUG
        logger->info("Program memory:");
        this->program.program.dump();
        logger->info("Functions memory:");
        this->program.functions.dump();
        logger->info("Classes memory:");
        this->program.classes.dump();
    #endif

    // Store the ammount of needed registers for the main frame.
    this->program.main_registers = this->frame_info.front().current_register;

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
        case RULE_PRINT: {
            auto print = static_cast<Print *>(rule);
            auto reg = this->compile(print->expression);
            this->add_opcode(OP_PRINT);
            this->add_opcode(reg);
            break;
        }
        case RULE_EXPRESSION_STATEMENT: {
            auto expr = static_cast<ExpressionStatement *>(rule);
            // Compile the expressions. r is the resulting register.
            auto r = this->compile(expr->expression);
            // Free the regiter for future use.
            this->frame_info.back().free_register(r);
            break;
        }
        case RULE_DECLARATION : {
            auto declaration = static_cast<Declaration *>(rule);
            auto reg = this->frame_info.back().get_register(true);
            this->blocks.back()->variables[declaration->name].reg = reg;
            if (declaration->initializer) {
                auto res = this->compile(declaration->initializer);
                this->add_opcode(OP_MOVE);
                this->add_opcode(reg);
                this->add_opcode(res);
                this->frame_info.back().free_register(res);
            } else {
                this->add_opcode(OP_LOAD);
                this->add_opcode(reg);
                this->add_constant_only(Type(declaration->type));
            }
            break;
        }
        case RULE_RETURN: {
            auto ret = static_cast<Return *>(rule);
            break;
        }
        case RULE_IF: {
            //! Still needs to finish what happens if the else branch is present!!
            auto rif = static_cast<If *>(rule);
            break;
        }
        case RULE_WHILE: {
            auto rwhile = static_cast<While *>(rule);
            break;
        }
        default: {
            logger->error("Invalid statement to compile.", rule->line);
            exit(EXIT_FAILURE);
        }
    }
    delete rule;
}

uint32_t Compiler::compile(Expression *rule)
{
    this->current_line = rule->line;
    uint32_t result = 0;

    switch (rule->rule) {
        case RULE_INTEGER: {
            auto integer = static_cast<Integer *>(rule);
            result = this->frame_info.back().get_register();
            this->add_opcode(OP_LOAD);
            this->add_opcode(result);
            this->add_constant_only(integer->value);
            break;
        }
        case RULE_FLOAT: {
            auto floating = static_cast<Float *>(rule);
            result = this->frame_info.back().get_register();
            this->add_opcode(OP_LOAD);
            this->add_opcode(result);
            this->add_constant_only(floating->value);
            break;
        }
        case RULE_STRING: {
            auto string = static_cast<String *>(rule);
            result = this->frame_info.back().get_register();
            this->add_opcode(OP_LOAD);
            this->add_opcode(result);
            this->add_constant_only(string->value);
            break;
        }
        case RULE_BOOLEAN: {
            auto boolean = static_cast<Boolean *>(rule);
            result = this->frame_info.back().get_register();
            this->add_opcode(OP_LOAD);
            this->add_opcode(result);
            this->add_constant_only(boolean->value);
            break;
        }
        case RULE_LIST: {
            auto list = static_cast<List *>(rule);
            break;
        }
        case RULE_DICTIONARY: {
            auto dictionary = static_cast<Dictionary *>(rule);
            break;
        }
        case RULE_NONE: {
            this->add_opcode(OP_LOAD);
            this->add_opcode(result = this->frame_info.back().get_register());
            this->add_constant_only(Value());
            break;
        }
        case RULE_GROUP: {
            auto group = static_cast<Group *>(rule);
            result = this->compile(group->expression);
            break;
        }
        case RULE_CAST: {
            auto cast = static_cast<Cast *>(rule);
            break;
        }
        case RULE_UNARY: {
            auto unary = static_cast<Unary *>(rule);
            break;
        }
        case RULE_BINARY: {
            auto binary = static_cast<Binary *>(rule);
            auto r1 = this->compile(binary->left);
            auto r2 = this->compile(binary->right);
            result = this->frame_info.back().get_register();
            switch (binary->op.type) {
                case TOKEN_PLUS: {
                    this->add_opcode(OP_ADD);
                    this->add_opcode(result);
                    this->add_opcode(r1);
                    this->add_opcode(r2);
                    // Release registers.
                    this->frame_info.back().free_register(r1);
                    this->frame_info.back().free_register(r2);
                    break;
                }
                default: { }
            }
            break;
        }
        case RULE_VARIABLE: {
            auto variable = static_cast<Variable *>(rule);
            result = this->blocks.back()->variables[variable->name].reg;
            break;
        }
        case RULE_ASSIGN: {
            auto assign = static_cast<Assign *>(rule);
            break;
        }
        case RULE_ASSIGN_ACCESS: {
            auto assign_access = static_cast<AssignAccess *>(rule);
            break;
        }
        case RULE_LOGICAL: {
            auto logical = static_cast<Logical *>(rule);
            break;
        }
        case RULE_FUNCTION: {
            auto function = static_cast<Function *>(rule);
            auto memory = this->current_memory;
            this->blocks.push_back(&function->block);
            this->current_memory = FUNCTIONS_MEMORY;

            this->current_memory = memory;
            this->blocks.pop_back();
            break;
        }
        case RULE_CALL: {
            auto call = static_cast<Call *>(rule);
            break;
        }
        case RULE_ACCESS: {
            auto access = static_cast<Access *>(rule);
            break;
        }
        default: {
            logger->error("Invalid expression to compile.", rule->line);
            exit(EXIT_FAILURE);
        }
    }
    delete rule;
    return result;
}

uint64_t Compiler::add_constant_only(Value value)
{
    this->get_current_memory()->constants.push_back(value);

    uint64_t index = this->get_current_memory()->constants.size() - 1;
    this->add_opcode(index);

    return index;
}

void Compiler::modify_constant(uint64_t index, Value value)
{
    this->get_current_memory()->constants[index] = value;
}

uint32_t Compiler::current_code_line()
{
    return this->get_current_memory()->code.size();
}
