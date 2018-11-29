/**
 * |----------------------------------|
 * | Nuua Stack Based Virtual Machine |
 * |----------------------------------|
 *
 * Copyright 2018 Erik Campobadal <soc@erik.cat>
 * https://nuua.io
 */

#include "../include/virtual_machine.hpp"
#include "../include/operation.hpp"
#include "../../Compiler/include/compiler.hpp"
#include "../../Logger/include/logger.hpp"

#define BINARY_POP() Value *b = this->pop(); Value *a = this->pop()

void VirtualMachine::push(Value *value)
{
    this->stack.push_back(value);
}

Value *VirtualMachine::pop()
{
    Value *value = this->stack.back();
    this->stack.pop_back();

    return value;
}

uint64_t VirtualMachine::read_instruction()
{
    return *this->program_counter++;
}

Value *VirtualMachine::read_constant()
{
    auto con = this->get_current_memory()->constants.at(this->read_instruction());
    auto result = new Value( *con );
    return result;
}

int VirtualMachine::read_integer()
{
    return static_cast<int>(this->read_constant()->nvalue);
}

std::string VirtualMachine::read_variable()
{
    return *this->read_constant()->svalue;
}

void VirtualMachine::do_list()
{
    std::vector<Value *> v;
    for (int pops = this->read_integer(); pops > 0; pops--) v.push_back(this->pop());
    this->push(new Value(v));
}

void VirtualMachine::do_dictionary()
{
    std::unordered_map<std::string, Value *> dictionary;
    std::vector<std::string> key_order;
    for (int e = this->read_integer(); e > 0; e--) {
        auto val = this->pop();
        auto n = *this->pop()->svalue;
        dictionary[n] = val;
        key_order.push_back(n);
    }
    this->push(new Value(dictionary, key_order));
}

void VirtualMachine::do_access()
{
    auto var = this->frames.back().heap.at(this->read_variable());
    auto index = this->pop();
    if (var->type == VALUE_LIST && index->type == VALUE_NUMBER) {
        this->push(var->lvalues->at(index->nvalue));
    } else if (var->type == VALUE_DICTIONARY && index->type == VALUE_STRING) {
        this->push(var->dvalues->values.at(*index->svalue));
    } else {
        if (var->type == VALUE_DICTIONARY) {
            logger->error("Invalid access instruction. You need to use a string as a key", this->get_current_line());
            exit(EXIT_FAILURE);
        } else {
            logger->error("Invalid access instruction. You need to use a number as a key", this->get_current_line());
            exit(EXIT_FAILURE);
        }
    }
}

Memory *VirtualMachine::get_current_memory()
{
    switch (this->current_memory) {
        case FUNCTIONS_MEMORY: { return &this->program.functions; }
        case CLASSES_MEMORY: { return &this->program.classes; }
        default: { return &this->program.program; }
    }
}

uint32_t VirtualMachine::get_current_line()
{
    return this->get_current_memory()->lines.at(
        static_cast<uint64_t>(this->program_counter - &this->get_current_memory()->code.front())
    );
}

void VirtualMachine::run()
{
    for (uint64_t instruction;;) {
        instruction = this->read_instruction();
        printf("=> %s\n", opcode_to_string(instruction).c_str());
        switch (instruction) {
            case OP_CONSTANT: { this->push(this->read_constant()); break; }
            case OP_MINUS: { this->push(Operation::iminus(this->pop())); break; }
            case OP_NOT: { this->push(Operation::inot(this->pop())); break; }
            case OP_ADD: { BINARY_POP(); this->push(Operation::iadd(a, b)); break; }
            case OP_SUB: { BINARY_POP(); this->push(Operation::isub(a, b)); break; }
            case OP_MUL: { BINARY_POP(); this->push(Operation::imul(a, b)); break; }
            case OP_DIV: { BINARY_POP(); this->push(Operation::idiv(a, b)); break; }
            case OP_EQ: { BINARY_POP(); this->push(Operation::ieq(a, b)); break; }
            case OP_NEQ: { BINARY_POP(); this->push(Operation::ineq(a, b)); break; }
            case OP_LT: { BINARY_POP(); this->push(Operation::ilt(a, b)); break; }
            case OP_LTE: { BINARY_POP(); this->push(Operation::ilte(a, b)); break; }
            case OP_HT: { BINARY_POP(); this->push(Operation::iht(a, b)); break; }
            case OP_HTE: { BINARY_POP(); this->push(Operation::ihte(a, b)); break; }
            // case OP_JUMP: { this->program_counter = &this->get_current_memory()->code.front() + (this->read_integer() - 1); break; }
            case OP_RJUMP: { this->program_counter += this->read_integer() - 1; break; }
            case OP_BRANCH_TRUE: { auto to = this->read_integer() - 1; if (this->pop()->to_bool()) this->program_counter += to; break; }
            case OP_BRANCH_FALSE: { auto to = this->read_integer() - 1; if (!this->pop()->to_bool()) this->program_counter += to; break; }
            case OP_STORE: { this->frames.back().heap[this->read_variable()] = this->pop(); break; }
            case OP_STORE_ACCESS: { BINARY_POP(); (*this->frames.back().heap.at(this->read_variable())->lvalues)[b->nvalue] = a; break; }
            case OP_LOAD: { this->push(new Value(*this->frames.back().heap.at(this->read_variable()))); break; }
            case OP_LIST: { this->do_list(); break; }
            case OP_DICTIONARY: { this->do_dictionary(); break; }
            case OP_ACCESS: { this->do_access(); break; }
            case OP_FUNCTION: { this->push(new Value(this->read_integer(), new Frame(this->frames.back()))); break; }
            case OP_CALL: { break; }
            case OP_LEN: { this->push(Operation::ilen(this->pop())); break; }
            case OP_PRINT: { this->pop()->println(); break; }
            case OP_EXIT: { for (auto value : this->stack) value->println(); return; }
            default: { logger->error("Unknown instruction at line", this->get_current_line()); exit(EXIT_FAILURE); break; }
        }
    }
}


void VirtualMachine::interpret(const char *source)
{
    auto compiler = new Compiler;
    this->program = compiler->compile(source);
    delete compiler;

    logger->info("Started interpreting...");

    if (this->program.program.code.size() > 0) {
        this->program_counter = &this->program.program.code[0];
        this->run();
    }

    logger->success("Finished interpreting");
}

void VirtualMachine::reset()
{
    this->program.reset();
}

#undef BINARY_POP
