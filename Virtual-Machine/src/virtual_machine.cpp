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

#define BINARY_POP() Value *b = this->pop(); Value *a = this->pop()
#define READ_INSTRUCTION() (*this->program_counter++)
#define READ_CONSTANT() (this->get_current_memory()->constants[READ_INSTRUCTION()])
#define READ_INT() (READ_CONSTANT().value_int)
#define READ_VARIABLE() (*READ_CONSTANT().value_string)

void VirtualMachine::push(Value value)
{
    if (this->top_stack - this->stack >= STACK_SIZE) {
        logger->error("Stack overflow", this->get_current_line());
        exit(EXIT_FAILURE);
    }

    *this->top_stack++ = value;
}

Value *VirtualMachine::pop()
{
    return --this->top_stack;
}

void VirtualMachine::do_list()
{
    std::vector<Value> v;
    for (int pops = READ_INT(); pops > 0; pops--) v.push_back(*this->pop());
    this->push(Value(v));
}

void VirtualMachine::do_dictionary()
{
    std::unordered_map<std::string, Value> dictionary;
    std::vector<std::string> key_order;
    for (int e = READ_INT(); e > 0; e--) {
        auto val = *this->pop();
        auto n = *this->pop()->value_string;
        dictionary[n] = val;
        key_order.push_back(n);
    }
    this->push(Value(dictionary, key_order));
}

void VirtualMachine::do_access()
{
    auto var = this->top_frame->heap.at(READ_VARIABLE());
    auto index = this->pop();
    if (var.is(VALUE_LIST) && index->is(VALUE_INT)) {
        this->push((*var.value_list)[index->value_int]);
    } else if (var.is(VALUE_DICT) && index->is(VALUE_STRING)) {
        this->push(var.value_dict->values[*index->value_string]);
    } else {
        if (var.is(VALUE_DICT)) {
            logger->error("Invalid access instruction. You need to use a string as a key", this->get_current_line());
            exit(EXIT_FAILURE);
        } else {
            logger->error("Invalid access instruction. You need to use a number as a key", this->get_current_line());
            exit(EXIT_FAILURE);
        }
    }
}

void VirtualMachine::do_declare()
{
    // Get the variable name
    auto name = READ_VARIABLE();
    auto default_value = READ_CONSTANT();

    if (this->variable_declared(name)) {
        logger->error("Variable '" + name + "' is already declared.", this->get_current_line());
        exit(EXIT_FAILURE);
    }

    this->top_frame->heap[name] = default_value;
}

void VirtualMachine::do_return()
{
    // Check the return type
    auto return_value = this->top_frame->caller;
    auto returned_value = *(this->top_stack - 1);
    *(this->top_stack - 1) = returned_value.cast(return_value.value_fun->return_type);

    // Turn back the program counter to the original one.
    this->program_counter = (this->top_frame--)->return_address;

    // Change back the current memory
    this->current_memory--;
}

void VirtualMachine::do_call()
{
    auto name = READ_VARIABLE();
    auto arguments = READ_INT();
    auto value = this->load_variable(name);

    if (!value.type.is(VALUE_FUN)) {
        logger->error("Target is not callable. Are you sure that '" + name + "' is a function?");
        exit(EXIT_FAILURE);
    }

    // Set the new frame to work on.
    *(++this->top_frame) = *value.value_fun->frame;

    // Set the return address
    this->top_frame->return_address = this->program_counter; // +3 becuase of the OP_CALL and it's 2 arguments.

    // Set the frame caller.
    this->top_frame->caller = value;

    // Set the memory to the functions memory.
    *(++this->current_memory) = FUNCTIONS_MEMORY;

    // Set the program counter depending on the function index.
    this->program_counter = &this->get_current_memory()->code[value.value_fun->index];
}

bool VirtualMachine::variable_declared(std::string name)
{
    return this->top_frame->heap.find(name) != this->top_frame->heap.end();
}

Value VirtualMachine::load_variable(std::string name)
{
    // Check if exists
    if (!this->variable_declared(name)) {
        logger->error("Undeclared variable '" + name + "'.", this->get_current_line());
        exit(EXIT_FAILURE);
    }

    return this->top_frame->heap[name];
}

void VirtualMachine::store_variable(std::string name, Value *new_value, bool only_store)
{
    // Check if exists
    if (!this->variable_declared(name)) {
        logger->error("Undeclared variable '" + name + "'.", this->get_current_line());
        exit(EXIT_FAILURE);
    }

    auto current_value = this->top_frame->heap[name];

    // Store and push the value to the stack to make it available as an expression.
    // It also checks the types and casts if nessesary.
    this->top_frame->heap[name] = current_value.type.same_as(&new_value->type)
            ? *new_value
            : new_value->cast(current_value.type);

    if (!only_store) this->push(this->top_frame->heap[name]);
}

