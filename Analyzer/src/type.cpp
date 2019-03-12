#include "../include/type.hpp"
#include "../../Logger/include/logger.hpp"

const std::unordered_map<std::string, ValueType> Type::value_types = {
    { "int", VALUE_INT },
    { "float", VALUE_FLOAT },
    { "bool", VALUE_BOOL },
    { "string", VALUE_STRING },
    { "list", VALUE_LIST },
    { "dict", VALUE_DICT },
    { "fun", VALUE_FUN },
    { "ref", VALUE_REF }
};

const std::vector<std::string> Type::types_string = {
    "VALUE_INT", "VALUE_FLOAT", "VALUE_BOOL",
    "VALUE_STRING", "VALUE_LIST", "VALUE_DICT", "VALUE_FUN",
    "VALUER_REF"
};

Type::Type(std::string name)
{
    // Check to see if it have any inner types.
    size_t position = name.find('[');
    if (position == std::string::npos) {
        // No inner type detected
        for (auto type : Type::value_types) {
            if (name.find(type.first) == 0) {
                this->type = type.second;
                return;
            }
        }
    }

    // There's an inner type.
    for (auto type : Type::value_types) {
        if (name.substr(0, position).find(type.first) == 0) {
            this->type = type.second;
            size_t last = name.rfind(']');
            if (last == std::string::npos) {
                logger->error("Unfinished inner type specifier.");
                exit(EXIT_FAILURE);
            }
            this->inner_type = new Type(name.substr(position + 1, last));
            return;
        }
    }

    logger->error("Unknown type '" + name + "'");
    exit(EXIT_FAILURE);
}

void Type::copy_to(Type *type)
{
    type->type = this->type;
    type->inner_type = this->inner_type;
}

bool Type::same_as(Type *type)
{
    // General case.
    if (!(this->type == type->type)) return false;

    // Recursive check for inner types.
    else if (this->inner_type != nullptr) {
        return this->inner_type->same_as(type->inner_type);
    }

    // The answer if no inner type is found.
    return true;
}

std::string Type::to_string()
{
    std::string result;

    for (auto &[key, value] : Type::value_types) {
        if (value == this->type) {
            result = key;
            break;
        }
    }

    if (this->inner_type) {
        result += '[' + this->inner_type->to_string() + ']';
    }

    return result;
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

Type::Type(Expression *rule, std::vector<Block> *blocks)
{
    /*
    switch (rule->rule) {
        case RULE_INTEGER: { this->type = VALUE_INT; return; }
        case RULE_FLOAT: { this->type = VALUE_FLOAT; return; }
        case RULE_STRING: { this->type = VALUE_STRING; return; }
        case RULE_BOOLEAN: { this->type = VALUE_BOOL; return; }
        case RULE_LIST: {
            this->type = VALUE_LIST;
            auto list = static_cast<List *>(rule);
            if (list->value.size() == 0) return; // The inner type will be nullptr.
            this->inner_type = new Type(list->value[0], blocks);
            return;
        }
        case RULE_DICTIONARY: {
            this->type = VALUE_DICT;
            auto dict = static_cast<Dictionary *>(rule);
            if (dict->value.size() == 0) return; // The inner type will be nullptr.
            this->inner_type = new Type(dict->value[dict->key_order[0]], blocks);
            return;
        }
        case RULE_NONE: { this->type = VALUE_NONE; return; }
        case RULE_GROUP: {
            Type(static_cast<Group *>(rule)->expression, blocks)
                .copy_to(this);
            return;
        }
        case RULE_CAST: {
            auto cast = static_cast<Cast *>(rule);
            // Check if the cast can be performed
            // (to be done)

            Type(cast->type)
                .copy_to(this);
            return;
        }
        case RULE_UNARY: {
            Type(static_cast<Unary *>(rule)->right, blocks)
                .copy_to(this);
            return;
        }
        case RULE_BINARY: {
            // Ignore the right one, they need to be compatible and analyzed before
            // this line is executed.
            Type(static_cast<Binary *>(rule)->left, blocks)
                .copy_to(this);
            return;
        }
        case RULE_VARIABLE: {
            auto var = static_cast<Variable *>(rule)->name;
            for (auto &block : *blocks) {
                auto res = block.get_variable(var);
                if (res) {
                    Type(res->type)
                        .copy_to(this);
                    return;
                }
            }
            logger->error("No variable named '" + var + "' was found in the current or previous blocks.");
            exit(EXIT_FAILURE);
            return;
        }
        case RULE_ASSIGN: {
            Type(static_cast<Assign *>(rule)->value, blocks)
                .copy_to(this);
            return;
        }
        case RULE_ASSIGN_ACCESS: {
            Type(static_cast<AssignAccess *>(rule)->value, blocks)
                .copy_to(this);
            return;
        }
        case RULE_LOGICAL: {
            auto logical = static_cast<Logical *>(rule);
            auto type = Type(logical->left, blocks);
            if (!Type(logical->right, blocks).same_as(&type)) {
                logger->error("Logical expression does not have the same type on both sides.");
                exit(EXIT_FAILURE);
            }
            type.copy_to(this);
            return;
        }
        case RULE_FUNCTION: {
            this->type = VALUE_FUN;
            return;
        }
        case RULE_CALL: {
            auto call = static_cast<Call *>(rule);
            for (auto &block : *blocks) {
                auto res = block.get_variable(call->callee);
                if (res) {
                    Type(res->return_type)
                        .copy_to(this);
                    return;
                }
            }
            logger->error("No variable named '" + call->callee + "' was found in the current or previous blocks.");
            exit(EXIT_FAILURE);
            return;
        }
        case RULE_ACCESS: {
            auto access = static_cast<Access *>(rule);
            for (auto &block : *blocks) {
                auto res = Type(block.get_variable(access->name)->type);
                if (!res.inner_type) {
                    logger->error("The type " + res.to_string() + " has no inner type.");
                    exit(EXIT_FAILURE);
                }
                res.inner_type->copy_to(this);
                return;
            }
            logger->error("No variable named '" + access->name + "' was found in the current or previous blocks.");
            exit(EXIT_FAILURE);
            return;
        }
        default: {
            logger->error("Invalid expression.", rule->line);
            exit(EXIT_FAILURE);
        }
    }
    */
}

void Type::deallocate()
{
    if (this->inner_type) {
        this->inner_type->deallocate();
        delete this->inner_type;
    }
}
