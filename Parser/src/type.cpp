#include "../include/type.hpp"
#include "../../Logger/include/logger.hpp"

const std::unordered_map<std::string, ValueType> Type::value_types = {
    { "int", VALUE_INT },
    { "float", VALUE_FLOAT },
    { "bool", VALUE_BOOL },
    { "string", VALUE_STRING },
};

const std::vector<std::string> Type::types_string = {
    "VALUE_INT", "VALUE_FLOAT", "VALUE_BOOL",
    "VALUE_STRING", "VALUE_LIST", "VALUE_DICT", "VALUE_FUN",
    "VALUE_CLASS", "VALUE_NO_TYPE"
};

Type::Type(std::string name)
{
    if (Type::value_types.find(name) != Type::value_types.end()) {
        this->type = Type::value_types.at(name);
    } else {
        this->type = VALUE_CLASS;
        this->class_name = name;
    }
}

bool Type::cast(Type *to, CastType *dest_casttype)
{
    #define DEST_CASTTYPE(t) if (dest_casttype) *dest_casttype = t
    switch (this->type) {
        case VALUE_INT: {
            switch (to->type) {
                case VALUE_FLOAT: { DEST_CASTTYPE(CAST_INT_FLOAT); break; }
                case VALUE_BOOL: { DEST_CASTTYPE(CAST_INT_BOOL); break; }
                case VALUE_STRING: { DEST_CASTTYPE(CAST_INT_STRING); break; }
                default: { return false; }
            }
            break;
        }
        case VALUE_FLOAT: {
            switch (to->type) {
                case VALUE_INT: { DEST_CASTTYPE(CAST_FLOAT_INT); break; }
                case VALUE_BOOL: { DEST_CASTTYPE(CAST_FLOAT_BOOL); break; }
                case VALUE_STRING: { DEST_CASTTYPE(CAST_FLOAT_STRING); break; }
                default: { return false; }
            }
            break;
        }
        case VALUE_BOOL: {
            switch (to->type) {
                case VALUE_INT: { DEST_CASTTYPE(CAST_BOOL_INT); break; }
                case VALUE_FLOAT: { DEST_CASTTYPE(CAST_BOOL_FLOAT); break; }
                case VALUE_STRING: { DEST_CASTTYPE(CAST_BOOL_STRING); break; }
                default: { return false; }
            }
            break;
        }
        case VALUE_LIST: {
            switch (to->type) {
                case VALUE_BOOL: { DEST_CASTTYPE(CAST_LIST_BOOL); break; }
                case VALUE_STRING: { DEST_CASTTYPE(CAST_LIST_STRING); break; }
                default: { return false; }
            }
            break;
        }
        case VALUE_DICT: {
            switch (to->type) {
                case VALUE_BOOL: { DEST_CASTTYPE(CAST_DICT_BOOL); break; }
                case VALUE_STRING: { DEST_CASTTYPE(CAST_DICT_STRING); break; }
                default: { return false; }
            }
            break;
        }
        case VALUE_STRING: {
            switch (to->type) {
                case VALUE_BOOL: { DEST_CASTTYPE(CAST_STRING_BOOL); break; }
                default: { return false; }
            }
            break;
        }
        default: { return false; }
    }
    #undef DEST_CASTTYPE
    return true;
}