Memory *VirtualMachine::get_current_memory()
{
    switch (*this->current_memory) {
        case FUNCTIONS_MEMORY: { return &this->program.functions; }
        case CLASSES_MEMORY: { return &this->program.classes; }
        default: { return &this->program.program; }
    }
}

uint32_t VirtualMachine::get_current_line()
{
    return this->get_current_memory()->lines[
        static_cast<uint64_t>(this->program_counter - &this->get_current_memory()->code.front()) - 1
    ];
}

void VirtualMachine::run()
{
    this->program_counter = &this->program.program.code[0];

    #if DEBUG
        uint64_t times = 0;
    #endif

    for (uint64_t instruction;;) {
        instruction = READ_INSTRUCTION();
        #if DEBUG
            // printf("=> %s (%llu) [%llu]\n", opcode_to_string(instruction).c_str(), instruction, times++);
        #endif
        switch (instruction) {
            case OP_PUSH: { this->push(READ_CONSTANT()); break; }
            case OP_POP: { this->pop(); break; }
            case OP_MINUS: { this->push(-*this->pop()); break; }
            case OP_NOT: { this->push(!*this->pop()); break; }
            case OP_ADD: { BINARY_POP(); this->push(*a + *b); break; }
            case OP_SUB: { BINARY_POP(); this->push(*a - *b); break; }
            case OP_MUL: { BINARY_POP(); this->push(*a * *b); break; }
            case OP_DIV: { BINARY_POP(); this->push(*a / *b); break; }
            case OP_EQ: { BINARY_POP(); this->push(*a == *b); break; }
            case OP_NEQ: { BINARY_POP(); this->push(*a != *b); break; }
            case OP_LT: { BINARY_POP(); this->push(*a < *b); break; }
            case OP_LTE: { BINARY_POP(); this->push(*a <= *b); break; }
            case OP_HT: { BINARY_POP(); this->push(*a > *b); break; }
            case OP_HTE: { BINARY_POP(); this->push(*a >= *b); break; }
            // case OP_JUMP: { this->program_counter = &this->get_current_memory()->code.front() + (READ_INT() - 1); break; }
            case OP_RJUMP: { this->program_counter += READ_INT() - 1; break; }
            case OP_BRANCH_TRUE: { auto to = READ_INT() - 1; if (this->pop()->to_bool()) this->program_counter += to; break; }
            case OP_BRANCH_FALSE: { auto to = READ_INT() - 1; if (!this->pop()->to_bool()) this->program_counter += to; break; }
            case OP_DECLARE: { this->do_declare(); break; }
            case OP_STORE: { this->store_variable(READ_VARIABLE(), this->pop(), false); break; }
            case OP_ONLY_STORE: { this->store_variable(READ_VARIABLE(), this->pop(), true); break; }
            // OP_STORE_ACCESS needs a re-write for dicts.
            case OP_STORE_ACCESS: { BINARY_POP(); (*this->top_frame->heap[READ_VARIABLE()].value_list)[b->value_int] = a; break; }
            case OP_LOAD: { this->push(this->load_variable(READ_VARIABLE())); break; }
            case OP_LIST: { this->do_list(); break; }
            case OP_DICTIONARY: { this->do_dictionary(); break; }
            case OP_ACCESS: { this->do_access(); break; }
            case OP_FUNCTION: { auto index = READ_INT(); auto return_type = READ_VARIABLE(); this->push(Value(index, return_type, new Frame(*this->top_frame))); break; }
            case OP_RETURN: { this->do_return(); break; }
            case OP_CALL: { this->do_call(); break; }
            case OP_LEN: { this->push(this->pop()->length()); break; }
            case OP_PRINT: { this->pop()->println(); break; }
            case OP_EXIT: { return; }
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

    if (this->program.program.code.size() > 0) this->run();

    logger->success("Finished interpreting");

    #if DEBUG
        if (this->top_stack - this->stack == 0) {
            logger->success("No memory leak detected");
        } else {
            logger->warning("Memory leak detected!");
            for (auto i = this->stack; i < this->top_stack; i++) i->println();
        }
    #endif

}

void VirtualMachine::reset()
{
    this->program.reset();
}

#undef BINARY_POP
#undef READ_INSTRUCTION
#undef READ_CONSTANT
#undef READ_INT
#undef READ_VARIABLE
