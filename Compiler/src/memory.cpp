#include "../include/memory.hpp"

void Memory::dump()
{
    // for (size_t i = 0; i < this->code->size(); i++) { printf("%llu ", this->code->at(i)); } printf("\n");
    for (size_t i = 0; i < this->code.size(); i++) {
        // Get the opcode
        opcode_t opcode = this->code[i];
        // Print the opcode
        print_opcode(opcode);
        // Get the opcode operands
        std::vector<OpCodeType> *operands = opcode_operands(opcode);
        // Print the opcode operands
        for (uint8_t k = 0; k < 3 && operands->at(k) != OT_NONE; k++) {
            switch (operands->at(k)) {
                case OT_REG: { printf(" R%04llu", this->code[++i]); break; }
                case OT_CONST: { printf(" C%04llu", this->code[++i]); break; }
                case OT_GLOBAL: { printf(" G%04llu", this->code[++i]); break; }
                case OT_LITERAL: { printf(" %05llu", this->code[++i]); break; }
                case OT_NONE: { /* Never happens */ }
            }
        }
        // Print a new line.
        printf("\n");
    }
}
