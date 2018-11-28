/**
 * |--------------------|
 * | Nuua Native Values |
 * |--------------------|
 *
 * Copyright 2019 Erik Campobadal <soc@erik.cat>
 * https://nuua.io
 */
#include "../include/value.hpp"

double Value::to_double()
{
    switch (this->type) {
        case VALUE_NUMBER: { return this->nvalue; }
        case VALUE_BOOLEAN: { return this->bvalue ? 1.0 : 0.0; }
        case VALUE_STRING: { return static_cast<double>(this->svalue->length()); }
        case VALUE_LIST: { return static_cast<double>(this->lvalues->size()); }
        case VALUE_DICTIONARY: { return static_cast<double>(this->dvalues->values.size()); }
        case VALUE_FUNCTION: { return static_cast<double>(reinterpret_cast<std::uintptr_t>(this->fvalue)); } // This looks a bit bad...
        default: { return 0.0; }
    }
}

bool Value::to_bool()
{
    return this->to_double() != 0;
}

std::string Value::to_string()
{
    switch (this->type) {
        case VALUE_NUMBER: { return std::to_string(this->nvalue); }
        case VALUE_BOOLEAN: { return this->bvalue ? "true" : "false"; }
        case VALUE_STRING: { return *this->svalue; }
        case VALUE_LIST: {
            std::string list = "[";
            if (this->lvalues->size() > 0) {
                for (auto element : *this->lvalues) {
                    list += (element->type == VALUE_STRING ? '\'' + element->to_string() + '\'' : element->to_string()) + ", ";
                }
                list.pop_back(); list.pop_back(); // Pop the space and the comma
            }
            return list + "]";
        }
        case VALUE_DICTIONARY: {
            std::string dictionary = "{";
            if (this->dvalues->values.size() > 0) {
                for (auto element : this->dvalues->key_order) {
                    auto value = this->dvalues->values.at(element);
                    dictionary += element + ": " + (value->type == VALUE_STRING ? '\'' + value->to_string() + '\'' : value->to_string()) + ", ";
                }
                dictionary.pop_back(); dictionary.pop_back(); // Pop the space and the comma
            }
            return dictionary + "}";
        }
        case VALUE_FUNCTION: {
            char fn[256];
            sprintf(fn, "<Function: 0x%llx>", reinterpret_cast<std::uintptr_t>(this->fvalue));
            return fn;
        }
        default: { return "none"; }
    }
}

void Value::print()
{
    printf("%s", this->to_string().c_str());
}

void Value::println()
{
    this->print();
    printf("\n");
}
