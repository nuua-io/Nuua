/**
 * |--------------------|
 * | Nuua Native Values |
 * |--------------------|
 *
 * Copyright 2019 Erik Campobadal <soc@erik.cat>
 * https://nuua.io
 */
#include "../include/value.hpp"
#include "../../Logger/include/logger.hpp"
#include <algorithm>
#include <cmath>

Value::Value(double a)
{
    // Check if the double can safely be converted to int.
    double i; // Integer part of the double.
    if (std::modf(a, &i) == 0.0) {
        this->type = VALUE_INT;
        this->value_int = i;
        return;
    }

    this->type = VALUE_FLOAT;
    this->value_float = a;
}

Value::Value(std::unordered_map<std::string, Value> a, std::vector<std::string> b)
    : type(VALUE_DICTIONARY), value_dict(new ValueDictionary(a, b)) {}

Value::Value(uint64_t index, Frame *frame)
    : type(VALUE_FUNCTION), value_fun(new ValueFunction(index, frame)) {}

bool Value::is(ValueType type)
{
    return this->type == type;
}

double Value::to_double()
{
    switch (this->type) {
        case VALUE_INT: { return static_cast<double>(this->value_int); }
        case VALUE_FLOAT: { return this->value_float; }
        case VALUE_BOOLEAN: { return static_cast<double>(this->value_bool); }
        case VALUE_STRING: { return static_cast<double>(this->value_string->length()); }
        case VALUE_LIST: { return static_cast<double>(this->value_list->size()); }
        case VALUE_DICTIONARY: { return static_cast<double>(this->value_dict->values.size()); }
        case VALUE_FUNCTION: { return static_cast<double>(reinterpret_cast<std::uintptr_t>(this->value_fun)); } // This looks a bit bad...
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
        case VALUE_INT: { return std::to_string(this->value_int); }
        case VALUE_FLOAT: { return std::to_string(this->value_float); }
        case VALUE_BOOLEAN: { return this->value_bool ? "true" : "false"; }
        case VALUE_STRING: { return *this->value_string; }
        case VALUE_LIST: {
            std::string list = "[";
            if (this->value_list->size() > 0) {
                for (auto element : *this->value_list) {
                    list += (element.type == VALUE_STRING ? '\'' + element.to_string() + '\'' : element.to_string()) + ", ";
                }
                list.pop_back(); list.pop_back(); // Pop the space and the comma
            }
            return list + "]";
        }
        case VALUE_DICTIONARY: {
            std::string dictionary = "{";
            if (this->value_dict->values.size() > 0) {
                for (auto element : this->value_dict->key_order) {
                    auto value = this->value_dict->values.at(element);
                    dictionary += element + ": " + (value.type == VALUE_STRING ? '\'' + value.to_string() + '\'' : value.to_string()) + ", ";
                }
                dictionary.pop_back(); dictionary.pop_back(); // Pop the space and the comma
            }
            return dictionary + "}";
        }
        case VALUE_FUNCTION: {
            char fn[256];
            sprintf(fn, "<Function: 0x%llx>", reinterpret_cast<std::uintptr_t>(this->value_fun));
            return fn;
        }
        default: { return "none"; }
    }
}

Value Value::length()
{
    return Value(this->to_double());
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

Value Value::operator -()
{
    if (this->is(VALUE_STRING)) {
        std::reverse(this->value_string->begin(), this->value_string->end());
        return Value(*this->value_string);
    }

    return Value(-this->to_double());
}

Value Value::operator !()
{
    return Value(!this->to_bool());
}

Value Value::operator +(Value &b)
{
    if (this->is(VALUE_STRING) || b.is(VALUE_STRING)) return Value(this->to_string() + b.to_string());

    return Value(this->to_double() + b.to_double());
}

Value Value::operator -(Value &b)
{
    return Value(this->to_double() - b.to_double());
}

Value Value::operator *(Value &b)
{
    return Value(this->to_double() * b.to_double());
}

Value Value::operator /(Value &b)
{
    auto bn = b.to_double();
    if (bn == 0) { logger->error("Division by zero."); exit(EXIT_FAILURE); }

    return Value(this->to_double() / bn);
}

Value Value::operator ==(Value &b)
{
    if (this->type == VALUE_STRING && b.type == VALUE_STRING) return Value(this->to_string() == b.to_string());

    return Value(this->to_double() == b.to_double());
}

Value Value::operator !=(Value &b)
{
    if (this->type == VALUE_STRING && b.type == VALUE_STRING) return Value(this->to_string() != b.to_string());

    return Value(this->to_double() != b.to_double());
}

Value Value::operator <(Value &b)
{
    return Value(this->to_double() < b.to_double());
}

Value Value::operator <=(Value &b)
{
    return Value(this->to_double() <= b.to_double());
}

Value Value::operator >(Value &b)
{
    return Value(this->to_double() > b.to_double());
}

Value Value::operator >=(Value &b)
{
    return Value(this->to_double() >= b.to_double());
}
