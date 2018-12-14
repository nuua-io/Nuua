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

Value::Value(std::unordered_map<std::string, Value> a, std::vector<std::string> b)
    : type(Type(VALUE_DICT)), value_dict(new ValueDictionary(a, b)) {}

Value::Value(uint64_t index, Frame *frame)
    : type(Type(VALUE_FUN)), value_fun(new ValueFunction(index, frame)) {}

Value::Value(Type type)
{
    this->type = type;
    switch (type.type) {
        case VALUE_NONE: { break; }
        case VALUE_INT: { this->value_int = 0; break; }
        case VALUE_FLOAT: { this->value_float = 0.0; break; }
        case VALUE_BOOL: { this->value_bool = false; break; }
        case VALUE_STRING: { this->value_string = new std::string(""); break; }
        case VALUE_LIST: { this->value_list = new std::vector<Value>(); break; }
        case VALUE_DICT: { this->value_dict = new ValueDictionary(std::unordered_map<std::string, Value>(), std::vector<std::string>()); break; }
        case VALUE_FUN: { this->value_fun = new ValueFunction(0, nullptr); break; }
        default: { logger->error("Can't declare this value type without an initializer."); exit(EXIT_FAILURE); }
    }
}

bool Value::is(Type *type)
{
    return this->type.same_as(type);
}

bool Value::is(ValueType type)
{
    return this->type.is(type);
}

double Value::to_double()
{
    switch (this->type.type) {
        case VALUE_INT: { return static_cast<double>(this->value_int); }
        case VALUE_FLOAT: { return this->value_float; }
        case VALUE_BOOL: { return static_cast<double>(this->value_bool); }
        case VALUE_STRING: { return static_cast<double>(this->value_string->length()); }
        case VALUE_LIST: { return static_cast<double>(this->value_list->size()); }
        case VALUE_DICT: { return static_cast<double>(this->value_dict->values.size()); }
        case VALUE_FUN: { return static_cast<double>(reinterpret_cast<std::uintptr_t>(this->value_fun)); } // This looks a bit bad...
        default: { return 0.0; }
    }
}

bool Value::to_bool()
{
    return this->to_double() != 0;
}

std::string Value::to_string()
{
    switch (this->type.type) {
        case VALUE_INT: { return std::to_string(this->value_int); }
        case VALUE_FLOAT: { return std::to_string(this->value_float); }
        case VALUE_BOOL: { return this->value_bool ? "true" : "false"; }
        case VALUE_STRING: { return *this->value_string; }
        case VALUE_LIST: {
            std::string list = "[";
            if (this->value_list->size() > 0) {
                for (auto element : *this->value_list) {
                    list += (element.is(VALUE_STRING) ? '\'' + element.to_string() + '\'' : element.to_string()) + ", ";
                }
                list.pop_back(); list.pop_back(); // Pop the space and the comma
            }
            return list + "]";
        }
        case VALUE_DICT: {
            std::string dictionary = "{";
            if (this->value_dict->values.size() > 0) {
                for (auto element : this->value_dict->key_order) {
                    auto value = this->value_dict->values.at(element);
                    dictionary += element + ": " + (value.is(VALUE_STRING) ? '\'' + value.to_string() + '\'' : value.to_string()) + ", ";
                }
                dictionary.pop_back(); dictionary.pop_back(); // Pop the space and the comma
            }
            return dictionary + "}";
        }
        case VALUE_FUN: {
            char fn[256];
            sprintf(fn, "<Function: 0x%llx>", reinterpret_cast<std::uintptr_t>(this->value_fun));
            return fn;
        }
        default: { return "none"; }
    }
}

Value Value::cast(Type type)
{
    if (this->type.same_as(&type)) return *this;

    switch (this->type.type) {
        case VALUE_NONE: {
            switch (type.type) {
                case VALUE_STRING: { return Value(this->to_string()); }
                default: { break; }
            }
            break;
        }
        case VALUE_INT: {
            switch (type.type) {
                case VALUE_FLOAT: { return Value(this->to_double()); }
                case VALUE_BOOL: { return Value(this->to_bool()); }
                case VALUE_STRING: { return Value(this->to_string()); }
                default: { break; }
            }
            break;
        }
        case VALUE_FLOAT: {
            switch (type.type) {
                case VALUE_INT: { double ipart; modf(this->value_float, &ipart); return Value(static_cast<int64_t>(ipart)); } // Will be converted to int in double constructor
                case VALUE_BOOL: { return Value(this->to_bool()); }
                case VALUE_STRING: { return Value(this->to_string()); }
                default: { break; }
            }
            break;
        }
        case VALUE_BOOL: {
            switch (type.type) {
                case VALUE_INT: { return Value(static_cast<int64_t>(this->value_bool)); } // Will be converted to int in double constructor
                case VALUE_FLOAT: { return Value(this->to_bool()); }
                case VALUE_STRING: { return Value(this->to_string()); }
                default: { break; }
            }
            break;
        }
        case VALUE_STRING: {
            switch (type.type) {
                case VALUE_INT: { return Value(static_cast<int64_t>(this->to_double())); }
                case VALUE_FLOAT: { return Value(this->to_double()); }
                case VALUE_BOOL: { return Value(this->to_bool()); }
                default: { break; }
            }
            break;
        }
        case VALUE_LIST: {
            switch (type.type) {
                case VALUE_INT: { return Value(static_cast<int64_t>(this->to_double())); }
                case VALUE_FLOAT: { return Value(this->to_double()); }
                case VALUE_BOOL: { return Value(this->to_bool()); }
                case VALUE_STRING: { return Value(this->to_string()); }
                default: { break; }
            }
            break;
        }
        case VALUE_DICT: {
            switch (type.type) {
                case VALUE_INT: { return Value(static_cast<int64_t>(this->to_double())); }
                case VALUE_FLOAT: { return Value(this->to_double()); }
                case VALUE_BOOL: { return Value(this->to_bool()); }
                case VALUE_STRING: { return Value(this->to_string()); }
                default: { break; }
            }
            break;
        }
        case VALUE_FUN: {
            switch (type.type) {
                case VALUE_STRING: { return Value(this->to_string()); }
                default: { break; }
            }
            break;
        }
    }

    logger->error("Invalid type conversion.");
    exit(EXIT_FAILURE);
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
    } else if (this->is(VALUE_INT)) return Value(-this->value_int);

    return Value(-this->to_double());
}

Value Value::operator !()
{
    return Value(!this->to_bool());
}

Value Value::operator +(Value &b)
{
    if (this->is(VALUE_STRING) || b.is(VALUE_STRING)) return Value(this->to_string() + b.to_string());
    else if (this->is(VALUE_INT) && b.is(VALUE_INT)) return Value(this->value_int + b.value_int);

    return Value(this->to_double() + b.to_double());
}

Value Value::operator -(Value &b)
{
    if (this->is(VALUE_INT) && b.is(VALUE_INT)) return Value(this->value_int - b.value_int);

    return Value(this->to_double() - b.to_double());
}

Value Value::operator *(Value &b)
{
    if (this->is(VALUE_INT) && b.is(VALUE_INT)) return Value(this->value_int * b.value_int);

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
    if (this->is(VALUE_STRING) && b.is(VALUE_STRING)) return Value(this->to_string() == b.to_string());

    return Value(this->to_double() == b.to_double());
}

Value Value::operator !=(Value &b)
{
    if (this->is(VALUE_STRING) && b.is(VALUE_STRING)) return Value(this->to_string() != b.to_string());

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
