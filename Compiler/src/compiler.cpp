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
#define SET_SOURCE_LOCATION(node) \
    if (this->current_column != node->column) this->set_column(node->column); \
    if (this->current_line != node->line) this->set_line(node->line); \
    if (!this->current_file || *this->current_file != *node->file) this->set_file(node->file);

// Class constant pool.
static std::unordered_map<
    // The class.
    std::shared_ptr<Class>,
    // Variable name and constant index.
    std::unordered_map<std::string, size_t>
> class_constant_pool;

reg_t Compiler::compile(const char *file)
{
    Analyzer analyzer = Analyzer(file);
    std::shared_ptr<std::vector<std::shared_ptr<Statement>>>code = std::make_shared<std::vector<std::shared_ptr<Statement>>>();
    std::shared_ptr<Block> block = analyzer.analyze(code);
    // Register the TLDs.
    this->register_tld(code, block);
    // Allocate the main registers.
    this->program->main_frame.allocate_registers(this->global.current_register);
    // Compile the code.
    this->compile_module(code, block);
    // Add the exit opcode.
    this->add_opcodes({{ OP_EXIT }});
    // Dump the program opcodes to the stdout.
    if (logger->show_opcodes) this->program->memory->dump();
    return block->get_variable("main")->reg;
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
                std::shared_ptr<Class> c = std::static_pointer_cast<Class>(node);
                for (const std::shared_ptr<Statement> &el : c->body) {
                    switch (el->rule) {
                        case RULE_FUNCTION: {
                            const std::shared_ptr<Function> &fun = std::static_pointer_cast<Function>(el);
                            this->compile_function(fun).copy_to(
                                &this->program->memory->constants[class_constant_pool[c][fun->value->name]]
                            );
                            break;
                        }
                        default: { /* Do nothing */ }
                    }
                }
                break;
            }
            case RULE_FUNCTION: {
                std::shared_ptr<Function> fun = std::static_pointer_cast<Function>(node);
                this->compile_function(fun).copy_to(
                    this->program->main_frame.registers.get() + block->get_variable(fun->value->name)->reg
                );
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
        if (std::find(compiled_modules.begin(), compiled_modules.end(), use->code) == compiled_modules.end()) {
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
                std::shared_ptr<Class> c = std::static_pointer_cast<Class>(tld);
                for (auto &[vn, vt] : c->block->variables) {
                    // Set the class register.
                    vt.reg = this->local.get_register(true);
                    // Set the constant register.
                    class_constant_pool[c][vn] = this->add_constant({ vt.type });
                }
                this->local.reset();
                break;
            }
            case RULE_FUNCTION: {
                std::shared_ptr<FunctionValue> fun = std::static_pointer_cast<Function>(tld)->value;
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
            // Check if it's a variable.
            BlockVariableType *var = use->block->get_variable(name);
            if (var) block->get_variable(name)->reg = var->reg;
        }
    }
    // block->debug();
}

