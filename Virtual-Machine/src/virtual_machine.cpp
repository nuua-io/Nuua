#include "../include/virtual_machine.hpp"
#include "../../Compiler/include/compiler.hpp"
#include "../../Logger/include/logger.hpp"

#define INST (this->program_counter)
#define AS_LITERAL(op) (INST->op)
#define AS_REGISTER(op) (this->top_frame->registers + INST->op)
#define AS_CONSTANT(op) (&this->get_current_memory()->constants[INST->op])

void VirtualMachine::run()
{
    for (uint64_t instruction; ; this->program_counter++) {
        // printf("=> %s (%i)\n", opcode_to_string(instruction).c_str(), instruction);
        switch (this->program_counter->instruction) {
            // Others
            case OP_EXIT: { return; }

            // Register manipulation
            case OP_MOVE_RR: { AS_REGISTER(op2)->copy_to(AS_REGISTER(op1)); break; }
            case OP_MOVE_RC: { AS_CONSTANT(op2)->copy_to(AS_REGISTER(op1)); break; }

            // Addition
            case OP_ADD_RR: { Value::op_add(AS_REGISTER(op1), AS_REGISTER(op2), AS_REGISTER(op3)); break; }
            case OP_ADD_RC: { Value::op_add(AS_REGISTER(op1), AS_REGISTER(op2), AS_CONSTANT(op3)); break; }
            case OP_ADD_CR: { Value::op_add(AS_REGISTER(op1), AS_CONSTANT(op2), AS_REGISTER(op3)); break; }
            case OP_ADD_CC: { Value::op_add(AS_REGISTER(op1), AS_CONSTANT(op2), AS_CONSTANT(op3)); break; }

            // Substraction
            case OP_SUB_RR: { Value::op_sub(AS_REGISTER(op1), AS_REGISTER(op2), AS_REGISTER(op3)); break; }
            case OP_SUB_RC: { Value::op_sub(AS_REGISTER(op1), AS_REGISTER(op2), AS_CONSTANT(op3)); break; }
            case OP_SUB_CR: { Value::op_sub(AS_REGISTER(op1), AS_CONSTANT(op2), AS_REGISTER(op3)); break; }
            case OP_SUB_CC: { Value::op_sub(AS_REGISTER(op1), AS_CONSTANT(op2), AS_CONSTANT(op3)); break; }

            // Multiplication
            case OP_MUL_RR: { Value::op_mul(AS_REGISTER(op1), AS_REGISTER(op2), AS_REGISTER(op3)); break; }
            case OP_MUL_RC: { Value::op_mul(AS_REGISTER(op1), AS_REGISTER(op2), AS_CONSTANT(op3)); break; }
            case OP_MUL_CR: { Value::op_mul(AS_REGISTER(op1), AS_CONSTANT(op2), AS_REGISTER(op3)); break; }
            case OP_MUL_CC: { Value::op_mul(AS_REGISTER(op1), AS_CONSTANT(op2), AS_CONSTANT(op3)); break; }

            // Division
            case OP_DIV_RR: { Value::op_div(AS_REGISTER(op1), AS_REGISTER(op2), AS_REGISTER(op3)); break; }
            case OP_DIV_RC: { Value::op_div(AS_REGISTER(op1), AS_REGISTER(op2), AS_CONSTANT(op3)); break; }
            case OP_DIV_CR: { Value::op_div(AS_REGISTER(op1), AS_CONSTANT(op2), AS_REGISTER(op3)); break; }
            case OP_DIV_CC: { Value::op_div(AS_REGISTER(op1), AS_CONSTANT(op2), AS_CONSTANT(op3)); break; }

            // Equality
            case OP_EQ_RR: { Value::op_eq(AS_REGISTER(op1), AS_REGISTER(op2), AS_REGISTER(op3)); break; }
            case OP_EQ_RC: { Value::op_eq(AS_REGISTER(op1), AS_REGISTER(op2), AS_CONSTANT(op3)); break; }
            case OP_EQ_CR: { Value::op_eq(AS_REGISTER(op1), AS_CONSTANT(op2), AS_REGISTER(op3)); break; }
            case OP_EQ_CC: { Value::op_eq(AS_REGISTER(op1), AS_CONSTANT(op2), AS_CONSTANT(op3)); break; }

            // No equality
            case OP_NEQ_RR: { Value::op_neq(AS_REGISTER(op1), AS_REGISTER(op2), AS_REGISTER(op3)); break; }
            case OP_NEQ_RC: { Value::op_neq(AS_REGISTER(op1), AS_REGISTER(op2), AS_CONSTANT(op3)); break; }
            case OP_NEQ_CR: { Value::op_neq(AS_REGISTER(op1), AS_CONSTANT(op2), AS_REGISTER(op3)); break; }
            case OP_NEQ_CC: { Value::op_neq(AS_REGISTER(op1), AS_CONSTANT(op2), AS_CONSTANT(op3)); break; }

            // Higher than
            case OP_HT_RR: { Value::op_ht(AS_REGISTER(op1), AS_REGISTER(op2), AS_REGISTER(op3)); break; }
            case OP_HT_RC: { Value::op_ht(AS_REGISTER(op1), AS_REGISTER(op2), AS_CONSTANT(op3)); break; }
            case OP_HT_CR: { Value::op_ht(AS_REGISTER(op1), AS_CONSTANT(op2), AS_REGISTER(op3)); break; }
            case OP_HT_CC: { Value::op_ht(AS_REGISTER(op1), AS_CONSTANT(op2), AS_CONSTANT(op3)); break; }

            // Higher than or equal
            case OP_HTE_RR: { Value::op_hte(AS_REGISTER(op1), AS_REGISTER(op2), AS_REGISTER(op3)); break; }
            case OP_HTE_RC: { Value::op_hte(AS_REGISTER(op1), AS_REGISTER(op2), AS_CONSTANT(op3)); break; }
            case OP_HTE_CR: { Value::op_hte(AS_REGISTER(op1), AS_CONSTANT(op2), AS_REGISTER(op3)); break; }
            case OP_HTE_CC: { Value::op_hte(AS_REGISTER(op1), AS_CONSTANT(op2), AS_CONSTANT(op3)); break; }

            // Lower than
            case OP_LT_RR: { Value::op_lt(AS_REGISTER(op1), AS_REGISTER(op2), AS_REGISTER(op3)); break; }
            case OP_LT_RC: { Value::op_lt(AS_REGISTER(op1), AS_REGISTER(op2), AS_CONSTANT(op3)); break; }
            case OP_LT_CR: { Value::op_lt(AS_REGISTER(op1), AS_CONSTANT(op2), AS_REGISTER(op3)); break; }
            case OP_LT_CC: { Value::op_lt(AS_REGISTER(op1), AS_CONSTANT(op2), AS_CONSTANT(op3)); break; }

            // Lower than or equal
            case OP_LTE_RR: { Value::op_lte(AS_REGISTER(op1), AS_REGISTER(op2), AS_REGISTER(op3)); break; }
            case OP_LTE_RC: { Value::op_lte(AS_REGISTER(op1), AS_REGISTER(op2), AS_CONSTANT(op3)); break; }
            case OP_LTE_CR: { Value::op_lte(AS_REGISTER(op1), AS_CONSTANT(op2), AS_REGISTER(op3)); break; }
            case OP_LTE_CC: { Value::op_lte(AS_REGISTER(op1), AS_CONSTANT(op2), AS_CONSTANT(op3)); break; }

            // Control flow (All relative jumps)
            case OP_FJUMP: { this->program_counter += AS_LITERAL(op1) - 1; break; }
            case OP_BJUMP: { this->program_counter -= AS_LITERAL(op1) + 1; break; }
            case OP_FJUMP_R: { if (AS_REGISTER(op2)->to_bool()) this->program_counter += AS_LITERAL(op1) - 1; break; }
            case OP_BJUMP_R: { if (AS_REGISTER(op2)->to_bool()) this->program_counter -= AS_LITERAL(op1) + 1; break; }
            case OP_FJUMP_C: { if (AS_CONSTANT(op2)->to_bool()) this->program_counter += AS_LITERAL(op1) - 1; break; }
            case OP_BJUMP_C: { if (AS_CONSTANT(op2)->to_bool()) this->program_counter -= AS_LITERAL(op1) + 1; break; }
            case OP_FNJUMP_R: { if (!AS_REGISTER(op2)->to_bool()) this->program_counter += AS_LITERAL(op1) - 1; break; }
            case OP_BNJUMP_R: { if (!AS_REGISTER(op2)->to_bool()) this->program_counter -= AS_LITERAL(op1) + 1; break; }
            case OP_FNJUMP_C: { if (!AS_CONSTANT(op2)->to_bool()) this->program_counter += AS_LITERAL(op1) - 1; break; }
            case OP_BNJUMP_C: { if (!AS_CONSTANT(op2)->to_bool()) this->program_counter -= AS_LITERAL(op1) + 1; break; }

            // Utilities
            case OP_PRINT_R: { AS_REGISTER(op1)->println(); break; }
            case OP_PRINT_C: { AS_CONSTANT(op1)->println(); break; }
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
    this->program_counter = (Instruction *) &this->program.program.code[0];
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

#undef INST
#undef AS_LITERAL
#undef AS_REGISTER
#undef AS_CONSTANT
