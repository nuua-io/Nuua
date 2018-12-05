/**
 * |----------------------------------|
 * | Nuua Stack Based Virtual Machine |
 * |----------------------------------|
 *
 * Copyright 2018 Erik Campobadal <soc@erik.cat>
 * https://nuua.io
 */

#include "../include/virtual_machine.hpp"
#include "../../Compiler/include/compiler.hpp"
#include "../../Logger/include/logger.hpp"

#define PUSH(value) (*this->top_stack++ = value)
#define POP() (--this->top_stack)
#define BINARY_POP() Value *b = POP(); Value *a = POP()
#define READ_INSTRUCTION() (*this->program_counter++)
#define READ_CONSTANT() (this->get_current_memory()->constants.at(READ_INSTRUCTION()))
#define READ_INTEGER() (static_cast<int>(READ_CONSTANT().nvalue))
#define READ_VARIABLE() (*READ_CONSTANT().svalue)

void VirtualMachine::do_list()
{
    std::vector<Value> v;
    for (int pops = READ_INTEGER(); pops > 0; pops--) v.push_back(*POP());
    PUSH(Value(v));
}

void VirtualMachine::do_dictionary()
{
    std::unordered_map<std::string, Value> dictionary;
    std::vector<std::string> key_order;
    for (int e = READ_INTEGER(); e > 0; e--) {
        auto val = *POP();
        auto n = *POP()->svalue;
        dictionary[n] = val;
        key_order.push_back(n);
    }
    PUSH(Value(dictionary, key_order));
}

void VirtualMachine::do_access()
{
    auto var = this->top_frame->heap.at(READ_VARIABLE());
    auto index = POP();
    if (var.type == VALUE_LIST && index->type == VALUE_NUMBER) {
        PUSH(var.lvalues->at(index->nvalue));
    } else if (var.type == VALUE_DICTIONARY && index->type == VALUE_STRING) {
        PUSH(var.dvalues->values.at(*index->svalue));
    } else {
        if (var.type == VALUE_DICTIONARY) {
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
        instruction = READ_INSTRUCTION();
        #if DEBUG
            // printf("=> %s\n", opcode_to_string(instruction).c_str());
        #endif
        switch (instruction) {
            case OP_CONSTANT: { PUSH(READ_CONSTANT()); break; }
            case OP_MINUS: { PUSH(-*POP()); break; }
            case OP_NOT: { PUSH(!*POP()); break; }
            case OP_ADD: { BINARY_POP(); PUSH(*a + *b); break; }
            case OP_SUB: { BINARY_POP(); PUSH(*a - *b); break; }
            case OP_MUL: { BINARY_POP(); PUSH(*a * *b); break; }
            case OP_DIV: { BINARY_POP(); PUSH(*a / *b); break; }
            case OP_EQ: { BINARY_POP(); PUSH(*a == *b); break; }
            case OP_NEQ: { BINARY_POP(); PUSH(*a != *b); break; }
            case OP_LT: { BINARY_POP(); PUSH(*a < *b); break; }
            case OP_LTE: { BINARY_POP(); PUSH(*a <= *b); break; }
            case OP_HT: { BINARY_POP(); PUSH(*a > *b); break; }
            case OP_HTE: { BINARY_POP(); PUSH(*a >= *b); break; }
            // case OP_JUMP: { this->program_counter = &this->get_current_memory()->code.front() + (READ_INTEGER() - 1); break; }
            case OP_RJUMP: { this->program_counter += READ_INTEGER() - 1; break; }
            case OP_BRANCH_TRUE: { auto to = READ_INTEGER() - 1; if (POP()->to_bool()) this->program_counter += to; break; }
            case OP_BRANCH_FALSE: { auto to = READ_INTEGER() - 1; if (!POP()->to_bool()) this->program_counter += to; break; }
            case OP_STORE: { this->top_frame->heap[READ_VARIABLE()] = *POP(); break; }
            case OP_STORE_ACCESS: { BINARY_POP(); (*this->top_frame->heap.at(READ_VARIABLE()).lvalues)[b->nvalue] = a; break; }
            case OP_LOAD: { PUSH(this->top_frame->heap.at(READ_VARIABLE())); break; }
            case OP_LIST: { this->do_list(); break; }
            case OP_DICTIONARY: { this->do_dictionary(); break; }
            case OP_ACCESS: { this->do_access(); break; }
            case OP_FUNCTION: { PUSH(Value(READ_INTEGER(), new Frame(*this->top_frame))); break; }
            case OP_CALL: { break; }
            case OP_LEN: { PUSH(POP()->length()); break; }
            case OP_PRINT: { POP()->println(); break; }
            case OP_EXIT: { for (auto i = this->stack; i < this->top_stack; i++) i->println(); return; }
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

#undef PUSH
#undef POP
#undef BINARY_POP
#undef READ_INSTRUCTION
#undef READ_CONSTANT
#undef READ_INTEGER
#undef READ_VARIABLE
