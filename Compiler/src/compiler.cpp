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
#include "../../Parser/include/parser.hpp"
#include "../include/program.hpp"
#include "../include/memory.hpp"
#include <algorithm>

#define ADD_LOG(rule, msg) (logger->add_entity(rule->file, rule->line, rule->column, msg))

static std::vector<std::vector<Statement *> *> compiled_modules;

Program Compiler::compile(const char *file)
{
    Analyzer analyzer = Analyzer(file);
    std::vector<Statement *> *code = new std::vector<Statement *>;
    Block block = analyzer.analyze(code);

    // Create the program memory.
    this->program.memory = new Memory();
    // Register the TLDs.
    this->register_tld(code, &block);
    // Allocate the main registers.
    this->program.main_frame.registers_size = this->global.current_register;
    this->program.main_frame.allocate_registers();

    // Compile the code.
    this->compile_module(code, &block);

    // Add the exit opcode.
    this->add_opcodes({{ OP_EXIT }});

    this->program.memory->dump();

    return this->program;
}

void Compiler::compile_module(std::vector<Statement *> *code, Block *block)
{
    std::vector<Use *> delay_usages;
    this->blocks.push_back(block);
    for (Statement *node : *code) {
        compiler_compile_module:
        switch (node->rule) {
            case RULE_USE: {
                delay_usages.push_back(static_cast<Use *>(node));
                break;
            }
            case RULE_EXPORT: {
                node = static_cast<Export *>(node)->statement;
                goto compiler_compile_module;
                break;
            }
            case RULE_CLASS: {
                break;
            }
            case RULE_FUNCTION: {
                Function *fun = static_cast<Function *>(node);
                // Get the entry point of the function.
                size_t entry = this->program.memory->code->size();
                // Push the function block.
                this->blocks.push_back(&fun->block);
                // Pop the function parameters.
                for (Declaration *param : fun->parameters) {
                    // Get the variable from the block.
                    BlockVariableType *var = this->get_variable(param->name).first;
                    // Assign the register to the parameter.
                    var->reg = this->local.get_register(true);
                    // Pop the parameter from the stack.
                    this->add_opcodes({{ OP_POP, var->reg }});
                }
                // Compile the function body.
                for (Statement *n : fun->body) this->compile(n);
                this->blocks.pop_back();
                // Create the function value and move it to the register.
                Value(entry, this->local.current_register).copy_to(
                    this->program.main_frame.registers + block->get_variable(fun->name)->reg
                );
                // Reset the local frame info.
                this->local.reset();
                break;
            }
            default: {
                ADD_LOG(node, "Invalid Top Level Declaration rule to compile");
                exit(logger->crash());
            }
        }
    }
    this->blocks.pop_back();
    for (Use *use : delay_usages) {
        // Only compile if needed.
        if (std::find(compiled_modules.begin(), compiled_modules.end(), code) == compiled_modules.end()) {
            // Register the module.
            this->compile_module(use->code, &use->block);
            // Set the module as registered.
            compiled_modules.push_back(code);
        }
    }
}

void Compiler::register_tld(std::vector<Statement *> *code, Block *block)
{
    static std::vector<std::vector<Statement *> *> registered_modules;
    std::vector<Use *> delay_usages;
    for (Statement *tld : *code) {
        compiler_register_tld:
        switch (tld->rule) {
            case RULE_USE: {
                delay_usages.push_back(static_cast<Use *>(tld));
                break;
            }
            case RULE_EXPORT: {
                tld = static_cast<Export *>(tld)->statement;
                goto compiler_register_tld;
                break;
            }
            case RULE_CLASS: {
                break;
            }
            case RULE_FUNCTION: {
                Function *fun = static_cast<Function *>(tld);
                BlockVariableType *var = block->get_variable(fun->name);
                var->reg = this->global.get_register(true);
                break;
            }
            default: {
                ADD_LOG(tld, "Invalid Top Level Declaration rule to register");
                exit(logger->crash());
            }
        }
    }
    for (Use *use : delay_usages) {
        // Make sure it's not registered yet.
        if (std::find(registered_modules.begin(), registered_modules.end(), use->code) == registered_modules.end()) {
            // Register the module.
            this->register_tld(use->code, &use->block);
            // Set the module as registered.
            registered_modules.push_back(use->code);
        }
        // Register the targets in the current block with the same register as use block.
        for (std::string &name : use->targets) {
            block->get_variable(name)->reg = use->block.get_variable(name)->reg;
        }
    }
    block->debug();
}

