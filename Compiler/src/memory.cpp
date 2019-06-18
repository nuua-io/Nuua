#include "../include/memory.hpp"

void Memory::dump()
{
    // for (size_t i = 0; i < this->code->size(); i++) { printf("%llu ", this->code->at(i)); } printf("\n");
    for (size_t i = 0; i < this->code.size(); i++) {
        // Get the opcode
        opcode_t opcode = this->code[i];
        // Print the opcode
        print_opcode(opcode);
        // printf("Opcode: %d\n, size: %d\n", opcode, opcode_operands(opcode)->size());
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

void Memory::show_refs()
{
    printf("File references:\n");
    for (const auto &[key, value] : this->files) {
        printf("%5zu: %s\n", key, value->c_str());
    }
    printf("\nLine references:\n");
    for (const auto &[key, value] : this->lines) {
        printf("%5zu: %d\n", key, static_cast<size_t>(value));
    }
    printf("\nColumn references:\n");
    for (const auto &[key, value] : this->columns) {
        printf("%5zu: %zu\n", key, static_cast<size_t>(value));
    }
}