void Compiler::compile(const std::shared_ptr<Statement> &rule)
{
    // Check if the local registers were reset.
    if (this->local.current_register == 0 && this->dead_variables.size() > 0) {
        this->dead_variables.clear();
    }
    // Free the registers of the dead variables.
    while (this->dead_variables.size() > 0) {
        this->local.free_register(this->dead_variables.back(), true);
        this->dead_variables.pop_back();
    }
    switch (rule->rule) {
        case RULE_PRINT: {
            std::shared_ptr<Print> print = std::static_pointer_cast<Print>(rule);
            if (this->is_constant(print->expression)) {
                SET_SOURCE_LOCATION(rule);
                this->add_opcodes({{ OP_PRINT_C }});
                this->compile(print->expression, false);
            } else {
                reg_t rx = this->compile(print->expression);
                SET_SOURCE_LOCATION(rule);
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
            reg_t rx = this->local.get_register(true);
            BlockVariableType *var = this->get_variable(dec->name).first;
            // Set the variable register in the current block.
            var->reg = rx;
            if (dec->initializer) {
                if (this->is_constant(dec->initializer)) {
                    SET_SOURCE_LOCATION(rule);
                    this->add_opcodes({{ OP_LOAD_C, rx }});
                    this->compile(dec->initializer, false);
                } else this->compile(dec->initializer, true, &rx);
            } else {
                SET_SOURCE_LOCATION(rule);
                this->add_opcodes({{ OP_LOAD_C, rx, this->add_constant({ dec->type }) }});
            }
            // Check if the variable is ever used.
            if (!var->last_use) {
                this->dead_variables.push_back(var->reg);
            }
            break;
        }
        case RULE_RETURN: {
            std::shared_ptr<Return> ret = std::static_pointer_cast<Return>(rule);
            if (ret->value) {
                if (this->is_constant(ret->value)) {
                    SET_SOURCE_LOCATION(rule);
                    this->add_opcodes({{ OP_PUSH_C }});
                    this->compile(ret->value, false);
                } else {
                    reg_t res = this->compile(ret->value);
                    SET_SOURCE_LOCATION(rule);
                    this->add_opcodes({{ OP_PUSH, res }});
                    this->local.free_register(res);
                }
            }
            SET_SOURCE_LOCATION(rule);
            this->add_opcodes({{ OP_RETURN }});
            break;
        }
        case RULE_DELETE: {
            std::shared_ptr<Delete> del = std::static_pointer_cast<Delete>(rule);
            this->compile(del->target, true, nullptr, std::shared_ptr<Expression>(), nullptr, true);
            break;
        }
        case RULE_IF: {
            std::shared_ptr<If> rif = std::static_pointer_cast<If>(rule);
            reg_t rx = this->compile(rif->condition);
            SET_SOURCE_LOCATION(rule);
            this->add_opcodes({{ OP_CFNJUMP, 0, rx }});
            // Save the jump index
            size_t jump_index = this->program->memory->code.size() - 2; // The 0 on the opcode above.
            // Set the then block.
            this->blocks.push_back(rif->then_block);
            // Compile the if then branch.
            for (const std::shared_ptr<Statement> &stmt : rif->then_branch) this->compile(stmt);
            // Remove the then block.
            this->blocks.pop_back();
            if (rif->else_branch.size() > 0) {
                // Add a jump to the then branch to avoid going to else.
                SET_SOURCE_LOCATION(rule);
                this->add_opcodes({{ OP_FJUMP, 0 }});
                size_t then_jump = this->program->memory->code.size() - 1;
                // Change the initial jump index to here.
                this->program->memory->code[jump_index] = this->program->memory->code.size() - (jump_index - 1);
                // Set the else block.
                this->blocks.push_back(rif->else_block);
                for (const std::shared_ptr<Statement> &stmt : rif->else_branch) this->compile(stmt);
                // Remove the else block.
                this->blocks.pop_back();
                this->program->memory->code[then_jump] = this->program->memory->code.size() - (then_jump - 1);
            } else this->program->memory->code[jump_index] = this->program->memory->code.size() - (jump_index - 1);
            break;
        }
        case RULE_WHILE: {
            std::shared_ptr<While> rwhile = std::static_pointer_cast<While>(rule);
            // Save the initial point.
            size_t initial_index = this->program->memory->code.size();
            // Compile the condition.
            reg_t rx = this->compile(rwhile->condition);
            // Perform the jump if nessesary.
            SET_SOURCE_LOCATION(rule);
            this->add_opcodes({{ OP_CFNJUMP, 0, rx }});
            this->local.free_register(rx);
            // Get the jump index for further modification.
            size_t jump_index = this->program->memory->code.size() - 2;
            // Set the while block.
            this->blocks.push_back(rwhile->block);
            // Compile the while body.
            for (const std::shared_ptr<Statement> &stmt : rwhile->body) this->compile(stmt);
            // Set the jump back up.
            size_t jb = this->program->memory->code.size() - initial_index;
            SET_SOURCE_LOCATION(rule);
            this->add_opcodes({{ OP_BJUMP, jb }});
            // Modify the branch jump.
            this->program->memory->code[jump_index] = this->program->memory->code.size() - (jump_index - 1);
            // Pop the for block.
            this->blocks.pop_back();
            break;
        }
        case RULE_FOR: {
            std::shared_ptr<For> rfor = std::static_pointer_cast<For>(rule);
            bool string_index = rfor->type->type == VALUE_DICT;
            // Current index.
            reg_t ri = this->local.get_register(true);
            reg_t ris = 0; // String variation for dictionaries.
            if (string_index) ris = this->local.get_register(true);
            // Loop variable.
            reg_t re = this->local.get_register(true);
            // Set the initial index.
            SET_SOURCE_LOCATION(rule);
            this->add_opcodes({{ OP_LOAD_C, ri, this->add_constant({ static_cast<nint_t>(0) }) }});
            // Save the initial point.
            size_t initial_index = this->program->memory->code.size();
            // Setup the for condition.
            reg_t rc = this->compile(rfor->iterator);
            // Set the for block.
            this->blocks.push_back(rfor->block);
            // Set the used registers in the block.
            this->get_variable(rfor->variable).first->reg = re;
            if (rfor->index != "" && string_index) {
                this->get_variable(rfor->index).first->reg = ris;
            }
            else if (rfor->index != "") {
                this->get_variable(rfor->index).first->reg = ri;
            }
            // Store the length of the iterator.
            reg_t r1 = this->local.get_register();
            SET_SOURCE_LOCATION(rule);
            switch (rfor->type->type) {
                case VALUE_STRING: { this->add_opcodes({{ OP_CAST_STRING_INT, r1, rc }}); break; }
                case VALUE_LIST: { this->add_opcodes({{ OP_CAST_LIST_INT, r1, rc }}); break; }
                case VALUE_DICT: { this->add_opcodes({{ OP_CAST_DICT_INT, r1, rc }}); break; }
                default: { ADD_LOG(rfor->iterator, "Invalid iterator type to compile: '" + rfor->type->to_string() + "'"); exit(logger->crash()); }
            }
            // Add the loop condition.
            reg_t rx = this->local.get_register(); // Loop condition.
            SET_SOURCE_LOCATION(rule);
            this->add_opcodes({{ OP_LT_INT, rx, ri, r1 }});
            this->local.free_register(r1);
            // Perform the jump if nessesary.
            SET_SOURCE_LOCATION(rule);
            this->add_opcodes({{ OP_CFNJUMP, 0, rx }});
            this->local.free_register(rx);
            // Get the jump index for further modification.
            size_t jump_index = this->program->memory->code.size() - 2;
            // Set the index variable if dictionary.
            if (string_index) {
                SET_SOURCE_LOCATION(rule);
                this->add_opcodes({{ OP_DKEY, ris, rc, ri }});
            }
            // Set the loop variable/s.
            SET_SOURCE_LOCATION(rule);
            switch (rfor->type->type) {
                case VALUE_STRING: { this->add_opcodes({{ OP_SGET, re, rc, ri }}); break; }
                case VALUE_LIST: { this->add_opcodes({{ OP_LGET, re, rc, ri }}); break; }
                case VALUE_DICT: { this->add_opcodes({{ OP_DGET, re, rc, ris }}); break; }
                default: { ADD_LOG(rfor->iterator, "Invalid iterator type to compile: '" + rfor->type->to_string() + "'"); exit(logger->crash()); }
            }
            // Compile the while body.
            for (const std::shared_ptr<Statement> &stmt : rfor->body) this->compile(stmt);
            // Increment the current index.
            SET_SOURCE_LOCATION(rule);
            this->add_opcodes({{ OP_IINC, ri }});
            // Set the jump back up.
            size_t jb = this->program->memory->code.size() - initial_index;
            SET_SOURCE_LOCATION(rule);
            this->add_opcodes({{ OP_BJUMP, jb }});
            // Modify the branch jump.
            // this->program->memory->code[jump_index] = (this->program->memory->code.size() - 1) - (jump_index + 1);
            this->program->memory->code[jump_index] = this->program->memory->code.size() - (jump_index - 1);
            // Remove the variables used.
            this->local.free_register(ri, true);
            if (string_index) this->local.free_register(ris, true);
            this->local.free_register(re, true);
            this->local.free_register(rc);
            // Pop the for block.
            this->blocks.pop_back();
            break;
        }
        default: {
            ADD_LOG(rule, "Compilation error: Invalid statement to compile");
            exit(logger->crash());
        }
    }
}

reg_t Compiler::compile(
    const std::shared_ptr<Expression> &rule,
    const bool load_constant,
    const reg_t *suggested_register,
    const std::shared_ptr<Expression> &assignment_value,
    reg_t *object_reg,
    const bool delete_access
) {
    reg_t result = 0;
    switch (rule->rule) {
        case RULE_INTEGER: {
            SET_SOURCE_LOCATION(rule);
            if (load_constant) {
                SET_SOURCE_LOCATION(rule);
                this->add_opcodes({{ OP_LOAD_C, result = suggested_register ? *suggested_register : this->local.get_register() }});
            }
            this->add_opcodes({{ this->add_constant({ std::static_pointer_cast<Integer>(rule)->value }) }});
            break;
        }
        case RULE_FLOAT: {
            if (load_constant) {
                SET_SOURCE_LOCATION(rule);
                this->add_opcodes({{ OP_LOAD_C, result = suggested_register ? *suggested_register : this->local.get_register() }});
            }
            this->add_opcodes({{ this->add_constant({ std::static_pointer_cast<Float>(rule)->value }) }});
            break;
        }
        case RULE_STRING: {
            if (load_constant) {
                SET_SOURCE_LOCATION(rule);
                this->add_opcodes({{ OP_LOAD_C, result = suggested_register ? *suggested_register : this->local.get_register() }});
            }
            this->add_opcodes({{ this->add_constant({ std::static_pointer_cast<String>(rule)->value }) }});
            break;
        }
        case RULE_BOOLEAN: {
            if (load_constant) {
                SET_SOURCE_LOCATION(rule);
                this->add_opcodes({{ OP_LOAD_C, result = suggested_register ? *suggested_register : this->local.get_register() }});
            }
            this->add_opcodes({{ this->add_constant({ std::static_pointer_cast<Boolean>(rule)->value }) }});
            break;
        }
        case RULE_LIST: {
            std::shared_ptr<List> list = std::static_pointer_cast<List>(rule);
            if (this->is_constant(list)) {
                // The list can be stored in the constant pool.
                if (load_constant) {
                    SET_SOURCE_LOCATION(rule);
                    this->add_opcodes({{ OP_LOAD_C, result = suggested_register ? *suggested_register : this->local.get_register() }});
                }
                Value v = Value(list->type);
                this->constant_list(list, v);
                this->add_opcodes({{ this->add_constant(v) }});
            } else {
                result = suggested_register ? *suggested_register : this->local.get_register();
                // The list needs to be constructed from the groud up
                this->add_opcodes({{ OP_LOAD_C, result, this->add_constant({ list->type }) }});
                for (const std::shared_ptr<Expression> &e : list->value) {
                    if (this->is_constant(e)) {
                        SET_SOURCE_LOCATION(rule);
                        this->add_opcodes({{ OP_LPUSH_C, result }});
                        this->compile(e, false);
                    } else {
                        reg_t ry = this->compile(e);
                        SET_SOURCE_LOCATION(rule);
                        this->add_opcodes({{ OP_LPUSH, result, ry }});
                        this->local.free_register(ry);
                    }
                }
            }
            break;
        }
        case RULE_DICTIONARY: {
            std::shared_ptr<Dictionary> dict = std::static_pointer_cast<Dictionary>(rule);
            if (this->is_constant(dict)) {
                // The list can be stored in the constant pool.
                if (load_constant) {
                    SET_SOURCE_LOCATION(rule);
                    this->add_opcodes({{ OP_LOAD_C, result = suggested_register ? *suggested_register : this->local.get_register() }});
                }
                Value v = dict->type;
                this->constant_dict(dict, v);
                this->add_opcodes({{ this->add_constant(v) }});
            } else {
                result = suggested_register ? *suggested_register : this->local.get_register();
                // The list needs to be constructed from the groud up
                SET_SOURCE_LOCATION(rule);
                this->add_opcodes({{ OP_LOAD_C, result, this->add_constant({ dict->type }) }});
                for (const auto &[key, value] : dict->value) {
                    size_t key_reg = this->add_constant({ key });
                    if (this->is_constant(value)) {
                        SET_SOURCE_LOCATION(rule);
                        this->add_opcodes({{ OP_DSET, result, key_reg }});
                        this->compile(value, false);
                    } else {
                        reg_t ry = this->compile(value);
                        SET_SOURCE_LOCATION(rule);
                        this->add_opcodes({{ OP_DSET, result, key_reg, ry }});
                        this->local.free_register(ry);
                    }
                }
            }
            break;
        }
        case RULE_OBJECT: {
            std::shared_ptr<Object> object = std::static_pointer_cast<Object>(rule);
            if (load_constant) {
                SET_SOURCE_LOCATION(rule);
                this->add_opcodes({{ OP_LOAD_C, result = suggested_register ? *suggested_register : this->local.get_register() }});
            }
            // Create the constant object.
            std::vector<std::string> props;
            for (const auto &[name, type] : object->c->block->variables) props.push_back(name);
            size_t objr = this->add_constant({ object->c->name, props });
            this->add_opcodes({{ objr }});
            // Assign each register its corresponding values.
            for (const auto &[name, type] : object->c->block->variables) {
                reg_t val;
                // Check if the value is part of the init arguments.
                for (const auto &[n, e] : object->arguments) {
                    if (n == name) {
                        val = this->compile(e);
                        goto object_add_prop;
                    }
                }
                SET_SOURCE_LOCATION(rule);
                this->add_opcodes({{ OP_LOAD_C, val = this->local.get_register(), class_constant_pool[object->c][name] }});
                object_add_prop:
                SET_SOURCE_LOCATION(rule);
                this->add_opcodes({{ OP_SPROP, type.reg, result, val }});
                this->local.free_register(val);
            }
            break;
        }
        case RULE_GROUP: {
            result = this->compile(std::static_pointer_cast<Group>(rule)->expression, load_constant, suggested_register);
            break;
        }
        case RULE_CAST: {
            std::shared_ptr<Cast> cast = std::static_pointer_cast<Cast>(rule);
            opcode_t base = OP_CAST_INT_FLOAT;
            reg_t rx = this->compile(cast->expression);
            SET_SOURCE_LOCATION(rule);
            this->add_opcodes({{ base + cast->cast_type, result = suggested_register ? *suggested_register : this->local.get_register(), rx }});
            this->local.free_register(rx);
            break;
        }
        case RULE_UNARY: {
            std::shared_ptr<Unary> unary = std::static_pointer_cast<Unary>(rule);
            opcode_t base = OP_NEG_BOOL;
            reg_t rx = this->compile(unary->right);
            SET_SOURCE_LOCATION(rule);
            this->add_opcodes({{ base + unary->type, result = suggested_register ? *suggested_register : this->local.get_register(), rx }});
            this->local.free_register(rx);
            break;
        }
        case RULE_BINARY: {
            std::shared_ptr<Binary> binary = std::static_pointer_cast<Binary>(rule);
            opcode_t base = OP_ADD_INT;
            reg_t ry = this->compile(binary->left);
            reg_t rz = this->compile(binary->right);
            // Use a suggested register if needed.
            if (suggested_register) result = *suggested_register;
            // Check if there are some dead variables to use.
            else if (this->dead_variables.size() > 0) {
                this->local.free_register(this->dead_variables.back(), true);
                this->dead_variables.pop_back();
                result = this->local.get_register();
            }
            // Otherwise get a new register.
            else result = this->local.get_register();
            SET_SOURCE_LOCATION(rule);
            this->add_opcodes({{ base + binary->type, result, ry, rz }});
            this->local.free_register(ry);
            this->local.free_register(rz);
            break;
        }
        case RULE_VARIABLE: {
            // std::pair<BlockVariableType *, bool> var = this->get_variable(std::static_pointer_cast<Variable>(rule)->name);
            const auto [variable, is_global] = this->get_variable(std::static_pointer_cast<Variable>(rule)->name);
            if (is_global) {
                // The variable is global, and needs to be loaded first.
                result = suggested_register ? *suggested_register : this->local.get_register();
                SET_SOURCE_LOCATION(rule);
                this->add_opcodes({{ OP_LOAD_G, result, variable->reg }});
            } else {
                if (suggested_register) {
                    SET_SOURCE_LOCATION(rule);
                    this->add_opcodes({{ OP_MOVE, *suggested_register, variable->reg }});
                } else result = variable->reg;
            }
            // Check if the variable is last used.
            if (variable->last_use.get() == std::static_pointer_cast<Node>(rule).get()) {
                // This is the last time the variable is needed.
                this->dead_variables.push_back(variable->reg);
            }
            break;
        }
        case RULE_ASSIGN: {
            std::shared_ptr<Assign> assign = std::static_pointer_cast<Assign>(rule);
            // Compile the value of the assignment
            switch (assign->type) {
                case ASSIGN_VALUE: {
                    result = this->compile(assign->value);
                    // Compile the target.
                    reg_t target = this->compile(assign->target);
                    // Check if it's a global variable.
                    if (assign->target->rule == RULE_VARIABLE) {
                        // Check if the assignment is to a global variable
                        const auto [variable, is_global] = this->get_variable(std::static_pointer_cast<Variable>(assign->target)->name);
                        if (is_global) {
                            // Assign it to it.
                            SET_SOURCE_LOCATION(rule);
                            this->add_opcodes({{ OP_SET_G, target /* variable->reg */ , result }});
                            goto analyzer_assign_finished;
                        }
                    }
                    SET_SOURCE_LOCATION(rule);
                    this->add_opcodes({{ OP_MOVE, target, result }});
                    analyzer_assign_finished:
                    this->local.free_register(target);
                    break;
                }
                case ASSIGN_ACCESS:
                case ASSIGN_PROP:
                case ASSIGN_PROP_ACCESS: {
                    result = this->compile(assign->target, true, nullptr, assign->value);
                    break;
                }
            }
            break;
        }
        case RULE_LOGICAL: {
            std::shared_ptr<Logical> logical = std::static_pointer_cast<Logical>(rule);
            result = suggested_register ? *suggested_register : this->local.get_register();
            reg_t ry = this->compile(logical->left);
            reg_t rz = this->compile(logical->right);
            switch (logical->op.type) {
                case TOKEN_OR: {
                    SET_SOURCE_LOCATION(rule);
                    this->add_opcodes({{ OP_OR, result, ry, rz }});
                    break;
                }
                case TOKEN_AND: {
                    SET_SOURCE_LOCATION(rule);
                    this->add_opcodes({{ OP_AND, result, ry, rz }});
                    break;
                }
                default: {
                    ADD_LOG(logical, "Invalid logical type to compile.");
                    exit(logger->crash());
                }
            }
            this->local.free_register(ry);
            this->local.free_register(rz);
            break;
        }
        case RULE_CALL: {
            std::shared_ptr<Call> call = std::static_pointer_cast<Call>(rule);
            reg_t target;
            if (call->is_method) {
                reg_t objr;
                target = this->compile(call->target, true, nullptr, std::shared_ptr<Expression>(), &objr);
                SET_SOURCE_LOCATION(rule);
                this->add_opcodes({{ OP_PUSH, objr }});
                this->local.free_register(objr);
            } else {
                target = this->compile(call->target);
            }
            // Push the function arguments.
            for (const std::shared_ptr<Expression> &arg : call->arguments) {
                if (this->is_constant(arg)) {
                    SET_SOURCE_LOCATION(rule);
                    this->add_opcodes({{ OP_PUSH_C }});
                    this->compile(arg, false);
                } else {
                    reg_t rx = this->compile(arg);
                    SET_SOURCE_LOCATION(rule);
                    this->add_opcodes({{ OP_PUSH, rx }});
                    this->local.free_register(rx);
                }
            }
            SET_SOURCE_LOCATION(rule);
            this->add_opcodes({{ OP_CALL, target }});
            this->local.free_register(target);
            if (call->has_return) {
                // Pop the return value from the stack into the return register.
                SET_SOURCE_LOCATION(rule);
                this->add_opcodes({{ OP_POP, result = suggested_register ? *suggested_register : this->local.get_register() }});
            }
            break;
        }
        case RULE_ACCESS: {
            std::shared_ptr<Access> access = std::static_pointer_cast<Access>(rule);
            reg_t index = this->compile(access->index);
            reg_t target;
            if (delete_access) {
                reg_t objr;
                if (access->target->rule == RULE_PROPERTY) {
                    target = this->compile(access->target, true, nullptr, std::shared_ptr<Expression>(), &objr);
                } else {
                    target = this->compile(access->target);
                }
                SET_SOURCE_LOCATION(rule);
                switch (access->type) {
                    case ACCESS_STRING: {
                        this->add_opcodes({{ OP_SDELETE, target, index }});
                        break;
                    }
                    case ACCESS_LIST: {
                        this->add_opcodes({{ OP_LDELETE, target, index }});
                        break;
                    }
                    case ACCESS_DICT: {
                        this->add_opcodes({{ OP_DDELETE, target, index }});
                        break;
                    }
                }
                if (access->target->rule == RULE_PROPERTY) {
                    const std::shared_ptr<Property> &prop = std::static_pointer_cast<Property>(access->target);
                    // Re-assign the result to the prop.
                    SET_SOURCE_LOCATION(rule);
                    this->add_opcodes({{
                        OP_SPROP, prop->c->block->get_variable(prop->name)->reg,
                        objr, target,
                    }});
                    this->local.free_register(objr);
                }
            } else if (assignment_value) {
                // The value needs to be compiled.
                reg_t objr;
                if (access->target->rule == RULE_PROPERTY) {
                    target = this->compile(access->target, true, nullptr, std::shared_ptr<Expression>(), &objr);
                } else {
                    target = this->compile(access->target);
                }
                result = this->compile(assignment_value);
                SET_SOURCE_LOCATION(rule);
                switch (access->type) {
                    case ACCESS_STRING: {
                        this->add_opcodes({{ OP_SSET, target, index, result }});
                        break;
                    }
                    case ACCESS_LIST: {
                        this->add_opcodes({{ OP_LSET, target, index, result }});
                        break;
                    }
                    case ACCESS_DICT: {
                        this->add_opcodes({{ OP_DSET, target, index, result }});
                        break;
                    }
                }
                if (access->target->rule == RULE_PROPERTY) {
                    const std::shared_ptr<Property> &prop = std::static_pointer_cast<Property>(access->target);
                    // Re-assign the result to the prop.
                    SET_SOURCE_LOCATION(rule);
                    this->add_opcodes({{
                        OP_SPROP, prop->c->block->get_variable(prop->name)->reg,
                        objr, target,
                    }});
                    this->local.free_register(objr);
                }
            } else {
                target = this->compile(access->target);
                SET_SOURCE_LOCATION(rule);
                switch (access->type) {
                    case ACCESS_STRING: {
                        this->add_opcodes({{ OP_SGET, result = suggested_register ? *suggested_register : this->local.get_register(), target, index }});
                        break;
                    }
                    case ACCESS_LIST: {
                        this->add_opcodes({{ OP_LGET, result = suggested_register ? *suggested_register : this->local.get_register(), target, index }});
                        break;
                    }
                    case ACCESS_DICT: {
                        this->add_opcodes({{ OP_DGET, result = suggested_register ? *suggested_register : this->local.get_register(), target, index }});
                        break;
                    }
                }
            }
            this->local.free_register(target);
            this->local.free_register(index);
            break;
        }
        case RULE_SLICE: {
            std::shared_ptr<Slice> slice = std::static_pointer_cast<Slice>(rule);
            reg_t r1;
            if (slice->start) r1 = this->compile(slice->start);
            else {
                r1 = this->local.get_register();
                SET_SOURCE_LOCATION(rule);
                this->add_opcodes({{ OP_LOAD_C, r1, this->add_constant({ static_cast<nint_t>(0) }) }});
            }
            reg_t r2 = slice->end ? this->compile(slice->end) : 0;
            reg_t r3;
            if (slice->step) r3 = this->compile(slice->step);
            else {
                r3 = this->local.get_register();
                SET_SOURCE_LOCATION(rule);
                this->add_opcodes({{ OP_LOAD_C, r3, this->add_constant({ static_cast<nint_t>(1) }) }});
            }
            reg_t rx = this->compile(slice->target);
            if (slice->end) {
                SET_SOURCE_LOCATION(rule);
                this->add_opcodes({{
                    slice->is_list ? OP_LSLICE : OP_SSLICE,
                    result = suggested_register ? *suggested_register : this->local.get_register(),
                    rx, r1, r2, r3
                }});
            } else {
                SET_SOURCE_LOCATION(rule);
                this->add_opcodes({{
                slice->is_list ? OP_LSLICEE : OP_SSLICEE,
                result = suggested_register ? *suggested_register : this->local.get_register(),
                rx, r1, r3
            }});
            }
            this->local.free_register(r1);
            this->local.free_register(r2);
            this->local.free_register(r3);
            this->local.free_register(rx);
            break;
        }
        case RULE_RANGE: {
            std::shared_ptr<Range> range = std::static_pointer_cast<Range>(rule);
            reg_t r1 = this->compile(range->start);
            reg_t r2 = this->compile(range->end);
            SET_SOURCE_LOCATION(rule);
            this->add_opcodes({{
                range->inclusive ? OP_RANGEI : OP_RANGEE,
                result = suggested_register ? *suggested_register : this->local.get_register(),
                r1, r2
            }});
            this->local.free_register(r1);
            this->local.free_register(r2);
            break;
        }
        case RULE_PROPERTY: {
            std::shared_ptr<Property> prop = std::static_pointer_cast<Property>(rule);
            // Compile the object.
            reg_t ry = this->compile(prop->object);
            if (assignment_value) {
                // The value needs to be compiled.
                result = this->compile(assignment_value);
                SET_SOURCE_LOCATION(rule);
                this->add_opcodes({{
                    OP_SPROP, prop->c->block->get_variable(prop->name)->reg,
                    ry, result,
                }});
            } else {
                SET_SOURCE_LOCATION(rule);
                this->add_opcodes({{
                    OP_LPROP, result = suggested_register ? *suggested_register : this->local.get_register(),
                    ry, prop->c->block->get_variable(prop->name)->reg
                }});
            }
            if (object_reg) *object_reg = ry;
            else this->local.free_register(ry);
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
    for (size_t i = this->blocks.size() - 1;; i--) {
        BlockVariableType *var = this->blocks[i]->get_variable(name);
        if (var) return { var, i == 0 };
        else if (i == 0) return { nullptr, false };
    }

    return { nullptr, false }; // Compiler warning... Totally useless.
}

BlockClassType *Compiler::get_class(const std::string &name)
{
    for (size_t i = this->blocks.size() - 1;; i--) {
        BlockClassType *c = this->blocks[i]->get_class(name);
        if (c) return c;
        else if (i == 0) return nullptr;
    }

    return nullptr; // Compiler warning... Totally useless.
}

void Compiler::add_opcodes(const std::vector<opcode_t> &opcodes)
{
    for (const opcode_t &op : opcodes) this->program->memory->code.push_back(op);
}

size_t Compiler::add_constant(const Value &value)
{
    this->program->memory->constants.push_back(std::move(value));
    return this->program->memory->constants.size() - 1;
}

void Compiler::constant_list(const std::shared_ptr<List> &list, Value &dest)
{
    for (const std::shared_ptr<Expression> &value : list->value) {
        switch (value->rule) {
            case RULE_INTEGER: { std::get<std::shared_ptr<nlist_t>>(dest.value)->push_back({ std::static_pointer_cast<Integer>(value)->value }); break; }
            case RULE_FLOAT: { std::get<std::shared_ptr<nlist_t>>(dest.value)->push_back({ std::static_pointer_cast<Float>(value)->value }); break; }
            case RULE_BOOLEAN: { std::get<std::shared_ptr<nlist_t>>(dest.value)->push_back({ std::static_pointer_cast<Boolean>(value)->value }); break; }
            case RULE_STRING:  { std::get<std::shared_ptr<nlist_t>>(dest.value)->push_back({ std::static_pointer_cast<String>(value)->value }); break; }
            case RULE_LIST: {
                Value v = Value(std::static_pointer_cast<List>(value)->type);
                this->constant_list(std::static_pointer_cast<List>(value), v);
                std::get<std::shared_ptr<nlist_t>>(dest.value)->push_back(std::move(v));
                break;
            }
            case RULE_DICTIONARY: {
                Value v = Value(std::static_pointer_cast<Dictionary>(value)->type);
                this->constant_dict(std::static_pointer_cast<Dictionary>(value), v);
                std::get<std::shared_ptr<nlist_t>>(dest.value)->push_back(std::move(v));
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
            case RULE_INTEGER: { std::get<std::shared_ptr<ndict_t>>(dest.value)->insert(key, { std::static_pointer_cast<Integer>(value)->value }); break; }
            case RULE_FLOAT: { std::get<std::shared_ptr<ndict_t>>(dest.value)->insert(key, { std::static_pointer_cast<Float>(value)->value }); break; }
            case RULE_BOOLEAN: { std::get<std::shared_ptr<ndict_t>>(dest.value)->insert(key, { std::static_pointer_cast<Boolean>(value)->value }); break; }
            case RULE_STRING:  { std::get<std::shared_ptr<ndict_t>>(dest.value)->insert(key, { std::static_pointer_cast<String>(value)->value }); break; }
            case RULE_LIST: {
                Value v = Value(std::static_pointer_cast<List>(value)->type);
                this->constant_list(std::static_pointer_cast<List>(value), v);
                std::get<std::shared_ptr<ndict_t>>(dest.value)->insert(key, std::move(v));
                break;
            }
            case RULE_DICTIONARY: {
                Value v = Value(std::static_pointer_cast<Dictionary>(value)->type);
                this->constant_dict(std::static_pointer_cast<Dictionary>(value), v);
                std::get<std::shared_ptr<ndict_t>>(dest.value)->insert(key, std::move(v));
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

Value Compiler::compile_function(const std::shared_ptr<Function> &f)
{
    const std::shared_ptr<FunctionValue> &fun = f->value;
    // Get the entry point of the function.
    size_t entry = this->program->memory->code.size();
    // Push the function block.
    this->blocks.push_back(fun->block);
    // Pop the function parameters.
    if (fun->parameters.size() > 0) {
        for (size_t i = fun->parameters.size() - 1;; i--) {
            // Get the variable from the block.
            BlockVariableType *var = this->get_variable(fun->parameters[i]->name).first;
            // Assign the register to the parameter.
            var->reg = this->local.get_register(true);
            // Pop the parameter from the stack.
            SET_SOURCE_LOCATION(fun);
            this->add_opcodes({{ OP_POP, var->reg }});
            // Check if the variable is ever used.
            if (!var->last_use) this->dead_variables.push_back(var->reg);
            if (i == 0) break; // Don't change this.
        }
    }
    // Compile the function body.
    for (const std::shared_ptr<Statement> &statement : fun->body) this->compile(statement);
    // Clear the function body (so that the elements may be freed)
    fun->body.clear();
    this->blocks.pop_back();
    // Get the number of registers needed.
    registers_size_t regs = this->local.current_register;
    // Reset the local frame info.
    this->local.reset();
    // Create the function value and return it.
    Value v = Value(entry, regs, Type(f));
    return v;
}

#undef ADD_LOG
