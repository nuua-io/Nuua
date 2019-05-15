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

void Compiler::compile(const char *file)
{
    printf("----> Compiler\n");
    Analyzer analyzer = Analyzer(file);
    std::shared_ptr<std::vector<std::shared_ptr<Statement>>>code = std::make_shared<std::vector<std::shared_ptr<Statement>>>();
    std::shared_ptr<Block> block = analyzer.analyze(code);

    // Register the TLDs.
    this->register_tld(code, block);
    // Allocate the main registers.
    this->program->main_frame.registers_size = this->global.current_register;
    this->program->main_frame.allocate_registers();

    // Compile the code.
    this->compile_module(code, block);

    // Add the exit opcode.
    this->add_opcodes({{ OP_EXIT }});

    this->program->memory->dump();

    printf("----> !Compiler\n");
}

void Compiler::compile_module(const std::shared_ptr<std::vector<std::shared_ptr<Statement>>> &code, const std::shared_ptr<Block> &block)
{
    static std::vector<std::shared_ptr<std::vector<std::shared_ptr<Statement>>>> compiled_modules;
    std::vector<std::shared_ptr<Use>> delay_usages;
    this->blocks.push_back(block);
    for (std::shared_ptr<Statement> &node : *code) {
        compiler_compile_module:
        switch (node->rule) {
            case RULE_USE: {
                delay_usages.push_back(std::static_pointer_cast<Use>(node));
                break;
            }
            case RULE_EXPORT: {
                node = std::static_pointer_cast<Export>(node)->statement;
                goto compiler_compile_module;
                break;
            }
            case RULE_CLASS: {
                break;
            }
            case RULE_FUNCTION: {
                std::shared_ptr<Function> fun = std::static_pointer_cast<Function>(node);
                // Get the entry point of the function.
                size_t entry = this->program->memory->code.size();
                // Push the function block.
                this->blocks.push_back(fun->block);
                // Pop the function parameters.
                for (const std::shared_ptr<Declaration> &param : fun->parameters) {
                    // Get the variable from the block.
                    BlockVariableType *var = this->get_variable(param->name).first;
                    // Assign the register to the parameter.
                    var->reg = this->local.get_register(true);
                    // Pop the parameter from the stack.
                    this->add_opcodes({{ OP_POP, var->reg }});
                }
                // Compile the function body.
                for (const std::shared_ptr<Statement> &statement : fun->body) this->compile(statement);
                // Clear the function body (so that the elements may be freed)
                fun->body.clear();
                this->blocks.pop_back();
                // Create the function value and move it to the register.
                Value(entry, this->local.current_register).copy_to(
                    this->program->main_frame.registers + block->get_variable(fun->name)->reg
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
    code->clear();
    this->blocks.pop_back();
    for (const std::shared_ptr<Use> &use : delay_usages) {
        // Only compile if needed.
        if (std::find(compiled_modules.begin(), compiled_modules.end(), code) == compiled_modules.end()) {
            // Register the module.
            this->compile_module(use->code, use->block);
            // Set the module as registered.
            compiled_modules.push_back(code);
        }
    }
}

void Compiler::register_tld(const std::shared_ptr<std::vector<std::shared_ptr<Statement>>> &code, const std::shared_ptr<Block> &block)
{
    static std::vector<std::shared_ptr<std::vector<std::shared_ptr<Statement>>>> registered_modules;
    std::vector<std::shared_ptr<Use>> delay_usages;
    for (std::shared_ptr<Statement> &tld : *code) {
        compiler_register_tld:
        switch (tld->rule) {
            case RULE_USE: {
                delay_usages.push_back(std::static_pointer_cast<Use>(tld));
                break;
            }
            case RULE_EXPORT: {
                tld = std::static_pointer_cast<Export>(tld)->statement;
                goto compiler_register_tld;
                break;
            }
            case RULE_CLASS: {
                break;
            }
            case RULE_FUNCTION: {
                std::shared_ptr<Function> fun = std::static_pointer_cast<Function>(tld);
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
    for (const std::shared_ptr<Use> &use : delay_usages) {
        // Make sure it's not registered yet.
        if (std::find(registered_modules.begin(), registered_modules.end(), use->code) == registered_modules.end()) {
            // Register the module.
            this->register_tld(use->code, use->block);
            // Set the module as registered.
            registered_modules.push_back(use->code);
        }
        // Register the targets in the current block with the same register as use block.
        for (const std::string &name : use->targets) {
            block->get_variable(name)->reg = use->block->get_variable(name)->reg;
        }
    }
    block->debug();
}

void Compiler::compile(const std::shared_ptr<Statement> &rule)
{
    switch (rule->rule) {
        case RULE_PRINT: {
            std::shared_ptr<Print> print = std::static_pointer_cast<Print>(rule);
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
            this->compile(std::static_pointer_cast<ExpressionStatement>(rule)->expression);
            break;
        }
        case RULE_DECLARATION: {
            std::shared_ptr<Declaration> dec = std::static_pointer_cast<Declaration>(rule);
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
            std::shared_ptr<Return> ret = std::static_pointer_cast<Return>(rule);
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
            std::shared_ptr<If> rif = std::static_pointer_cast<If>(rule);
            break;
        }
        case RULE_WHILE: {
            std::shared_ptr<While> rwhile = std::static_pointer_cast<While>(rule);
            break;
        }
        case RULE_FOR: {
            std::shared_ptr<For> rfor = std::static_pointer_cast<For>(rule);
            break;
        }
        default: {
            ADD_LOG(rule, "Compilation error: Invalid statement to compile");
            exit(logger->crash());
        }
    }
}

register_t Compiler::compile(const std::shared_ptr<Expression> &rule, const bool load_constant, const register_t *suggested_register)
{
    register_t result = 0;
    switch (rule->rule) {
        case RULE_INTEGER: {
            if (load_constant) this->add_opcodes({{ OP_LOAD_C, result = suggested_register ? *suggested_register : this->local.get_register() }});
            this->add_opcodes({{ this->add_constant({ std::static_pointer_cast<Integer>(rule)->value }) }});
            break;
        }
        case RULE_FLOAT: {
            if (load_constant) this->add_opcodes({{ OP_LOAD_C, result = suggested_register ? *suggested_register : this->local.get_register() }});
            this->add_opcodes({{ this->add_constant({ std::static_pointer_cast<Float>(rule)->value }) }});
            break;
        }
        case RULE_STRING: {
            if (load_constant) this->add_opcodes({{ OP_LOAD_C, result = suggested_register ? *suggested_register : this->local.get_register() }});
            this->add_opcodes({{ this->add_constant({ std::static_pointer_cast<String>(rule)->value }) }});
            break;
        }
        case RULE_BOOLEAN: {
            if (load_constant) this->add_opcodes({{ OP_LOAD_C, result = suggested_register ? *suggested_register : this->local.get_register() }});
            this->add_opcodes({{ this->add_constant({ std::static_pointer_cast<Boolean>(rule)->value }) }});
            break;
        }
        case RULE_LIST: {
            std::shared_ptr<List> list = std::static_pointer_cast<List>(rule);
            result = suggested_register ? *suggested_register : this->local.get_register();
            if (this->is_constant(list)) {
                // The list can be stored in the constant pool.
                if (load_constant) this->add_opcodes({{ OP_LOAD_C, result }});
                Value v;
                this->constant_list(list, v);
                this->add_opcodes({{ this->add_constant(v) }});
            } else {
                // The list needs to be constructed from the groud up
                this->add_opcodes({{ OP_LOAD_C, result, this->add_constant(Value(list->type)) }});
                for (const std::shared_ptr<Expression> &e : list->value) {
                    if (this->is_constant(e)) {
                        this->add_opcodes({{ OP_LPUSH_C, result }});
                        this->compile(e, false);
                    } else {
                        register_t ry = this->compile(e);
                        this->add_opcodes({{ OP_LPUSH, result, ry }});
                        this->local.free_register(ry);
                    }
                }
            }
            break;
        }
        case RULE_DICTIONARY: {
            std::shared_ptr<Dictionary> dict = std::static_pointer_cast<Dictionary>(rule);
            break;
        }
        case RULE_GROUP: {
            this->compile(std::static_pointer_cast<Group>(rule)->expression, suggested_register);
            break;
        }
        case RULE_CAST: {
            std::shared_ptr<Cast> cast = std::static_pointer_cast<Cast>(rule);
            opcode_t base = OP_CAST_INT_FLOAT;
            register_t rx = this->compile(cast->expression);
            this->add_opcodes({{ base + cast->cast_type, result = suggested_register ? *suggested_register : this->local.get_register(), rx }});
            break;
        }
        case RULE_UNARY: {
            std::shared_ptr<Unary> unary = std::static_pointer_cast<Unary>(rule);
            opcode_t base = OP_NEG_BOOL;
            register_t rx = this->compile(unary->right);
            this->add_opcodes({{ base + unary->type, result = suggested_register ? *suggested_register : this->local.get_register(), rx }});
            break;
        }
        case RULE_BINARY: {
            std::shared_ptr<Binary> binary = std::static_pointer_cast<Binary>(rule);
            opcode_t base = OP_ADD_INT;
            register_t ry = this->compile(binary->left);
            register_t rz = this->compile(binary->right);
            this->add_opcodes({{ base + binary->type, result = suggested_register ? *suggested_register : this->local.get_register(), ry, rz }});
            break;
        }
        case RULE_VARIABLE: {
            std::pair<BlockVariableType *, bool> var = this->get_variable(std::static_pointer_cast<Variable>(rule)->name);
            if (var.second) {
                // The variable is global, and needs to be loaded first.
                result = suggested_register ? *suggested_register : this->local.get_register();
                this->add_opcodes({{ OP_LOAD_G, result, var.first->reg }});
            } else result = var.first->reg;
            break;
        }
        case RULE_ASSIGN: {
            std::shared_ptr<Assign> assign = std::static_pointer_cast<Assign>(rule);
            break;
        }
        case RULE_LOGICAL: {
            std::shared_ptr<Logical> logical = std::static_pointer_cast<Logical>(rule);
            break;
        }
        case RULE_CALL: {
            std::shared_ptr<Call> call = std::static_pointer_cast<Call>(rule);
            break;
        }
        case RULE_ACCESS: {
            std::shared_ptr<Access> access = std::static_pointer_cast<Access>(rule);
            break;
        }
        case RULE_SLICE: {
            std::shared_ptr<Slice> slice = std::static_pointer_cast<Slice>(rule);
            break;
        }
        case RULE_RANGE: {
            std::shared_ptr<Range> range = std::static_pointer_cast<Range>(rule);
            break;
        }
        default: {
            ADD_LOG(rule, "Compilation error: Invalid expression to compile");
            exit(logger->crash());
        }
    }
    return result;
}

std::pair<BlockVariableType *, bool> Compiler::get_variable(const std::string &name)
{
    for (size_t i = this->blocks.size() - 1; i >= 0; i--) {
        BlockVariableType *var = this->blocks[i]->get_variable(name);
        if (var) return { var, i == 0 };
    }
    return { nullptr, false };
}

void Compiler::add_opcodes(const std::vector<opcode_t> &opcodes)
{
    for (const opcode_t &op : opcodes) this->program->memory->code.push_back(op);
}

size_t Compiler::add_constant(const Value &value)
{
    printf("Adding constant: %s\n", value.type.to_string().c_str());
    this->program->memory->constants.push_back(std::move(value));
    printf("Constant added.\n");
    return this->program->memory->constants.size() - 1;
}

void Compiler::constant_list(const std::shared_ptr<List> &list, Value &dest)
{
    for (const std::shared_ptr<Expression> &value : list->value) {
        switch (value->rule) {
            case RULE_INTEGER: { dest.value_list->push_back({ std::static_pointer_cast<Integer>(value)->value }); break; }
            case RULE_FLOAT: { dest.value_list->push_back({ std::static_pointer_cast<Float>(value)->value }); break; }
            case RULE_BOOLEAN: { dest.value_list->push_back({ std::static_pointer_cast<Boolean>(value)->value }); break; }
            case RULE_STRING:  { dest.value_list->push_back({ std::static_pointer_cast<String>(value)->value }); break; }
            case RULE_LIST: {
                Value v;
                this->constant_list(std::static_pointer_cast<List>(value), v);
                dest.value_list->push_back(std::move(v));
                break;
            }
            case RULE_DICTIONARY: {
                Value v;
                this->constant_dict(std::static_pointer_cast<Dictionary>(value), v);
                dest.value_list->push_back(std::move(v));
                break;
            }
            default: {
                logger->add_entity(value->file, value->line, value->column, "Unable to create a constant list because of unknown constant list item.");
                exit(logger->crash());
            }
        }
    }
}

void Compiler::constant_dict(const std::shared_ptr<Dictionary> &dict, Value &dest)
{
    for (const auto &[key, value] : dict->value) {
        switch (value->rule) {
            case RULE_INTEGER: { dest.value_dict->insert(key, { std::static_pointer_cast<Integer>(value)->value }); break; }
            case RULE_FLOAT: { dest.value_dict->insert(key, { std::static_pointer_cast<Float>(value)->value }); break; }
            case RULE_BOOLEAN: { dest.value_dict->insert(key, { std::static_pointer_cast<Boolean>(value)->value }); break; }
            case RULE_STRING:  { dest.value_dict->insert(key, { std::static_pointer_cast<String>(value)->value }); break; }
            case RULE_LIST: {
                Value v;
                this->constant_list(std::static_pointer_cast<List>(value), v);
                dest.value_dict->insert(key, std::move(v));
                break;
            }
            case RULE_DICTIONARY: {
                Value v;
                this->constant_dict(std::static_pointer_cast<Dictionary>(value), v);
                dest.value_dict->insert(key, std::move(v));
                break;
            }
            default: {
                logger->add_entity(value->file, value->line, value->column, "Unable to create a constant list because of unknown constant list item.");
                exit(logger->crash());
            }
        }
    }
}

bool Compiler::is_constant(const std::shared_ptr<Expression> &expression)
{
    switch (expression->rule) {
        case RULE_INTEGER:
        case RULE_FLOAT:
        case RULE_BOOLEAN:
        case RULE_STRING: { return true; }
        case RULE_LIST: {
            for (const std::shared_ptr<Expression> &value : std::static_pointer_cast<List>(expression)->value) {
                if (!this->is_constant(value)) return false;
            }
            return true;
        }
        case RULE_DICTIONARY: {
            for (const auto &[key, value] : std::static_pointer_cast<Dictionary>(expression)->value) {
                if (!this->is_constant(value)) return false;
            }
            return true;
        }
        default: { return false; }
    }
}

void Compiler::set_file(const std::shared_ptr<const std::string> &file)
{
    this->program->memory->files[this->program->memory->code.size()] = file;
    this->current_file = file;
}

void Compiler::set_line(const line_t line)
{
    this->program->memory->lines[this->program->memory->code.size()] = line;
    this->current_line = line;
}
void Compiler::set_column(const column_t column)
{
    this->program->memory->columns[this->program->memory->code.size()] = column;
    this->current_column = column;
}

#undef ADD_LOG
