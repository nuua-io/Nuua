#include "../include/type.hpp"
#include "../../Logger/include/logger.hpp"

const std::unordered_map<std::string, ValueType> Type::value_types = {
    { "none", VALUE_NONE },
    { "int", VALUE_INT },
    { "float", VALUE_FLOAT },
    { "bool", VALUE_BOOL },
    { "string", VALUE_STRING },
    { "list", VALUE_LIST },
    { "dict", VALUE_DICT },
    { "fun", VALUE_FUN },
};

const std::vector<std::string> Type::types_string = {
    "VALUE_NONE", "VALUE_INT", "VALUE_FLOAT", "VALUE_BOOL",
    "VALUE_STRING", "VALUE_LIST", "VALUE_DICT", "VALUE_FUN"
};

Type::Type(std::string name)
{
    for (auto type : Type::value_types) {
        if (name.find(type.first) == 0) {
            this->type = type.second;
            return;
        }
    }

    logger->error("Unknown type '" + name + "'");
    exit(EXIT_FAILURE);
}

bool Type::is(ValueType type)
{
    return this->type == type;
}

bool Type::same_as(Type *type)
{
    // General case.
    if (!this->is(type->type)) return false;

    // Recursive check for special cases.
    else if (this->is(VALUE_LIST))
        return this->listType->same_as(type->listType);
    else if (this->is(VALUE_DICT))
        return this->dictType->first->same_as(type->dictType->first) && this->dictType->second->same_as(type->dictType->second);

    // The answer if no special case is found.
    return true;
}

void Type::print()
{
    printf("%s", Type::types_string[this->type].c_str());
}

void Type::println()
{
    this->print();
    printf("\n");
}

std::string Type::get_string(Expression *rule)
{
    switch (rule->rule) {
        case RULE_INTEGER: { return "int"; }
        case RULE_FLOAT: { return "float"; }
        case RULE_STRING: { return "string"; }
        case RULE_BOOLEAN: { return "bool"; }
        case RULE_LIST: {
            auto list = static_cast<List *>(rule);
            if (list->value.size() == 0) return "list[?]";
            std::string inside = Type::get_string(list->value[0]);
            for (size_t i = 1; i < list->value.size(); i++) {
                if (Type::get_string(list->value[i]) != inside) {
                    logger->error("Multiple types inside a list");
                    exit(EXIT_FAILURE);
                }
            }
            return "list[" + inside + "]";
        }
        case RULE_DICTIONARY: {
            auto dict = static_cast<Dictionary *>(rule);
            if (dict->value.size() == 0) return "dict[?]";
            std::string inside = Type::get_string(dict->value[dict->key_order[0]]);
            for (size_t i = 1; i < dict->value.size(); i++) {
                if (Type::get_string(dict->value[dict->key_order[i]]) != inside) {
                    logger->error("Multiple types inside a dictionary.");
                    exit(EXIT_FAILURE);
                }
            }
            return "dict[" + inside + "]";
        }
        case RULE_NONE: { return "none"; }
        case RULE_GROUP: {
            return Type::get_string(static_cast<Group *>(rule)->expression);
        }
        case RULE_CAST: {
            auto cast = static_cast<Cast *>(rule);
            // Check if the cast can be performed
            // (to be done)

            return cast->type;
        }
        case RULE_UNARY: {
            return Type::get_string(static_cast<Unary *>(rule)->right);
        }
        case RULE_BINARY: {
            auto binary = static_cast<Binary *>(rule);
            /*
            auto type = Type::get_string(binary->left);
            if (type != Type::get_string(binary->right)) {
                logger->error(
                    "Binary expression does not have the same type on both sides. "
                    + type + " vs " + Type::get_string(binary->right)
                );
                exit(EXIT_FAILURE);
            }
            */
            return "<" + Type::get_string(binary->left) + "=" + Type::get_string(binary->right) + ">";
        }
        case RULE_VARIABLE: {
            return "<" + static_cast<Variable *>(rule)->name + ">";
        }
        case RULE_ASSIGN: {
            return Type::get_string(static_cast<Assign *>(rule)->value);
        }
        case RULE_ASSIGN_ACCESS: {
            return Type::get_string(static_cast<AssignAccess *>(rule)->value);
        }
        case RULE_LOGICAL: {
            auto logical = static_cast<Logical *>(rule);
            auto type = Type::get_string(logical->left);
            if (type != Type::get_string(logical->right)) {
                logger->error("Logical expression does not have the same type on both sides.");
                exit(EXIT_FAILURE);
            }
            return type;
        }
        case RULE_FUNCTION: {
            return "fun";
        }
        case RULE_CALL: {
            return "<:" + static_cast<Call *>(rule)->callee + ":>";
        }
        case RULE_ACCESS: {
            return "<[" + static_cast<Access *>(rule)->name + "]>";
        }
        default: {
            logger->error("Invalid expression.", rule->line);
            exit(EXIT_FAILURE);
        }
    }
}
