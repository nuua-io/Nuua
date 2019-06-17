#include "../include/virtual_machine.hpp"
#include "../../Compiler/include/value.hpp"
#include "../../Logger/include/logger.hpp"
#include <math.h>

#define BASE_PC (&this->program->memory->code.front())
#define END_PC (&this->program->memory->code.back())
#define PC (this->program_counter)
#define PC_AT(at) (PC + at)
#define GLOBAL(at) (this->program->main_frame.registers.get() + *PC_AT(at))
#define LITERAL(at) (*PC_AT(at))
#define REGISTER(at) (this->active_frame->registers.get() + *PC_AT(at))
#define CONSTANT(at) (this->program->memory->constants.data() + *PC_AT(at))
#define PROP(object_at, prop_at) (GETV(REGISTER(object_at)->value, std::shared_ptr<nobject_t>)->registers.get() + *PC_AT(prop_at))
#define INC_PC(num) (PC += num)
#define PUSH(value_ptr) ((value_ptr)->copy_to(this->top_stack++))
#define POP(value_ptr) (--this->top_stack)->copy_to(value_ptr)
#define CRASH(msg) logger->add_entity(this->current_file(), this->current_line(), this->current_column(), msg); exit(logger->crash())

// Safe cheks
#define CHECK_OBJECT(object_at) if (!GETV(REGISTER(object_at)->value, std::shared_ptr<nobject_t>)) { \
    CRASH("Segmentation fault: Uninitialized object '" + REGISTER(object_at)->to_string() + "'."); }
#define CHECK_FUN(fun_at) if (!GETV(REGISTER(fun_at)->value, std::shared_ptr<nfun_t>)) { \
    CRASH("Segmentation fault: Uninitialized function '" + REGISTER(fun_at)->to_string() + "'."); }