void Compiler::compile(Statement *rule)
{
    switch (rule->rule) {
        case RULE_PRINT: {
            Print *print = static_cast<Print *>(rule);
            if (this->is_constant(print->expression)) {
                this->add_opcodes({{ OP_PRINT_C }});
                this->compile(print->expression, false);
            } else {
                register_t rx = this->compile(print->expression);
                this->add_opcodes({{ OP_PRINT, rx }});
                this->local.free_register(rx);
            }
            break;
        }
        case RULE_EXPRESSION_STATEMENT: {
            this->compile(static_cast<ExpressionStatement *>(rule)->expression);
            break;
        }
        case RULE_DECLARATION: {
            Declaration *dec = static_cast<Declaration *>(rule);
            // Get a register from the register allocator.
            register_t rx = this->local.get_register(true);
            // Set the variable register in the current block.
            this->get_variable(dec->name).first->reg = rx;
            if (dec->initializer) {
                if (this->is_constant(dec->initializer)) {
                    this->add_opcodes({{ OP_LOAD_C, rx }});
                    this->compile(dec->initializer, false);
                } else this->compile(dec->initializer, true, &rx);
            } else {
                this->add_opcodes({{ OP_LOAD_C, rx, this->add_constant(Value(dec->type)) }});
            }
            break;
        }
        case RULE_RETURN: {
            Return *ret = static_cast<Return *>(rule);
            if (ret->value) {
                if (this->is_constant(ret->value)) {
                    this->add_opcodes({{ OP_PUSH_C }});
                    this->compile(ret->value, false);
                } else {
                    register_t res = this->compile(ret->value);
                    this->add_opcodes({{ OP_PUSH, res }});
                    this->local.free_register(res);
                }
            }
            this->add_opcodes({{ OP_RETURN }});
            break;
        }
        case RULE_IF: {
            If *rif = static_cast<If *>(rule);
            break;
        }
        case RULE_WHILE: {
            While *rwhile = static_cast<While *>(rule);
            break;
        }
        case RULE_FOR: {
            For *rfor = static_cast<For *>(rule);
            break;
        }
        default: {
            ADD_LOG(rule, "Compilation error: Invalid statement to compile");
            exit(logger->crash());
        }
    }
    delete rule;
}

register_t Compiler::compile(Expression *rule, bool load_constant, register_t *suggested_register)
{
    register_t result = 0;
    switch (rule->rule) {
        case RULE_INTEGER: {
            if (load_constant) this->add_opcodes({{ OP_LOAD_C, result = suggested_register ? *suggested_register : this->local.get_register() }});
            this->add_opcodes({{ this->add_constant({ static_cast<Integer *>(rule)->value }) }});
            break;
        }
        case RULE_FLOAT: {
            if (load_constant) this->add_opcodes({{ OP_LOAD_C, result = suggested_register ? *suggested_register : this->local.get_register() }});
            this->add_opcodes({{ this->add_constant({ static_cast<Float *>(rule)->value }) }});
            break;
        }
        case RULE_STRING: {
            if (load_constant) this->add_opcodes({{ OP_LOAD_C, result = suggested_register ? *suggested_register : this->local.get_register() }});
            this->add_opcodes({{ this->add_constant({ static_cast<String *>(rule)->value }) }});
            break;
        }
        case RULE_BOOLEAN: {
            if (load_constant) this->add_opcodes({{ OP_LOAD_C, result = suggested_register ? *suggested_register : this->local.get_register() }});
            this->add_opcodes({{ this->add_constant({ static_cast<Boolean *>(rule)->value }) }});
            break;
        }
        case RULE_LIST: {
            List *list = static_cast<List *>(rule);
            result = suggested_register ? *suggested_register : this->local.get_register();
            if (this->is_constant(list)) {
                // The list can be stored in the constant pool.
                if (load_constant) this->add_opcodes({{ OP_LOAD_C, result }});
                // Value v = this->constant_list(list);
                // this->add_opcodes({{ this->add_constant(v) }});
            } else {
                // The list needs to be constructed from the groud up
                printf("Creating empty list...");
                auto a = this->add_constant(Value(list->type));
                printf("OK!\n");
                this->add_opcodes({{ OP_LOAD_C, result, a }});
                printf("OK!\n");
                for (Expression *e : list->value) {
                    Parser::debug_ast(e);
                    register_t ry = this->compile(e);
                    printf("COMPILED LIST ITEM\n");
                    // this->add_opcodes({{ OP_LPUSH, result, ry }});
                    // this->local.free_register(ry);
                }
                printf("OK!\n");
            }
            break;
        }
        case RULE_DICTIONARY: {
            Dictionary *dict = static_cast<Dictionary *>(rule);
            break;
        }
        case RULE_GROUP: {
            this->compile(static_cast<Group *>(rule)->expression, suggested_register);
            break;
        }
        case RULE_CAST: {
            Cast *cast = static_cast<Cast *>(rule);
            break;
        }
        case RULE_UNARY: {
            Unary *unary = static_cast<Unary *>(rule);
            break;
        }
        case RULE_BINARY: {
            Binary *binary = static_cast<Binary *>(rule);
            opcode_t base = OP_ADD_INT;
            register_t ry = this->compile(binary->left);
            register_t rz = this->compile(binary->right);
            this->add_opcodes({{ base + binary->type, result = suggested_register ? *suggested_register : this->local.get_register(), ry, rz }});
            break;
        }
        case RULE_VARIABLE: {
            std::pair<BlockVariableType *, bool> var = this->get_variable(static_cast<Variable *>(rule)->name);
            if (var.second) {
                // The variable is global, and needs to be loaded first.
                result = suggested_register ? *suggested_register : this->local.get_register();
                this->add_opcodes({{ OP_LOAD_G, result, var.first->reg }});
            } else result = var.first->reg;
            break;
        }
        case RULE_ASSIGN: {
            Assign *assign = static_cast<Assign *>(rule);
            break;
        }
        case RULE_LOGICAL: {
            Logical *logical = static_cast<Logical *>(rule);
            break;
        }
        case RULE_CALL: {
            Call *call = static_cast<Call *>(rule);
            break;
        }
        case RULE_ACCESS: {
            Access *access = static_cast<Access *>(rule);
            break;
        }
        case RULE_SLICE: {
            Slice *slice = static_cast<Slice *>(rule);
            break;
        }
        case RULE_RANGE: {
            Range *range = static_cast<Range *>(rule);
            break;
        }
        default: {
            ADD_LOG(rule, "Compilation error: Invalid expression to compile");
            exit(logger->crash());
        }
    }
    delete rule;
    return result;
}