bool Type::unary(Token &op, Type *dest_type, UnaryType *dest_unarytype)
{
    #define DEST_TYPE(t) if (dest_type) dest_type->type = t
    #define DEST_UNARYTYPE(t) if (dest_unarytype) *dest_unarytype = t
    switch (op.type) {
        case TOKEN_BANG: {
            if (this->type == VALUE_BOOL) {
                DEST_TYPE(VALUE_BOOL);
                DEST_UNARYTYPE(UNARY_NEG_BOOL);
            } else return false;
            break;
        }
        case TOKEN_PLUS: {
            if (this->type == VALUE_INT) {
                DEST_TYPE(VALUE_INT);
                DEST_UNARYTYPE(UNARY_PLUS_INT);
            } else if (this->type == VALUE_FLOAT) {
                DEST_TYPE(VALUE_FLOAT);
                DEST_UNARYTYPE(UNARY_PLUS_FLOAT);
            } else if (this->type == VALUE_BOOL) {
                DEST_TYPE(VALUE_INT);
                DEST_UNARYTYPE(UNARY_PLUS_BOOL);
            } else return false;
            break;
        }
        case TOKEN_MINUS: {
            if (this->type == VALUE_INT) {
                DEST_TYPE(VALUE_INT);
                DEST_UNARYTYPE(UNARY_MINUS_INT);
            } else if (this->type == VALUE_FLOAT) {
                DEST_TYPE(VALUE_FLOAT);
                DEST_UNARYTYPE(UNARY_MINUS_FLOAT);
            } else if (this->type == VALUE_BOOL) {
                DEST_TYPE(VALUE_INT);
                DEST_UNARYTYPE(UNARY_MINUS_BOOL);
            } else return false;
            break;
        }
        default: {
            // Should never fire.
            return false;
        }
    }
    #undef DEST_TYPE
    #undef DEST_UNARYTYPE
    return true;
}