void VirtualMachine::run()
{
    for (;;) {
        // printf("=> %llu (%llu)\n", PC, *PC);
        switch (*PC) {
            case OP_EXIT: { return; }
            case OP_MOVE: { REGISTER(2)->copy_to(REGISTER(1)); INC_PC(3); break; }
            case OP_LOAD_C: { CONSTANT(2)->copy_to(REGISTER(1)); INC_PC(3); break; }
            case OP_LOAD_G: { GLOBAL(2)->copy_to(REGISTER(1)); INC_PC(3); break; }
            case OP_SET_G: { REGISTER(2)->copy_to(GLOBAL(1)); INC_PC(3); break; }
            case OP_PUSH: { PUSH(REGISTER(1)); INC_PC(2); break; }
            case OP_PUSH_C: { PUSH(CONSTANT(1)); INC_PC(2); break; }
            case OP_POP: { POP(REGISTER(1)); INC_PC(2); break; }
            case OP_SGET: {
                REGISTER(2)->type.copy_to(REGISTER(1)->type);
                const nint_t &index = GETV(REGISTER(3)->value, nint_t);
                const nstring_t &str = GETV(REGISTER(2)->value, nstring_t);
                if (index < 0 || static_cast<size_t>(index) > str.length()) {
                    CRASH("Index out of range. The index at this point of execution must be between [0, " + std::to_string(str.length()) + "]");
                }
                REGISTER(1)->value = std::string(1, str[index]);
                INC_PC(4);
                break;
            }
            case OP_SSET: {
                nstring_t &target = GETV(REGISTER(1)->value, nstring_t);
                const nint_t &index = GETV(REGISTER(2)->value, nint_t);
                const nstring_t &value = GETV(REGISTER(3)->value, nstring_t);
                if (index < 0 || static_cast<size_t>(index) > target.length()) {
                    CRASH("Index out of range. The index at this point of execution must be between [0, " + std::to_string(target.length()) + "]");
                }
                target[index] = value[0];
                INC_PC(4);
                break;
            }
            case OP_SDELETE: {
                GETV(REGISTER(1)->value, nstring_t).erase(GETV(REGISTER(2)->value, nint_t), 1);
                INC_PC(3);
                break;
            }
            case OP_LPUSH: {
                GETV(REGISTER(1)->value, std::shared_ptr<nlist_t>)->push_back(*REGISTER(2));
                INC_PC(3);
                break;
            }
            case OP_LPUSH_C: {
                GETV(REGISTER(1)->value, std::shared_ptr<nlist_t>)->push_back(*CONSTANT(2));
                INC_PC(3);
                break;
            }
            case OP_LPOP: { break; }
            case OP_LGET: {
                REGISTER(2)->type.inner_type->copy_to(REGISTER(1)->type);
                nint_t index = GETV(REGISTER(3)->value, nint_t);
                const std::shared_ptr<nlist_t> &list = GETV(REGISTER(2)->value, std::shared_ptr<nlist_t>);
                if (index < 0 || static_cast<size_t>(index) > list->size()) {
                    CRASH("Index out of range. The index at this point of execution must be between [0, " + std::to_string(list->size()) + "]");
                }
                (*list)[index].copy_to(REGISTER(1));
                INC_PC(4);
                break;
            }
            case OP_LSET: {
                const std::shared_ptr<nlist_t> &list = GETV(REGISTER(1)->value, std::shared_ptr<nlist_t>);
                const nint_t &index = GETV(REGISTER(2)->value, nint_t);
                if (index < 0 || static_cast<size_t>(index) > list->size()) {
                    CRASH("Index out of range. The index at this point of execution must be between [0, " + std::to_string(list->size()) + "]");
                }
                REGISTER(3)->copy_to(&list->at(index));
                INC_PC(4);
                break;
            }
            case OP_LDELETE: {
                std::shared_ptr<nlist_t> &target = GETV(REGISTER(1)->value, std::shared_ptr<nlist_t>);
                target->erase(target->begin() + GETV(REGISTER(2)->value, nint_t));
                INC_PC(3);
                break;
            }
            case OP_DKEY: {
                const std::shared_ptr<ndict_t> &d = GETV(REGISTER(2)->value, std::shared_ptr<ndict_t>);
                const nint_t &index = GETV(REGISTER(3)->value, nint_t);
                REGISTER(1)->value = d->key_order[index];
                REGISTER(1)->retype(VALUE_STRING);
                INC_PC(4);
                break;
            }
            case OP_DGET: {
                const std::shared_ptr<ndict_t> &d = GETV(REGISTER(2)->value, std::shared_ptr<ndict_t>);
                const nstring_t &key = GETV(REGISTER(3)->value, nstring_t);
                REGISTER(2)->type.inner_type->copy_to(REGISTER(1)->type);
                // REGISTER(1)->value = d.values.at(key);
                d->values.at(key).copy_to(REGISTER(1));
                INC_PC(4);
                break;
            }
            case OP_DSET: {
                break;
            }
            case OP_DDELETE: {
                std::shared_ptr<ndict_t> &target = GETV(REGISTER(1)->value, std::shared_ptr<ndict_t>);
                target->values.erase(GETV(REGISTER(2)->value, nstring_t));
                INC_PC(3);
                break;
            }
            case OP_CALL: {
                CHECK_FUN(1);
                const std::shared_ptr<nfun_t> &r = GETV(REGISTER(1)->value, std::shared_ptr<nfun_t>);
                // Set the new frame and allocate it's registers.
                (++this->active_frame)->setup(r->registers, PC + 2);
                // Change the program counter.
                PC = BASE_PC + r->index;
                // printf("Frame: %llu\n", ++frame);
                break;
            }
            case OP_RETURN: {
				// Delete the allocated memory
				this->active_frame->free_registers();
                // Drop the frame and reset the PC position.
                PC = (this->active_frame--)->return_address;
                break;
            }
            case OP_LPROP: {
                CHECK_OBJECT(2);
                PROP(2, 3)->copy_to(REGISTER(1));
                INC_PC(4);
                break;
            }
            case OP_SPROP: {
                CHECK_OBJECT(2);
                REGISTER(3)->copy_to(PROP(2, 1));
                INC_PC(4);
                break;
            }
            case OP_CAST_INT_FLOAT: {
                REGISTER(1)->value = static_cast<nfloat_t>(GETV(REGISTER(2)->value, nint_t));
                REGISTER(1)->retype(VALUE_FLOAT);
                INC_PC(3);
                break;
            }
            case OP_CAST_INT_BOOL: {
                REGISTER(1)->value = static_cast<nbool_t>(GETV(REGISTER(2)->value, nint_t));
                REGISTER(1)->retype(VALUE_BOOL);
                INC_PC(3);
                break;
            }
            case OP_CAST_INT_STRING: {
                REGISTER(1)->value = REGISTER(2)->to_string();
                REGISTER(1)->retype(VALUE_STRING);
                INC_PC(3);
                break;
            }
            case OP_CAST_FLOAT_INT: {
                REGISTER(1)->value = static_cast<nint_t>(GETV(REGISTER(2)->value, nfloat_t));
                REGISTER(1)->retype(VALUE_INT);
                INC_PC(3);
                break;
            }
            case OP_CAST_FLOAT_BOOL: {
                REGISTER(1)->value = static_cast<nbool_t>(GETV(REGISTER(2)->value, nfloat_t));
                REGISTER(1)->retype(VALUE_BOOL);
                INC_PC(3);
                break;
            }
            case OP_CAST_FLOAT_STRING: {
                REGISTER(1)->value = REGISTER(2)->to_string();
                REGISTER(1)->retype(VALUE_STRING);
                INC_PC(3);
                break;
            }
            case OP_CAST_BOOL_INT: {
                REGISTER(1)->value = static_cast<nint_t>(GETV(REGISTER(2)->value, nbool_t));
                REGISTER(1)->retype(VALUE_INT);
                INC_PC(3);
                break;
            }
            case OP_CAST_BOOL_FLOAT: {
                REGISTER(1)->value = static_cast<nfloat_t>(GETV(REGISTER(2)->value, nbool_t));
                REGISTER(1)->retype(VALUE_FLOAT);
                INC_PC(3);
                break;
            }
            case OP_CAST_BOOL_STRING: {
                REGISTER(1)->value = REGISTER(2)->to_string();
                REGISTER(1)->retype(VALUE_STRING);
                INC_PC(3);
                break;
            }
            case OP_CAST_LIST_STRING: {
                REGISTER(1)->value = REGISTER(2)->to_string();
                REGISTER(1)->retype(VALUE_STRING);
                INC_PC(3);
                break;
            }
            case OP_CAST_LIST_BOOL: {
                REGISTER(1)->value = static_cast<nbool_t>(GETV(REGISTER(2)->value, std::shared_ptr<nlist_t>)->size() != 0);
                REGISTER(1)->retype(VALUE_BOOL);
                INC_PC(3);
                break;
            }
            case OP_CAST_LIST_INT: {
                REGISTER(1)->value = static_cast<nint_t>(GETV(REGISTER(2)->value, std::shared_ptr<nlist_t>)->size());
                REGISTER(1)->retype(VALUE_INT);
                INC_PC(3);
                break;
            }
            case OP_CAST_DICT_STRING: {
                REGISTER(1)->value = REGISTER(2)->to_string();
                REGISTER(1)->retype(VALUE_STRING);
                INC_PC(3);
                break;
            }
            case OP_CAST_DICT_BOOL: {
                REGISTER(1)->value = GETV(REGISTER(2)->value, std::shared_ptr<ndict_t>)->values.size() != 0;
                REGISTER(1)->retype(VALUE_BOOL);
                INC_PC(3);
                break;
            }
            case OP_CAST_DICT_INT: {
                REGISTER(1)->value = static_cast<nint_t>(GETV(REGISTER(2)->value, std::shared_ptr<ndict_t>)->values.size());
                REGISTER(1)->retype(VALUE_INT);
                INC_PC(3);
                break;
            }
            case OP_CAST_STRING_BOOL: {
                REGISTER(1)->value = GETV(REGISTER(2)->value, nstring_t).length() != 0;
                REGISTER(1)->retype(VALUE_BOOL);
                INC_PC(3);
                break;
            }
            case OP_CAST_STRING_INT: {
                REGISTER(1)->value = static_cast<nint_t>(GETV(REGISTER(2)->value, nstring_t).length());
                REGISTER(1)->retype(VALUE_INT);
                INC_PC(3);
                break;
            }
            case OP_NEG_BOOL: {
                REGISTER(1)->value = !GETV(REGISTER(2)->value, nbool_t);
                REGISTER(1)->retype(VALUE_BOOL);
                INC_PC(3);
                break;
            }
            case OP_MINUS_INT: {
                REGISTER(1)->value = -GETV(REGISTER(2)->value, nint_t);
                REGISTER(1)->retype(VALUE_INT);
                INC_PC(3);
                break;
            }
            case OP_MINUS_FLOAT: {
                REGISTER(1)->value = -GETV(REGISTER(2)->value, nfloat_t);
                REGISTER(1)->retype(VALUE_FLOAT);
                INC_PC(3);
                break;
            }
            case OP_MINUS_BOOL: {
                REGISTER(1)->value = -static_cast<nint_t>(GETV(REGISTER(2)->value, nbool_t));
                REGISTER(1)->retype(VALUE_INT);
                INC_PC(3);
                break;
            }
            case OP_PLUS_INT: {
                REGISTER(1)->value = +GETV(REGISTER(2)->value, nint_t);
                REGISTER(1)->retype(VALUE_INT);
                INC_PC(3);
                break;
            }
            case OP_PLUS_FLOAT: {
                REGISTER(1)->value = +GETV(REGISTER(2)->value, nfloat_t);
                REGISTER(1)->retype(VALUE_FLOAT);
                INC_PC(3);
                break;
            }
            case OP_PLUS_BOOL: {
                REGISTER(1)->value = +static_cast<nint_t>(GETV(REGISTER(2)->value, nbool_t));
                REGISTER(1)->retype(VALUE_INT);
                INC_PC(3);
                break;
            }
            case OP_IINC: { ++GETV(REGISTER(1)->value, nint_t); INC_PC(2); break; }
            case OP_IDEC: { ++GETV(REGISTER(1)->value, nint_t); INC_PC(2); break; }
            case OP_ADD_INT: {
                REGISTER(1)->value = GETV(REGISTER(2)->value, nint_t) + GETV(REGISTER(3)->value, nint_t);
                REGISTER(1)->retype(VALUE_INT);
                INC_PC(4);
                break;
            }
            case OP_ADD_FLOAT: {
                REGISTER(1)->value = GETV(REGISTER(2)->value, nfloat_t) + GETV(REGISTER(3)->value, nfloat_t);
                REGISTER(1)->retype(VALUE_FLOAT);
                INC_PC(4);
                break;
            }
            case OP_ADD_STRING: {
                REGISTER(1)->value = GETV(REGISTER(2)->value, nstring_t) + GETV(REGISTER(3)->value, nstring_t);
                REGISTER(1)->retype(VALUE_STRING);
                INC_PC(4);
                break;
            }
            case OP_ADD_BOOL: {
                REGISTER(1)->value = static_cast<nint_t>(GETV(REGISTER(2)->value, nbool_t) + GETV(REGISTER(3)->value, nbool_t));
                REGISTER(1)->retype(VALUE_INT);
                INC_PC(4);
                break;
            }
            case OP_ADD_LIST: {
                break;
            }
            case OP_ADD_DICT: {
                break;
            }
            case OP_SUB_INT: {
                REGISTER(1)->value = GETV(REGISTER(2)->value, nint_t) - GETV(REGISTER(3)->value, nint_t);
                REGISTER(1)->retype(VALUE_INT);
                INC_PC(4);
                break;
            }
            case OP_SUB_FLOAT: {
                REGISTER(1)->value = GETV(REGISTER(2)->value, nfloat_t) - GETV(REGISTER(3)->value, nfloat_t);
                REGISTER(1)->retype(VALUE_INT);
                INC_PC(4);
                break;
            }
            case OP_SUB_BOOL: {
                REGISTER(1)->value = static_cast<nint_t>(GETV(REGISTER(2)->value, nbool_t) * GETV(REGISTER(3)->value, nbool_t));
                REGISTER(1)->retype(VALUE_INT);
                INC_PC(4);
                break;
            }
            case OP_MUL_INT: {
                REGISTER(1)->value = static_cast<nint_t>(GETV(REGISTER(2)->value, nint_t) * GETV(REGISTER(3)->value, nint_t));
                REGISTER(1)->retype(VALUE_INT);
                INC_PC(4);
                break;
            }
            case OP_MUL_FLOAT: {
                REGISTER(1)->value = static_cast<nfloat_t>(GETV(REGISTER(2)->value, nfloat_t) * GETV(REGISTER(3)->value, nfloat_t));
                REGISTER(1)->retype(VALUE_FLOAT);
                INC_PC(4);
                break;
            }
            case OP_MUL_BOOL: {
                REGISTER(1)->value = static_cast<nint_t>(GETV(REGISTER(2)->value, nbool_t) * GETV(REGISTER(3)->value, nbool_t));
                REGISTER(1)->retype(VALUE_INT);
                INC_PC(4);
                break;
            }
            case OP_MUL_INT_STRING: {
                break;
            }
            case OP_MUL_STRING_INT: {
                break;
            }
            case OP_MUL_INT_LIST: {
                break;
            }
            case OP_MUL_LIST_INT: {
                break;
            }
            case OP_DIV_INT: {
                nint_t divident = GETV(REGISTER(2)->value, nint_t);
                nint_t divisor = GETV(REGISTER(3)->value, nint_t);
				if (divisor == 0) { CRASH("Division by 0 -> " + std::to_string(divident) + " / " + std::to_string(divisor)); }
                REGISTER(1)->value = static_cast<nfloat_t>(divident / divisor);
                REGISTER(1)->retype(VALUE_FLOAT);
                INC_PC(4);
                break;
            }
            case OP_DIV_FLOAT: {
                nfloat_t divident = GETV(REGISTER(2)->value, nfloat_t);
                nfloat_t divisor = GETV(REGISTER(3)->value, nfloat_t);
				if (divisor == 0) { CRASH("Division by 0 -> " + std::to_string(divident) + " / " + std::to_string(divisor)); }
                REGISTER(1)->value = static_cast<nfloat_t>(divident / divisor);
                REGISTER(1)->retype(VALUE_FLOAT);
                INC_PC(4);
                break;
            }
            case OP_DIV_STRING_INT: {
                break;
            }
            case OP_DIV_LIST_INT: {
                break;
            }
            case OP_EQ_INT: {
                REGISTER(1)->value = GETV(REGISTER(2)->value, nint_t) == GETV(REGISTER(3)->value, nint_t);
                REGISTER(1)->retype(VALUE_BOOL);
                INC_PC(4);
                break;
            }
            case OP_EQ_FLOAT: {
                REGISTER(1)->value = GETV(REGISTER(2)->value, nfloat_t) == GETV(REGISTER(3)->value, nfloat_t);
                REGISTER(1)->retype(VALUE_BOOL);
                INC_PC(4);
                break;
            }
            case OP_EQ_STRING: {
                REGISTER(1)->value = GETV(REGISTER(2)->value, nstring_t).compare(GETV(REGISTER(3)->value, nstring_t)) == 0;
                REGISTER(1)->retype(VALUE_BOOL);
                INC_PC(4);
                break;
            }
            case OP_EQ_BOOL: {
                REGISTER(1)->value = GETV(REGISTER(2)->value, nbool_t) == GETV(REGISTER(3)->value, nbool_t);
                REGISTER(1)->retype(VALUE_BOOL);
                INC_PC(4);
                break;
            }
            case OP_EQ_LIST: {
                REGISTER(1)->value = REGISTER(2)->same_as(*REGISTER(3));
                REGISTER(1)->retype(VALUE_BOOL);
                INC_PC(4);
                break;
            }
            case OP_EQ_DICT: {
                REGISTER(1)->value = REGISTER(2)->same_as(*REGISTER(3));
                REGISTER(1)->retype(VALUE_BOOL);
                INC_PC(4);
                break;
            }
            case OP_NEQ_INT: {
                REGISTER(1)->value = GETV(REGISTER(2)->value, nint_t) != GETV(REGISTER(3)->value, nint_t);
                REGISTER(1)->retype(VALUE_BOOL);
                INC_PC(4);
                break;
            }
            case OP_NEQ_FLOAT: {
                REGISTER(1)->value = GETV(REGISTER(2)->value, nfloat_t) != GETV(REGISTER(3)->value, nfloat_t);
                REGISTER(1)->retype(VALUE_BOOL);
                INC_PC(4);
                break;
            }
            case OP_NEQ_STRING: {
                REGISTER(1)->value = GETV(REGISTER(2)->value, nstring_t).compare(GETV(REGISTER(3)->value, nstring_t)) != 0;
                REGISTER(1)->retype(VALUE_BOOL);
                INC_PC(4);
                break;
            }
            case OP_NEQ_BOOL: {
                REGISTER(1)->value = GETV(REGISTER(2)->value, nbool_t) != GETV(REGISTER(3)->value, nbool_t);
                REGISTER(1)->retype(VALUE_BOOL);
                INC_PC(4);
                break;
            }
            case OP_NEQ_LIST: {
                REGISTER(1)->value = !REGISTER(2)->same_as(*REGISTER(3));
                REGISTER(1)->retype(VALUE_BOOL);
                INC_PC(4);
                break;
            }
            case OP_NEQ_DICT: {
                REGISTER(1)->value = !REGISTER(2)->same_as(*REGISTER(3));
                REGISTER(1)->retype(VALUE_BOOL);
                INC_PC(4);
                break;
            }
            case OP_HT_INT: {
                REGISTER(1)->value = GETV(REGISTER(2)->value, nint_t) > GETV(REGISTER(3)->value, nint_t);
                REGISTER(1)->retype(VALUE_BOOL);
                INC_PC(4);
                break;
            }
            case OP_HT_FLOAT: {
                REGISTER(1)->value = GETV(REGISTER(2)->value, nfloat_t) > GETV(REGISTER(3)->value, nfloat_t);
                REGISTER(1)->retype(VALUE_BOOL);
                INC_PC(4);
                break;
            }
            case OP_HT_STRING: {
                REGISTER(1)->value = GETV(REGISTER(2)->value, nstring_t) > GETV(REGISTER(3)->value, nstring_t);
                REGISTER(1)->retype(VALUE_BOOL);
                INC_PC(4);
                break;
            }
            case OP_HT_BOOL: {
                REGISTER(1)->value = GETV(REGISTER(2)->value, nbool_t) > GETV(REGISTER(3)->value, nbool_t);
                REGISTER(1)->retype(VALUE_BOOL);
                INC_PC(4);
                break;
            }
            case OP_HTE_INT: {
                REGISTER(1)->value = GETV(REGISTER(2)->value, nint_t) >= GETV(REGISTER(3)->value, nint_t);
                REGISTER(1)->retype(VALUE_BOOL);
                INC_PC(4);
                break;
            }
            case OP_HTE_FLOAT: {
                REGISTER(1)->value = GETV(REGISTER(2)->value, nfloat_t) >= GETV(REGISTER(3)->value, nfloat_t);
                REGISTER(1)->retype(VALUE_BOOL);
                INC_PC(4);
                break;
            }
            case OP_HTE_STRING: {
                REGISTER(1)->value = GETV(REGISTER(2)->value, nstring_t) >= GETV(REGISTER(3)->value, nstring_t);
                REGISTER(1)->retype(VALUE_BOOL);
                INC_PC(4);
                break;
            }
            case OP_HTE_BOOL: {
                REGISTER(1)->value = GETV(REGISTER(2)->value, nbool_t) >= GETV(REGISTER(3)->value, nbool_t);
                REGISTER(1)->retype(VALUE_BOOL);
                INC_PC(4);
                break;
            }
            case OP_LT_INT: {
                REGISTER(1)->value = GETV(REGISTER(2)->value, nint_t) < GETV(REGISTER(3)->value, nint_t);
                REGISTER(1)->retype(VALUE_BOOL);
                INC_PC(4);
                break;
            }
            case OP_LT_FLOAT: {
                REGISTER(1)->value = GETV(REGISTER(2)->value, nfloat_t) < GETV(REGISTER(3)->value, nfloat_t);
                REGISTER(1)->retype(VALUE_BOOL);
                INC_PC(4);
                break;
            }
            case OP_LT_STRING: {
                REGISTER(1)->value = GETV(REGISTER(2)->value, nstring_t) < GETV(REGISTER(3)->value, nstring_t);
                REGISTER(1)->retype(VALUE_BOOL);
                INC_PC(4);
                break;
            }
            case OP_LT_BOOL: {
                REGISTER(1)->value = GETV(REGISTER(2)->value, nbool_t) < GETV(REGISTER(3)->value, nbool_t);
                REGISTER(1)->retype(VALUE_BOOL);
                INC_PC(4);
                break;
            }
            case OP_LTE_INT: {
                REGISTER(1)->value = GETV(REGISTER(2)->value, nint_t) <= GETV(REGISTER(3)->value, nint_t);
                REGISTER(1)->retype(VALUE_BOOL);
                INC_PC(4);
                break;
            }
            case OP_LTE_FLOAT: {
                REGISTER(1)->value = GETV(REGISTER(2)->value, nfloat_t) <= GETV(REGISTER(3)->value, nfloat_t);
                REGISTER(1)->retype(VALUE_BOOL);
                INC_PC(4);
                break;
            }
            case OP_LTE_STRING: {
                REGISTER(1)->value = GETV(REGISTER(2)->value, nstring_t) <= GETV(REGISTER(3)->value, nstring_t);
                REGISTER(1)->retype(VALUE_BOOL);
                INC_PC(4);
                break;
            }
            case OP_LTE_BOOL: {
                REGISTER(1)->value = GETV(REGISTER(2)->value, nbool_t) <= GETV(REGISTER(3)->value, nbool_t);
                REGISTER(1)->retype(VALUE_BOOL);
                INC_PC(4);
                break;
            }
            case OP_OR: {
                REGISTER(1)->value = GETV(REGISTER(2)->value, nbool_t) || GETV(REGISTER(3)->value, nbool_t);
                REGISTER(1)->retype(VALUE_BOOL);
                INC_PC(4);
                break;
            }
            case OP_AND: {
                REGISTER(1)->value = GETV(REGISTER(2)->value, nbool_t) && GETV(REGISTER(3)->value, nbool_t);
                REGISTER(1)->retype(VALUE_BOOL);
                INC_PC(4);
                break;
            }
            case OP_FJUMP: { PC += LITERAL(1); break; }
            case OP_BJUMP: { PC -= LITERAL(1); break; }
            case OP_CFJUMP: {
                if (GETV(REGISTER(2)->value, nbool_t)) { PC += LITERAL(1); break; }
                INC_PC(3);
                break;
            }
            case OP_CBJUMP: {
                if (GETV(REGISTER(2)->value, nbool_t)) { PC -= LITERAL(1); break; }
                INC_PC(3);
                break;
            }
            case OP_CFNJUMP: {
                if (!GETV(REGISTER(2)->value, nbool_t)) { PC += LITERAL(1); break; }
                INC_PC(3);
                break;
            }
            case OP_CBNJUMP: {
                if (!GETV(REGISTER(2)->value, nbool_t)) { PC -= LITERAL(1); break; }
                INC_PC(3);
                break;
            }
            case OP_SSLICE: {
                /*
                const nstring_t &target = GETV(REGISTER(2)->value, nstring_t);
                const nint_t k = GETV(REGISTER(5)->value, nint_t); // The step index.
                printf("*\n");
                if (k == 0) { CRASH("The 'step' in a slice can't be 0."); }
                printf("*\n");
                nint_t i = GETV(REGISTER(2)->value, nint_t); // The start index.
                if (k > 0 && i > target.length()) i = target.length();
                else if (k < 0 && i > target.length()) i = target.length() - 1;
                nint_t j = GETV(REGISTER(4)->value, nint_t); // The end index.
                if (k > 0 && j > target.length()) j = target.length();
                else if (k < 0 && j > target.length()) j = target.length() - 1;
                */
                std::string str;
                // for (size_t n = 0; (i + n * k) < j; ++n) str += (*target)[i + n * k];
                REGISTER(1)->value = str;
                REGISTER(1)->retype(VALUE_STRING);
                INC_PC(6);
                break;
            }
            case OP_SSLICEE: {
                break;
            }
            case OP_LSLICE: {
                break;
            }
            case OP_LSLICEE: {
                break;
            }
            case OP_RANGEE: {
                REGISTER(1)->value = std::make_shared<nlist_t>();
                for (int64_t i = GETV(REGISTER(2)->value, nint_t); i < GETV(REGISTER(3)->value, nint_t); i++) {
                    GETV(REGISTER(1)->value, std::shared_ptr<nlist_t>)->push_back({ i });
                }
                REGISTER(1)->retype(VALUE_LIST, std::make_shared<Type>(VALUE_INT));
                INC_PC(4);
                break;
            }
            case OP_RANGEI: {
                REGISTER(1)->value = std::make_shared<nlist_t>();
                for (int64_t i = GETV(REGISTER(2)->value, nint_t); i <= GETV(REGISTER(3)->value, nint_t); i++) {
                    GETV(REGISTER(1)->value, std::shared_ptr<nlist_t>)->push_back({ i });
                }
                REGISTER(1)->retype(VALUE_LIST, std::make_shared<Type>(VALUE_INT));
                INC_PC(4);
                break;
            }
            case OP_PRINT: { printf("%s\n", REGISTER(1)->to_string().c_str()); INC_PC(2); break; }
            case OP_PRINT_C: { printf("%s\n", CONSTANT(1)->to_string().c_str()); INC_PC(2); break; }
        }
    }
}