std::pair<BlockVariableType *, bool> Compiler::get_variable(std::string &name)
{
    for (size_t i = this->blocks.size() - 1; i >= 0; i--) {
        BlockVariableType *var = this->blocks[i]->get_variable(name);
        if (var) return { var, i == 0 };
    }
    return { nullptr, false };
}

void Compiler::add_opcodes(std::vector<opcode_t> opcodes)
{
    for (opcode_t &op : opcodes) this->program.memory->code->push_back(op);
}

size_t Compiler::add_constant(Value value)
{
    printf("Adding constant: %s\n", value.type.to_string().c_str());
    this->program.memory->constants.push_back(value);
    printf("Constant added.\n");
    return this->program.memory->constants.size() - 1;
}

Value Compiler::constant_list(List *list)
{
    Value dest = Value(list->type);
    for (Expression *e : list->value) {
        switch (e->rule) {
            case RULE_INTEGER: { dest.value_list->push_back({ static_cast<Integer *>(e)->value }); break; }
            case RULE_FLOAT: { dest.value_list->push_back({ static_cast<Float *>(e)->value }); break; }
            case RULE_BOOLEAN: { dest.value_list->push_back({ static_cast<Boolean *>(e)->value }); break; }
            case RULE_STRING:  { dest.value_list->push_back({ static_cast<String *>(e)->value }); break; }
            case RULE_LIST: {
                dest.value_list->push_back(this->constant_list(static_cast<List *>(e)));
                break;
            }
            case RULE_DICTIONARY: { dest.value_list->push_back(this->constant_dict(static_cast<Dictionary *>(e))); break; }
            default: {
                logger->add_entity(e->file, e->line, e->column, "Unable to create a constant list because of unknown constant list item.");
                exit(logger->crash());
            }
        }
        delete e;
    }
    return dest;
}

Value Compiler::constant_dict(Dictionary *dict)
{
    Value v = Value(dict->type);
    for (std::pair<std::string, Expression *> el : dict->value) {
        switch (el.second->rule) {
            case RULE_INTEGER: { v.value_dict->insert(el.first, { static_cast<Integer *>(el.second)->value }); break; }
            case RULE_FLOAT: { v.value_dict->insert(el.first, { static_cast<Float *>(el.second)->value }); break; }
            case RULE_BOOLEAN: { v.value_dict->insert(el.first, { static_cast<Boolean *>(el.second)->value }); break; }
            case RULE_STRING:  { v.value_dict->insert(el.first, { static_cast<String *>(el.second)->value }); break; }
            case RULE_LIST: { v.value_dict->insert(el.first, this->constant_list(static_cast<List *>(el.second))); break; }
            case RULE_DICTIONARY: { v.value_dict->insert(el.first, this->constant_dict(static_cast<Dictionary *>(el.second))); break; }
            default: {
                logger->add_entity(el.second->file, el.second->line, el.second->column, "Unable to create a constant dictionary because of unknown constant dictionary item.");
                exit(logger->crash());
            }
        }
        delete el.second;
    }
    return v;
}

bool Compiler::is_constant(Expression *expression)
{
    switch (expression->rule) {
        case RULE_INTEGER:
        case RULE_FLOAT:
        case RULE_BOOLEAN:
        case RULE_STRING: { return true; }
        case RULE_LIST: {
            List *l = static_cast<List *>(expression);
            for (Expression *value : l->value) {
                if (!this->is_constant(value)) return false;
            }
            return true;
        }
        case RULE_DICTIONARY: {
            Dictionary *d = static_cast<Dictionary *>(expression);
            for (auto &[key, value] : d->value) {
                if (!this->is_constant(value)) return false;
            }
            return true;
        }
        default: { return false; }
    }
}

void Compiler::set_file(const std::string *file)
{
    this->program.memory->files->insert(
        {{ this->program.memory->code->size(), file }}
    );

    this->current_file = file;
}

void Compiler::set_line(const line_t line)
{
    this->program.memory->lines->insert(
        {{ this->program.memory->code->size(), line }}
    );

    this->current_line = line;
}
void Compiler::set_column(const column_t column)
{
    this->program.memory->columns->insert(
        {{ this->program.memory->code->size(), column }}
    );

    this->current_column = column;
}

#undef ADD_LOG