bool Type::binary(Token &op, Type *t1, Type *dest_type, BinaryType *dest_bintype)
{
    #define DEST_TYPE(t) if (dest_type) dest_type->type = t
    #define DEST_BINTYPE(t) if (dest_bintype) *dest_bintype = t
    switch (op.type) {
        case TOKEN_PLUS: {
            if (!this->same_as(t1)) return false;
            else if (this->type == VALUE_INT) {
                DEST_TYPE(VALUE_INT);
                DEST_BINTYPE(BINARY_ADD_INT);
            } else if (this->type == VALUE_FLOAT) {
                DEST_TYPE(VALUE_FLOAT);
                DEST_BINTYPE(BINARY_ADD_FLOAT);
            } else if (this->type == VALUE_STRING) {
                DEST_TYPE(VALUE_STRING);
                DEST_BINTYPE(BINARY_ADD_STRING);
            } else if (this->type == VALUE_BOOL) {
                DEST_TYPE(VALUE_INT);
                DEST_BINTYPE(BINARY_ADD_BOOL);
            } else if (this->type == VALUE_LIST) {
                if (dest_type) this->copy_to(dest_type);
                DEST_BINTYPE(BINARY_ADD_LIST);
            } else if (this->type == VALUE_DICT) {
                if (dest_type) this->copy_to(dest_type);
                DEST_BINTYPE(BINARY_ADD_DICT);
            } else return false;
            break;
        }
        case TOKEN_MINUS: {
            if (!this->same_as(t1)) return false;
            else if (this->type == VALUE_INT) {
                DEST_TYPE(VALUE_INT);
                DEST_BINTYPE(BINARY_SUB_INT);
            } else if (this->type == VALUE_FLOAT) {
                DEST_TYPE(VALUE_FLOAT);
                DEST_BINTYPE(BINARY_SUB_FLOAT);
            } else if (this->type == VALUE_BOOL) {
                DEST_TYPE(VALUE_INT);
                DEST_BINTYPE(BINARY_SUB_BOOL);
            } else return false;
            break;
        }
        case TOKEN_STAR: {
            if (!this->same_as(t1)) {
                // Check the special cases: int * string | string * int || int * list | list * int
                if (this->type == VALUE_INT && t1->type == VALUE_STRING) {
                    DEST_TYPE(VALUE_STRING);
                    DEST_BINTYPE(BINARY_MUL_INT_STRING);
                } else if (this->type == VALUE_STRING && t1->type == VALUE_INT) {
                    DEST_TYPE(VALUE_STRING);
                    DEST_BINTYPE(BINARY_MUL_STRING_INT);
                } else if (this->type == VALUE_INT && t1->type == VALUE_LIST) {
                    if (dest_type) t1->copy_to(dest_type);
                    DEST_BINTYPE(BINARY_MUL_INT_LIST);
                } else if (this->type == VALUE_LIST && t1->type == VALUE_INT) {
                    if (dest_type) this->copy_to(dest_type);
                    DEST_BINTYPE(BINARY_MUL_LIST_INT);
                } else return false;
            } else if (this->type == VALUE_INT) {
                DEST_TYPE(VALUE_INT);
                DEST_BINTYPE(BINARY_MUL_INT);
            } else if (this->type == VALUE_FLOAT) {
                DEST_TYPE(VALUE_FLOAT);
                DEST_BINTYPE(BINARY_MUL_FLOAT);
            } else if (this->type == VALUE_BOOL) {
                DEST_TYPE(VALUE_INT);
                DEST_BINTYPE(BINARY_MUL_BOOL);
            } else return false;
            break;
        } case TOKEN_SLASH: {
            if (!this->same_as(t1)) {
                if (this->type == VALUE_STRING && t1->type == VALUE_INT) {
                    if (dest_type) *dest_type = Type(VALUE_LIST, new Type(VALUE_STRING)); // Possible mem leak
                    DEST_BINTYPE(BINARY_DIV_STRING_INT);
                } else if (this->type == VALUE_LIST && t1->type == VALUE_INT) {
                    if (dest_type) *dest_type = Type(VALUE_LIST, new Type(*this)); // Possible mem leak
                    DEST_BINTYPE(BINARY_DIV_LIST_INT);
                } else return false;
            } else if (this->type == VALUE_INT) {
                DEST_TYPE(VALUE_FLOAT);
                DEST_BINTYPE(BINARY_DIV_INT);
            } else if (this->type == VALUE_FLOAT) {
                DEST_TYPE(VALUE_FLOAT);
                DEST_BINTYPE(BINARY_DIV_FLOAT);
            } else return false;
            break;
        }
        case TOKEN_EQUAL_EQUAL: {
            if (!this->same_as(t1)) return false;
            else if (this->type == VALUE_INT) {
                DEST_BINTYPE(BINARY_EQ_INT);
            } else if (this->type == VALUE_FLOAT) {
                DEST_BINTYPE(BINARY_EQ_FLOAT);
            } else if (this->type == VALUE_STRING) {
                DEST_BINTYPE(BINARY_EQ_STRING);
            } else if (this->type == VALUE_BOOL) {
                DEST_BINTYPE(BINARY_EQ_BOOL);
            } else if (this->type == VALUE_LIST) {
                DEST_BINTYPE(BINARY_EQ_LIST);
            } else if (this->type == VALUE_DICT) {
                DEST_BINTYPE(BINARY_EQ_DICT);
            } else return false;
            DEST_TYPE(VALUE_BOOL);
            break;
        }
        case TOKEN_BANG_EQUAL: {
            if (!this->same_as(t1)) return false;
            else if (this->type == VALUE_INT) {
                DEST_BINTYPE(BINARY_NEQ_INT);
            } else if (this->type == VALUE_FLOAT) {
                DEST_BINTYPE(BINARY_NEQ_FLOAT);
            } else if (this->type == VALUE_STRING) {
                DEST_BINTYPE(BINARY_NEQ_STRING);
            } else if (this->type == VALUE_BOOL) {
                DEST_BINTYPE(BINARY_NEQ_BOOL);
            } else if (this->type == VALUE_LIST) {
                DEST_BINTYPE(BINARY_NEQ_LIST);
            } else if (this->type == VALUE_DICT) {
                DEST_BINTYPE(BINARY_NEQ_DICT);
            } else return false;
            DEST_TYPE(VALUE_BOOL);
            break;
        }
        case TOKEN_HIGHER: {
            if (!this->same_as(t1)) return false;
            else if (this->type == VALUE_INT) {
                DEST_BINTYPE(BINARY_HT_INT);
            } else if (this->type == VALUE_FLOAT) {
                DEST_BINTYPE(BINARY_HT_FLOAT);
            } else if (this->type == VALUE_STRING) {
                DEST_BINTYPE(BINARY_HT_STRING);
            } else if (this->type == VALUE_BOOL) {
                DEST_BINTYPE(BINARY_HT_BOOL);
            } else return false;
            DEST_TYPE(VALUE_BOOL);
            break;
        }
        case TOKEN_HIGHER_EQUAL: {
            if (!this->same_as(t1)) return false;
            else if (this->type == VALUE_INT) {
                DEST_BINTYPE(BINARY_HTE_INT);
            } else if (this->type == VALUE_FLOAT) {
                DEST_BINTYPE(BINARY_HTE_FLOAT);
            } else if (this->type == VALUE_STRING) {
                DEST_BINTYPE(BINARY_HTE_STRING);
            } else if (this->type == VALUE_BOOL) {
                DEST_BINTYPE(BINARY_HTE_BOOL);
            } else return false;
            DEST_TYPE(VALUE_BOOL);
            break;
        }
        case TOKEN_LOWER: {
            if (!this->same_as(t1)) return false;
            else if (this->type == VALUE_INT) {
                DEST_BINTYPE(BINARY_LT_INT);
            } else if (this->type == VALUE_FLOAT) {
                DEST_BINTYPE(BINARY_LT_FLOAT);
            } else if (this->type == VALUE_STRING) {
                DEST_BINTYPE(BINARY_LT_STRING);
            } else if (this->type == VALUE_BOOL) {
                DEST_BINTYPE(BINARY_LT_BOOL);
            } else return false;
            DEST_TYPE(VALUE_BOOL);
            break;
        }
        case TOKEN_LOWER_EQUAL: {
            if (!this->same_as(t1)) return false;
            else if (this->type == VALUE_INT) {
                DEST_BINTYPE(BINARY_LTE_INT);
            } else if (this->type == VALUE_FLOAT) {
                DEST_BINTYPE(BINARY_LTE_FLOAT);
            } else if (this->type == VALUE_STRING) {
                DEST_BINTYPE(BINARY_LTE_STRING);
            } else if (this->type == VALUE_BOOL) {
                DEST_BINTYPE(BINARY_LTE_BOOL);
            } else return false;
            DEST_TYPE(VALUE_BOOL);
            break;
        }
        default: {
            // This case should not even be fired.
            return false;
        }
    }
    #undef DEST_TYPE
    #undef DEST_BINTYPE
    return true;
}

