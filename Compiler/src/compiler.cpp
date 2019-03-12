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

void Compiler::add_opcodes(uint64_t opcode, uint8_t times)
{
    while (times-- > 0) this->add_opcode(opcode);
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
    this->add_opcodes(OP_EMPTY, 3);

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
            if (this->is_constant(print->expression)) {
                this->add_opcode(OP_PRINT_C);
                this->compile(print->expression, false);
                this->add_opcodes(OP_EMPTY, 2);
            } else {
                auto reg = this->compile(print->expression);
                this->add_opcode(OP_PRINT_R);
                this->add_opcode(reg);
                this->add_opcodes(OP_EMPTY, 2);
                this->frame_info.back().free_register(reg);
            }
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
            this->get_variable(declaration->name)->reg = reg;
            if (declaration->initializer) {
                if (this->is_constant(declaration->initializer)) {
                    this->add_opcode(OP_MOVE_RC);
                    this->add_opcode(reg);
                    this->compile(declaration->initializer, false);
                    this->add_opcodes(OP_EMPTY, 1);
                } else {
                    this->compile(declaration->initializer, true, &reg);
                    // auto res = this->compile(declaration->initializer);
                    // this->add_opcode(OP_MOVE_RR);
                    // this->add_opcode(reg);
                    // this->add_opcode(res);
                    // this->frame_info.back().free_register(res);
                }
            } else {
                this->add_opcode(OP_MOVE_RC);
                this->add_opcode(reg);
                this->add_constant_only(Type(declaration->type));
                this->add_opcodes(OP_EMPTY, 1);
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
            this->blocks.push_back(&rwhile->block);
            auto initial_index = this->current_code_line();
            uint32_t literal_position;
            if (this->is_constant(rwhile->condition)) {
                this->add_opcode(OP_FNJUMP_C);
                this->add_opcode(0); // This will determine the jump offset.
                literal_position = this->current_code_line() - 1;
                this->compile(rwhile->condition, false);
                this->add_opcodes(OP_EMPTY, 1);
            } else {
                auto r1 = this->compile(rwhile->condition);
                this->add_opcode(OP_FNJUMP_R);
                this->add_opcode(0); // This will determine the jump offset.
                literal_position = this->current_code_line() - 1;
                this->add_opcode(r1);
                this->add_opcodes(OP_EMPTY, 1);
            }
            auto start_index = this->current_code_line();
            // Compile the while body.
            for (auto stmt : rwhile->body) this->compile(stmt);
            // Jump back to the condition.
            this->add_opcode(OP_BJUMP);
            this->add_opcode(((this->current_code_line() - 2 - initial_index - 1) / MAX_OPERANDS) + 1);
            this->add_opcodes(OP_EMPTY, 2);
            // Modify the jump offset of the literal in the condition.
            this->modify_literal(literal_position, ((this->current_code_line() - start_index + 3) / MAX_OPERANDS) - 1);
            this->blocks.pop_back();
            break;
        }
        default: {
            logger->error("Invalid statement to compile.", rule->line);
            exit(EXIT_FAILURE);
        }
    }
    delete rule;
}

