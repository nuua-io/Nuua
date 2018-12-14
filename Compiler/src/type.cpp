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
            this->type = type.second; return;
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