void Type::copy_to(Type *type)
{
    type->type = this->type;
    if (this->inner_type) type->inner_type = new Type(*this->inner_type);  // Possible mem leak
    type->class_name = this->class_name;
    for (Type *t : this->parameters) type->parameters.push_back(new Type(*t));
}

bool Type::same_as(Type *type)
{
    // General case.
    if (!(this->type == type->type)) return false;

    // For classes.
    if (this->type == VALUE_CLASS) {
        return this->class_name == type->class_name;
    }

    // Recursive check for inner types.
    else if (this->inner_type != nullptr) {
        return this->inner_type->same_as(type->inner_type);
    }

    // The answer if no inner type is found.
    return true;
}

std::string Type::to_string()
{
    // Check if it's a no type.
    if (this->type == VALUE_NO_TYPE) return "<no-type>";

    // Check if it's a simple type (int, float, bool, string).
    for (auto &[key, value] : Type::value_types) {
        if (value == this->type) {
            return key;
        }
    }

    // Check if it's a complex type (list / dict / function):
    if (this->type == VALUE_LIST) return "[" + this->inner_type->to_string() + "]";
    if (this->type == VALUE_DICT) return "{" + this->inner_type->to_string() + "}";
    if (this->type == VALUE_FUN) {
        std::string result = "(";
        for (Type *inner : this->parameters) result += inner->to_string() + ", ";
        // pop the ", " of the last element
        if (this->parameters.size() > 0) { result.pop_back(); result.pop_back(); result += " "; }
        // Append the return type if needed.
        if (this->inner_type) result += "-> " + this->inner_type->to_string();
        return result + ")";
    }

    // It's a class.
    return this->class_name;
}

void Type::print()
{
    printf("%s", this->to_string().c_str());
}

void Type::println()
{
    this->print();
    printf("\n");
}

Type::Type(Function *fun)
{
    this->type = VALUE_FUN;
    this->inner_type = fun->return_type; // return_type will already be nullptr if it have no return type.
    for (Statement *dec : fun->parameters) this->parameters.push_back(static_cast<Declaration *>(dec)->type);
}