void VirtualMachine::interpret(const char *file, const std::vector<std::string> &argv)
{
    // Compile the code.
    Compiler compiler = Compiler(this->program);
    reg_t main = compiler.compile(file);
    // Call the main function.
    const std::shared_ptr<nfun_t> &callee = GETV((this->program->main_frame.registers.get() + main)->value, std::shared_ptr<nfun_t>);
    // Push the argv of the main function.
    nlist_t args;
    for (const std::string arg : argv) {
        args.push_back({ arg });
    }
    Value av = { args, std::make_shared<Type>(VALUE_STRING) };
    PUSH(&av);
    // Set the new frame and allocate it's registers.
    (++this->active_frame)->setup(callee->registers, END_PC);
    // Change the program counter.
    PC = BASE_PC + callee->index;
    // Run the compiled code.
    this->run();
}

/*
void VirtualMachine::reset()
{

}
*/

std::shared_ptr<const std::string> VirtualMachine::current_file()
{
    std::pair<size_t, std::shared_ptr<const std::string>> result = { 0, std::shared_ptr<const std::string>() };
    for (const auto &el : this->program->memory->files) {
        if (!result.second) result = el;
        else if (el.first > result.first && el.first <= static_cast<size_t>(PC - BASE_PC)) result = el;
    }
    return result.second;
}

line_t VirtualMachine::current_line()
{
    std::pair<size_t, line_t> result = { 0, 0 };
    for (const auto &el : this->program->memory->lines) {
        if (result.first == 0 && result.second == 0) result = el;
        else if (el.first > result.first && el.first <= static_cast<size_t>(PC - BASE_PC)) result = el;
    }
    return result.second;
}

column_t VirtualMachine::current_column()
{
    std::pair<size_t, column_t> result = { 0, 0 };
    for (const auto &el : this->program->memory->columns) {
        if (result.first == 0 && result.second == 0) result = el;
        else if (el.first > result.first && el.first <= static_cast<size_t>(PC - BASE_PC)) result = el;
    }
    return result.second;
}

#undef PC
