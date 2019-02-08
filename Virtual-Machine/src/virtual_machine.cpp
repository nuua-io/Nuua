/**
 * |----------------------------------|
 * | Nuua Stack Based Virtual Machine |
 * |----------------------------------|
 *
 * Copyright 2019 Erik Campobadal <soc@erik.cat>
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
            /*
                Pushes a new value to the stack.
            */
            case OP_PUSH: {
                this->push(READ_CONSTANT());
                break;
            }
            /*
                Pops the top value of the stack.
            */
            case OP_POP: {
                this->pop();
                break;
            }
            /*
                Performs -a.
            */
            case OP_MINUS: {
                this->push(-*this->pop());
                break;
            }
            /*
                Performs !a.
            */
            case OP_NOT: {
                this->push(!*this->pop());
                break;
            }
            /*
                Performs a + b.
            */
            case OP_ADD: {
                BINARY_POP();
                this->push(*a + *b);
                break;
            }
            /*
                Performs a - b.
            */
            case OP_SUB: {
                BINARY_POP();
                this->push(*a - *b);
                break;
            }
            /*
                Performs a * b.
            */
            case OP_MUL: {
                BINARY_POP();
                this->push(*a * *b);
                break;
            }
            /*
                Performs a / b. Can't perform division by 0 (b = 0).
            */
            case OP_DIV: {
                BINARY_POP();
                this->push(*a / *b);
                break;
            }
            /*
                Performs a == b.
            */
            case OP_EQ: {
                BINARY_POP();
                this->push(*a == *b);
                break;
            }
            /*
                Performs a != b.
            */
            case OP_NEQ: {
                BINARY_POP();
                this->push(*a != *b);
                break;
            }
            /*
                Performs a < b.
            */
            case OP_LT: {
                BINARY_POP();
                this->push(*a < *b);
                break;
            }
            /*
                Performs a <= b.
            */
            case OP_LTE: {
                BINARY_POP();
                this->push(*a <= *b);
                break;
            }
            /*
                Performs a > b.
            */
            case OP_HT: {
                BINARY_POP();
                this->push(*a > *b);
                break;
            }
            /*
                Performs a >= b.
            */
            case OP_HTE: {
                BINARY_POP();
                this->push(*a >= *b);
                break;
            }
            /*
                Casts a value type to another.
            */
            case OP_CAST: {
                this->push(this->pop()->cast(READ_VARIABLE()));
                break;
            }
            /*
                Performs a relative jump in the program counter.
            */
            case OP_RJUMP: {
                this->program_counter += READ_INT() - 1;
                break;
            }
            /*
                Performs a branch if a is true.
            */
            case OP_BRANCH_TRUE: {
                auto to = READ_INT() - 1;
                if (this->pop()->to_bool()) this->program_counter += to;
                break;
            }
            /*
                Performs a branch if a is false.
            */
            case OP_BRANCH_FALSE: {
                auto to = READ_INT() - 1;
                if (!this->pop()->to_bool()) this->program_counter += to;
                break;
            }
            /*
                Declares a variable.
            */
            case OP_DECLARE: {
                // Get the variable name
                auto name = READ_VARIABLE();
                auto default_value = READ_CONSTANT();
                this->top_frame->heap[name] = default_value;
                break;
            }
            /*
                Performs  a = b.
            */
            case OP_STORE: {
                // Get the variable name to store to.
                auto name = READ_VARIABLE();
                // Store the variable to the current frame
                this->top_frame->heap[name] = *this->pop();
                // Push the value to the stack.
                this->push(this->top_frame->heap[name]);
                break;
            }
            /*
                Performs  a = b. It does not push the value.
            */
            case OP_ONLY_STORE: {
                // Store and push the value to the stack to make it available as an expression.
                this->top_frame->heap[READ_VARIABLE()] = *this->pop();
                break;
            }
            /*
                Performs  a[b] = c.
            */
            case OP_STORE_ACCESS_INT: {
                BINARY_POP();
                (*this->top_frame->heap[READ_VARIABLE()].value_list)[b->value_int] = a;
                break;
            }
            case OP_STORE_ACCESS_STRING: {
                BINARY_POP();
                this->top_frame->heap[READ_VARIABLE()].value_dict->values[*b->value_string] = a;
                break;
            }
            /*
                Performs  a.
            */
            case OP_LOAD: {
                this->push(this->top_frame->heap[READ_VARIABLE()]);
                break;
            }
            /*
                Creates a list given X stack values.
            */
            case OP_LIST: {
                std::vector<Value> v;
                for (int pops = READ_INT(); pops > 0; pops--) v.push_back(*this->pop());
                this->push(Value(v));
                break;
            }
            /*
                Creates a dictionary given X stack values.
            */
            case OP_DICTIONARY: {
                std::unordered_map<std::string, Value> dictionary;
                std::vector<std::string> key_order;
                for (int e = READ_INT(); e > 0; e--) {
                    auto val = *this->pop();
                    auto n = *this->pop()->value_string;
                    dictionary[n] = val;
                    key_order.push_back(n);
                }
                this->push(Value(dictionary, key_order));
                break;
            }
            /*
                Performs a[b].
            */
            case OP_ACCESS_INT: {
                this->push((*this->top_frame->heap[READ_VARIABLE()].value_list)[this->pop()->value_int]);
                break;
            }
            case OP_ACCESS_STRING: {
                this->push(this->top_frame->heap[READ_VARIABLE()].value_dict->values[*this->pop()->value_string]);
                break;
            }
            /*
                Creates a function value.
            */
            case OP_FUNCTION: {
                auto index = READ_INT();
                auto return_type = READ_VARIABLE();
                this->push(Value(index, return_type, new Frame(*this->top_frame)));
                break;
            }
            /*
                Performs return X.
            */
            case OP_RETURN: {
                // Turn back the program counter to the original one.
                this->program_counter = (this->top_frame--)->return_address;
                // Change back the current memory
                this->current_memory--;
                break;
            }
            /*
                Performs a(X, Y, Z, ...).
            */
            case OP_CALL: {
                auto name = READ_VARIABLE();
                auto arguments = READ_INT();
                auto value = this->top_frame->heap[name];
                // Set the new frame to work on.
                *(++this->top_frame) = *value.value_fun->frame;
                // Set the return address
                this->top_frame->return_address = this->program_counter;
                // Set the frame caller.
                // this->top_frame->caller = value;
                // Set the memory to the functions memory.
                *(++this->current_memory) = FUNCTIONS_MEMORY;
                // Set the program counter depending on the function index.
                this->program_counter = &this->get_current_memory()->code[value.value_fun->index];
                break;
            }
            /*
                Pushes the length of the value.
            */
            case OP_LEN: {
                this->push(this->pop()->length());
                break;
            }
            /*
                Prints the top of the stack value.
            */
            case OP_PRINT: {
                this->pop()->println();
                break;
            }
            /*
                Exits the virtual machine, finishing the execution.
            */
            case OP_EXIT: {
                return;
            }
            /*
                Generic error case.
            */
            default: {
                logger->error("Unknown instruction at line", this->get_current_line());
                exit(EXIT_FAILURE);
                break;
            }
        }
    }
}

void VirtualMachine::interpret(const char *source)
{
    this->program = Compiler().compile(source);

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