Type::Type(Expression *rule, std::vector<Block *> *blocks)
{
    switch (rule->rule) {
        case RULE_INTEGER: { this->type = VALUE_INT; break; }
        case RULE_FLOAT: { this->type = VALUE_FLOAT; break; }
        case RULE_STRING: { this->type = VALUE_STRING; break; }
        case RULE_BOOLEAN: { this->type = VALUE_BOOL; break; }
        case RULE_LIST: {
            this->type = VALUE_LIST;
            List *list = static_cast<List *>(rule);
            if (list->value.size() == 0) break; // The inner type will be nullptr.
            this->inner_type = new Type(list->value[0], blocks);
            break;
        }
        case RULE_DICTIONARY: {
            this->type = VALUE_DICT;
            Dictionary *dict = static_cast<Dictionary *>(rule);
            if (dict->value.size() == 0) break; // The inner type will be nullptr.
            this->inner_type = new Type(dict->value[dict->key_order[0]], blocks);
            break;
        }
        // case RULE_NONE: { this->type = VALUE_NONE; return; }
        case RULE_GROUP: {
            Type(static_cast<Group *>(rule)->expression, blocks).copy_to(this);
            break;
        }
        case RULE_CAST: {
            Cast *cast = static_cast<Cast *>(rule);
            // Check if the cast can be performed
            // is done by the analyzer.
            cast->type->copy_to(this);
            break;
        }
        case RULE_UNARY: {
            Unary *u = static_cast<Unary *>(rule);
            Type type = Type(u->right, blocks);
            type.unary(u->op, this); // No need to check, the binary MUST be analyzed first.
            break;
        }
        case RULE_BINARY: {
            // Ignore the right one, they need to be compatible and analyzed before
            // this line is executed.
            Binary *b = static_cast<Binary *>(rule);
            Type lt = Type(b->left, blocks);
            Type rt = Type(b->right, blocks);
            lt.binary(b->op, &rt, this); // No need to check, the binary MUST be analyzed first.
            break;
        }
        case RULE_VARIABLE: {
            std::string var = static_cast<Variable *>(rule)->name;
            BlockVariableType *res = Block::get_single_variable(var, blocks);
            if (res) {
                res->type->copy_to(this);
                break;
            }
            logger->add_entity(rule->file, rule->line, rule->column, "No variable named '" + var + "' was found in the current or previous blocks.");
            exit(logger->crash());
            break;
        }
        case RULE_ASSIGN: {
            Type(static_cast<Assign *>(rule)->value, blocks).copy_to(this);
            break;
        }
        case RULE_LOGICAL: {
            Logical *logical = static_cast<Logical *>(rule);
            Type type = Type(logical->left, blocks);
            type.copy_to(this);
            break;
        }
        case RULE_FUNCTION: {
            this->type = VALUE_FUN;
            break;
        }
        case RULE_CALL: {
            Call *call = static_cast<Call *>(rule);
            Type t = Type(call->target, blocks);
            if (t.inner_type) t.inner_type->copy_to(this); // Function return type.
            else this->type = VALUE_NO_TYPE; // The function has no return value.
            break;
        }
        case RULE_ACCESS: {
            Access *access = static_cast<Access *>(rule);
            Type t = Type(access->target, blocks);
            if (!t.inner_type) {
                logger->add_entity(access->file, access->line, access->column, "The type " + t.to_string() + " has no inner type.");
                exit(logger->crash());
            }
            t.inner_type->copy_to(this);
            break;
        }
        case RULE_SLICE: {
            Type(static_cast<Slice *>(rule)->target, blocks).copy_to(this);
            break;
        }
        case RULE_RANGE: {
            this->type = VALUE_LIST;
            this->inner_type = new Type(VALUE_INT);
            break;
        }
        default: {
            logger->add_entity(rule->file, rule->line, rule->column, "Invalid expression to get the type of.");
            exit(logger->crash());
        }
    }
}

void Type::deallocate()
{
    // Deallocate the innter type.
    if (this->inner_type) {
        this->inner_type->deallocate();
        delete this->inner_type;
    }
    // Deallocate the parameters if needed.
    for (Type *t : this->parameters) {
        t->deallocate();
        delete t;
    }
}