uint64_t Compiler::compile(Expression *rule, bool const_opcode, uint64_t *suggested_register)
{
    this->current_line = rule->line;
    uint64_t result = 0;

    switch (rule->rule) {
        case RULE_INTEGER: {
            auto integer = static_cast<Integer *>(rule);
            if (const_opcode) {
                this->add_opcode(OP_MOVE_RC);
                this->add_opcode(result = this->frame_info.back().get_register());
            }
            this->add_constant_only(integer->value);
            break;
        }
        case RULE_FLOAT: {
            auto floating = static_cast<Float *>(rule);
            if (const_opcode) {
                this->add_opcode(OP_MOVE_RC);
                this->add_opcode(result = this->frame_info.back().get_register());
            }
            this->add_constant_only(floating->value);
            break;
        }
        case RULE_STRING: {
            auto string = static_cast<String *>(rule);
            if (const_opcode) {
                this->add_opcode(OP_MOVE_RC);
                this->add_opcode(result = this->frame_info.back().get_register());
            }
            this->add_constant_only(string->value);
            break;
        }
        case RULE_BOOLEAN: {
            auto boolean = static_cast<Boolean *>(rule);
            if (const_opcode) {
                this->add_opcode(OP_MOVE_RC);
                this->add_opcode(result = this->frame_info.back().get_register());
            }
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
        /*
        case RULE_NONE: {
            if (const_opcode) {
                this->add_opcode(OP_MOVE_RC);
                this->add_opcode(result = this->frame_info.back().get_register());
            }
            this->add_constant_only(Value());
            break;
        }
        */
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
            uint64_t token;
            switch (binary->op.type) {
                case TOKEN_PLUS: { token = OP_ADD_RR; break; }
                case TOKEN_MINUS: { token = OP_SUB_RR; break; }
                case TOKEN_STAR: { token = OP_MUL_RR; break; }
                case TOKEN_SLASH: { token = OP_DIV_RR; break; }
                case TOKEN_EQUAL_EQUAL: { token = OP_EQ_RR; break; }
                case TOKEN_BANG_EQUAL: { token = OP_NEQ_RR; break; }
                case TOKEN_LOWER: { token = OP_LT_RR; break; }
                case TOKEN_LOWER_EQUAL: { token = OP_LTE_RR; break; }
                case TOKEN_HIGHER: { token = OP_HT_RR; break; }
                case TOKEN_HIGHER_EQUAL: { token = OP_HTE_RR; break; }
                default: { token = 0; }
            }

            if (this->is_constant(binary->left) && this->is_constant(binary->right)) {
                this->add_opcode(token + 3);
                if (suggested_register != nullptr) this->add_opcode(result = *suggested_register);
                else this->add_opcode(result = this->frame_info.back().get_register());
                this->compile(binary->left, false);
                this->compile(binary->right, false);
            } else if (this->is_constant(binary->left)) {
                auto r1 = this->compile(binary->right);
                this->frame_info.back().free_register(r1);
                this->add_opcode(token + 2);
                if (suggested_register != nullptr) this->add_opcode(result = *suggested_register);
                else this->add_opcode(result = this->frame_info.back().get_register());
                this->compile(binary->left, false);
                this->add_opcode(r1);
            } else if (this->is_constant(binary->right)) {
                auto r1 = this->compile(binary->left);
                this->frame_info.back().free_register(r1);
                this->add_opcode(token + 1);
                if (suggested_register != nullptr) this->add_opcode(result = *suggested_register);
                else this->add_opcode(result = this->frame_info.back().get_register());
                this->add_opcode(r1);
                this->compile(binary->right, false);
            } else {
                auto r1 = this->compile(binary->left);
                auto r2 = this->compile(binary->right);
                this->frame_info.back().free_register(r1);
                this->frame_info.back().free_register(r2);
                this->add_opcode(token);
                if (suggested_register != nullptr) this->add_opcode(result = *suggested_register);
                else this->add_opcode(result = this->frame_info.back().get_register());
                this->add_opcode(r1);
                this->add_opcode(r2);
            }
            break;
        }
        case RULE_VARIABLE: {
            auto variable = static_cast<Variable *>(rule);
            result = this->get_variable(variable->name)->reg;
            break;
        }
        case RULE_ASSIGN: {
            /*
            Assign *assign = static_cast<Assign *>(rule);
            result = this->get_variable(assign->name)->reg;
            if (this->is_constant(assign->value)) {
                this->add_opcode(OP_MOVE_RC);
                this->add_opcode(result);
                this->compile(assign->value, false);
            } else {
                this->compile(assign->value, true, &result);
                // auto res = this->compile(assign->value);
                // this->add_opcode(OP_MOVE_RR);
                // this->add_opcode(result);
                // this->add_opcode(res);
                // this->frame_info.back().free_register(res);
            }
            break;
            */
        }
        /*
        case RULE_ASSIGN_ACCESS: {
            auto assign_access = static_cast<AssignAccess *>(rule);
            break;
        }
        */
        case RULE_LOGICAL: {
            auto logical = static_cast<Logical *>(rule);
            break;
        }
        /*
        case RULE_FUNCTION: {
            auto function = static_cast<Function *>(rule);
            auto memory = this->current_memory;
            this->blocks.push_back(&function->block);
            this->current_memory = FUNCTIONS_MEMORY;

            this->current_memory = memory;
            this->blocks.pop_back();
            break;
        }
        */
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

void Compiler::modify_literal(size_t index, uint64_t value)
{
    this->get_current_memory()->code[index] = value;
}

uint64_t Compiler::current_code_line()
{
    return this->get_current_memory()->code.size();
}

bool Compiler::is_constant(Expression *expression)
{
    switch (expression->rule) {
        case RULE_INTEGER:
        case RULE_FLOAT:
        case RULE_BOOLEAN:
        case RULE_STRING: { return true; }
        default: { return false; }
    }
}

BlockVariableType *Compiler::get_variable(std::string &name)
{
    for (int16_t i = this->blocks.size() - 1; i >= 0; i--) {
        auto var = this->blocks[i]->get_variable(name);
        if (var) return var;
    }

    return nullptr;
}
