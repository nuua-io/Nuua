#include "../include/virtual_machine.hpp"
#include "../../Compiler/include/compiler.hpp"
#include "../../Logger/include/logger.hpp"

#define READ_INSTRUCTION() (*this->program_counter++)
#define READ_REGISTER() (this->top_frame->registers + READ_INSTRUCTION())
#define READ_CONSTANT() (&this->get_current_memory()->constants[READ_INSTRUCTION()])

void VirtualMachine::run()
{
    for (;;) {
        switch (READ_INSTRUCTION()) {
            case OP_LOAD: {
                auto dest = READ_REGISTER();
                READ_CONSTANT()->copy_to(dest);
                break;
            }
            case OP_MOVE: {
                auto dest = READ_REGISTER();
                READ_REGISTER()->copy_to(dest);
                break;
            }
            case OP_ADD: {
                auto dest = READ_REGISTER();
                auto op1 = READ_REGISTER();
                auto op2 = READ_REGISTER();
                Value::op_add(dest, op1, op2);
                break;
            }
            case OP_PRINT: {
                READ_REGISTER()->println();
                break;
            }
            case OP_EXIT: { return; }
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
    this->frames->allocate_registers(this->program.main_registers);
    // Set the first opcode
    this->program_counter = &this->program.program.code[0];
    // Run the compiled code.
    logger->info("Started interpreting...");
    this->run();
    logger->success("Finished interpreting!");
    // Clear the main frame registers
    this->frames->free_registers();
}

void VirtualMachine::reset()
{

}
