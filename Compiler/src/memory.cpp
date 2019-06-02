#include "../include/memory.hpp"

void Memory::dump()
{
    // for (size_t i = 0; i < this->code->size(); i++) { printf("%llu ", this->code->at(i)); } printf("\n");
    for (size_t i = 0; i < this->code.size(); i++) {
        // Get the opcode
        opcode_t opcode = this->code[i];
        // Print the opcode
        print_opcode(opcode);
        // Print the opcode operands
        for (const OpCodeType &ot : *opcode_operands(opcode)) {
            switch (ot) {
                case OT_REG: { printf(" R-%05zu", this->code[++i]); break; }
                case OT_CONST: { printf(" C-%05zu", this->code[++i]); break; }
                case OT_GLOBAL: { printf(" G-%05zu", this->code[++i]); break; }
                case OT_LITERAL: { printf(" L-%05zu", this->code[++i]); break; }
                case OT_PROP: { printf(" P-%05zu", this->code[++i]); break; }
            }
        }
        // Print a new line.
        printf("\n");
    }
}
