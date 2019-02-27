#include "../include/virtual_machine.hpp"
#include "../../Compiler/include/compiler.hpp"
#include "../../Logger/include/logger.hpp"

#define READ_LITERAL() (*this->program_counter++)
#define READ_REGISTER() (this->top_frame->registers + READ_LITERAL())
#define READ_CONSTANT() (&this->get_current_memory()->constants[READ_LITERAL()])

void VirtualMachine::run()
{
    for (uint64_t instruction;;) {
        instruction = READ_LITERAL();
        // printf("=> %s (%i)\n", opcode_to_string(instruction).c_str(), instruction);
        switch (instruction) {

            // Others
            case OP_EXIT: { return; }

            // Register manipulation
            case OP_MOVE_RR: {
                auto dest = READ_REGISTER();
                READ_REGISTER()->copy_to(dest);
                break;
            }
            case OP_MOVE_RC: {
                auto dest = READ_REGISTER();
                READ_CONSTANT()->copy_to(dest);
                break;
            }

            // Addition
            case OP_ADD_RR: {
                auto dest = READ_REGISTER();
                auto op1 = READ_REGISTER();
                auto op2 = READ_REGISTER();
                Value::op_add(dest, op1, op2);
                break;
            }
            case OP_ADD_RC: {
                auto dest = READ_REGISTER();
                auto op1 = READ_REGISTER();
                auto op2 = READ_CONSTANT();
                Value::op_add(dest, op1, op2);
                break;
            }
            case OP_ADD_CR: {
                auto dest = READ_REGISTER();
                auto op1 = READ_CONSTANT();
                auto op2 = READ_REGISTER();
                Value::op_add(dest, op1, op2);
                break;
            }
            case OP_ADD_CC: {
                auto dest = READ_REGISTER();
                auto op1 = READ_CONSTANT();
                auto op2 = READ_CONSTANT();
                Value::op_add(dest, op1, op2);
                break;
            }

            // Substraction
            case OP_SUB_RR: {
                auto dest = READ_REGISTER();
                auto op1 = READ_REGISTER();
                auto op2 = READ_REGISTER();
                Value::op_sub(dest, op1, op2);
                break;
            }
            case OP_SUB_RC: {
                auto dest = READ_REGISTER();
                auto op1 = READ_REGISTER();
                auto op2 = READ_CONSTANT();
                Value::op_sub(dest, op1, op2);
                break;
            }
            case OP_SUB_CR: {
                auto dest = READ_REGISTER();
                auto op1 = READ_CONSTANT();
                auto op2 = READ_REGISTER();
                Value::op_sub(dest, op1, op2);
                break;
            }
            case OP_SUB_CC: {
                auto dest = READ_REGISTER();
                auto op1 = READ_CONSTANT();
                auto op2 = READ_CONSTANT();
                Value::op_sub(dest, op1, op2);
                break;
            }

            // Multiplication
            case OP_MUL_RR: {
                auto dest = READ_REGISTER();
                auto op1 = READ_REGISTER();
                auto op2 = READ_REGISTER();
                Value::op_mul(dest, op1, op2);
                break;
            }
            case OP_MUL_RC: {
                auto dest = READ_REGISTER();
                auto op1 = READ_REGISTER();
                auto op2 = READ_CONSTANT();
                Value::op_mul(dest, op1, op2);
                break;
            }
            case OP_MUL_CR: {
                auto dest = READ_REGISTER();
                auto op1 = READ_CONSTANT();
                auto op2 = READ_REGISTER();
                Value::op_mul(dest, op1, op2);
                break;
            }
            case OP_MUL_CC: {
                auto dest = READ_REGISTER();
                auto op1 = READ_CONSTANT();
                auto op2 = READ_CONSTANT();
                Value::op_mul(dest, op1, op2);
                break;
            }

            // Division
            case OP_DIV_RR: {
                auto dest = READ_REGISTER();
                auto op1 = READ_REGISTER();
                auto op2 = READ_REGISTER();
                Value::op_div(dest, op1, op2);
                break;
            }
            case OP_DIV_RC: {
                auto dest = READ_REGISTER();
                auto op1 = READ_REGISTER();
                auto op2 = READ_CONSTANT();
                Value::op_div(dest, op1, op2);
                break;
            }
            case OP_DIV_CR: {
                auto dest = READ_REGISTER();
                auto op1 = READ_CONSTANT();
                auto op2 = READ_REGISTER();
                Value::op_div(dest, op1, op2);
                break;
            }
            case OP_DIV_CC: {
                auto dest = READ_REGISTER();
                auto op1 = READ_CONSTANT();
                auto op2 = READ_CONSTANT();
                Value::op_div(dest, op1, op2);
                break;
            }

            // Equality
            case OP_EQ_RR: {
                auto dest = READ_REGISTER();
                auto op1 = READ_REGISTER();
                auto op2 = READ_REGISTER();
                Value::op_eq(dest, op1, op2);
                break;
            }
            case OP_EQ_RC: {
                auto dest = READ_REGISTER();
                auto op1 = READ_REGISTER();
                auto op2 = READ_CONSTANT();
                Value::op_eq(dest, op1, op2);
                break;
            }
            case OP_EQ_CR: {
                auto dest = READ_REGISTER();
                auto op1 = READ_CONSTANT();
                auto op2 = READ_REGISTER();
                Value::op_eq(dest, op1, op2);
                break;
            }
            case OP_EQ_CC: {
                auto dest = READ_REGISTER();
                auto op1 = READ_CONSTANT();
                auto op2 = READ_CONSTANT();
                Value::op_eq(dest, op1, op2);
                break;
            }

            // No equality
            case OP_NEQ_RR: {
                auto dest = READ_REGISTER();
                auto op1 = READ_REGISTER();
                auto op2 = READ_REGISTER();
                Value::op_neq(dest, op1, op2);
                break;
            }
            case OP_NEQ_RC: {
                auto dest = READ_REGISTER();
                auto op1 = READ_REGISTER();
                auto op2 = READ_CONSTANT();
                Value::op_neq(dest, op1, op2);
                break;
            }
            case OP_NEQ_CR: {
                auto dest = READ_REGISTER();
                auto op1 = READ_CONSTANT();
                auto op2 = READ_REGISTER();
                Value::op_neq(dest, op1, op2);
                break;
            }
            case OP_NEQ_CC: {
                auto dest = READ_REGISTER();
                auto op1 = READ_CONSTANT();
                auto op2 = READ_CONSTANT();
                Value::op_neq(dest, op1, op2);
                break;
            }

            // Higher than
            case OP_HT_RR: {
                auto dest = READ_REGISTER();
                auto op1 = READ_REGISTER();
                auto op2 = READ_REGISTER();
                Value::op_ht(dest, op1, op2);
                break;
            }
            case OP_HT_RC: {
                auto dest = READ_REGISTER();
                auto op1 = READ_REGISTER();
                auto op2 = READ_CONSTANT();
                Value::op_ht(dest, op1, op2);
                break;
            }
            case OP_HT_CR: {
                auto dest = READ_REGISTER();
                auto op1 = READ_CONSTANT();
                auto op2 = READ_REGISTER();
                Value::op_ht(dest, op1, op2);
                break;
            }
            case OP_HT_CC: {
                auto dest = READ_REGISTER();
                auto op1 = READ_CONSTANT();
                auto op2 = READ_CONSTANT();
                Value::op_ht(dest, op1, op2);
                break;
            }

            // Higher than or equal
            case OP_HTE_RR: {
                auto dest = READ_REGISTER();
                auto op1 = READ_REGISTER();
                auto op2 = READ_REGISTER();
                Value::op_hte(dest, op1, op2);
                break;
            }
            case OP_HTE_RC: {
                auto dest = READ_REGISTER();
                auto op1 = READ_REGISTER();
                auto op2 = READ_CONSTANT();
                Value::op_hte(dest, op1, op2);
                break;
            }
            case OP_HTE_CR: {
                auto dest = READ_REGISTER();
                auto op1 = READ_CONSTANT();
                auto op2 = READ_REGISTER();
                Value::op_hte(dest, op1, op2);
                break;
            }
            case OP_HTE_CC: {
                auto dest = READ_REGISTER();
                auto op1 = READ_CONSTANT();
                auto op2 = READ_CONSTANT();
                Value::op_hte(dest, op1, op2);
                break;
            }

            // Lower than
            case OP_LT_RR: {
                auto dest = READ_REGISTER();
                auto op1 = READ_REGISTER();
                auto op2 = READ_REGISTER();
                Value::op_lt(dest, op1, op2);
                break;
            }
            case OP_LT_RC: {
                auto dest = READ_REGISTER();
                auto op1 = READ_REGISTER();
                auto op2 = READ_CONSTANT();
                Value::op_lt(dest, op1, op2);
                break;
            }
            case OP_LT_CR: {
                auto dest = READ_REGISTER();
                auto op1 = READ_CONSTANT();
                auto op2 = READ_REGISTER();
                Value::op_lt(dest, op1, op2);
                break;
            }
            case OP_LT_CC: {
                auto dest = READ_REGISTER();
                auto op1 = READ_CONSTANT();
                auto op2 = READ_CONSTANT();
                Value::op_lt(dest, op1, op2);
                break;
            }

            // Lower than or equal
            case OP_LTE_RR: {
                auto dest = READ_REGISTER();
                auto op1 = READ_REGISTER();
                auto op2 = READ_REGISTER();
                Value::op_lte(dest, op1, op2);
                break;
            }
            case OP_LTE_RC: {
                auto dest = READ_REGISTER();
                auto op1 = READ_REGISTER();
                auto op2 = READ_CONSTANT();
                Value::op_lte(dest, op1, op2);
                break;
            }
            case OP_LTE_CR: {
                auto dest = READ_REGISTER();
                auto op1 = READ_CONSTANT();
                auto op2 = READ_REGISTER();
                Value::op_lte(dest, op1, op2);
                break;
            }
            case OP_LTE_CC: {
                auto dest = READ_REGISTER();
                auto op1 = READ_CONSTANT();
                auto op2 = READ_CONSTANT();
                Value::op_lte(dest, op1, op2);
                break;
            }

            // Control flow (All relative jumps)
            case OP_FJUMP: {
                this->program_counter += READ_LITERAL() - 1;
                break;
            }
            case OP_BJUMP: {
                this->program_counter -= READ_LITERAL() + 1;
                break;
            }
            case OP_FJUMP_R: {
                auto position = READ_LITERAL() - 1;
                if (READ_REGISTER()->to_bool()) this->program_counter += position;
                break;
            }
            case OP_BJUMP_R: {
                auto position = READ_LITERAL() + 1;
                if (READ_REGISTER()->to_bool()) this->program_counter -= position;
                break;
            }
            case OP_FJUMP_C: {
                auto position = READ_LITERAL() - 1;
                if (READ_CONSTANT()->to_bool()) this->program_counter += position;
                break;
            }
            case OP_BJUMP_C: {
                auto position = READ_LITERAL() + 1;
                if (READ_CONSTANT()->to_bool()) this->program_counter -= position;
                break;
            }
            case OP_FNJUMP_R: {
                auto position = READ_LITERAL() - 1;
                if (!READ_REGISTER()->to_bool()) this->program_counter += position;
                break;
            }
            case OP_BNJUMP_R: {
                auto position = READ_LITERAL() + 1;
                if (!READ_REGISTER()->to_bool()) this->program_counter -= position;
                break;
            }
            case OP_FNJUMP_C: {
                auto position = READ_LITERAL() - 1;
                if (!READ_CONSTANT()->to_bool()) this->program_counter += position;
                break;
            }
            case OP_BNJUMP_C: {
                auto position = READ_LITERAL() + 1;
                if (!READ_CONSTANT()->to_bool()) this->program_counter -= position;
                break;
            }

            // Utilities
            case OP_PRINT_R: {
                READ_REGISTER()->println();
                break;
            }
            case OP_PRINT_C: {
                READ_CONSTANT()->println();
                break;
            }
        }
    }
}

Memory *VirtualMachine::get_current_memory()
{
    switch (*this->current_memory) {
        case FUNCTIONS_MEMORY: { return &this->program.functions; }
        case CLASSES_MEMORY: { return &this->program.classes; }
        default: { return &this->program.program; }
    }
}

void VirtualMachine::interpret(const char *source)
{
    // Compile the code.
    auto compiler = new Compiler;
    this->program = compiler->compile(source);
    delete compiler;
    // Allocate the main frame registers.
    // this->frames[0].allocate_registers(this->program.main_registers);
    this->top_frame->allocate_registers(this->program.main_registers);
    // Set the first opcode
    this->program_counter = &this->program.program.code[0];
    // Run the compiled code.
    logger->info("Started interpreting...");
    this->run();
    logger->success("Finished interpreting!");
    // Clear the main frame registers
    this->top_frame->free_registers();
}

void VirtualMachine::reset()
{

}

#undef READ_LITERAL
#undef READ_REGISTER
#undef READ_CONSTANT
