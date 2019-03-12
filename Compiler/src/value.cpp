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
#include <iostream>

Value::Value(std::unordered_map<std::string, Value> &a, std::vector<std::string> &b)
    : type(Type(VALUE_DICT)), value_dict(new ValueDictionary(a, b)) {}

Value::Value(uint64_t index, Frame *frame)
    : type(Type(VALUE_FUN)), value_fun(new ValueFunction(index, frame)) {}

Value::Value(Type type)
{
    this->type = type;
    switch (type.type) {
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

double Value::to_double()
{
    auto value = this->get_value();
    switch (value->type.type) {
        case VALUE_INT: { return static_cast<double>(value->value_int); }
        case VALUE_FLOAT: { return value->value_float; }
        case VALUE_BOOL: { return static_cast<double>(value->value_bool); }
        case VALUE_STRING: { return static_cast<double>(value->value_string->length()); }
        case VALUE_LIST: { return static_cast<double>(value->value_list->size()); }
        case VALUE_DICT: { return static_cast<double>(value->value_dict->values.size()); }
        case VALUE_FUN: { return static_cast<double>(reinterpret_cast<std::uintptr_t>(value->value_fun)); } // This looks a bit bad...
        default: { return 0.0; }
    }
}

bool Value::to_bool()
{
    return this->get_value()->to_double() != 0;
}

std::string Value::to_string()
{
    auto value = this->get_value();

    switch (value->type.type) {
        case VALUE_INT: { return std::to_string(value->value_int); }
        case VALUE_FLOAT: { return std::to_string(value->value_float); }
        case VALUE_BOOL: { return value->value_bool ? "true" : "false"; }
        case VALUE_STRING: { return *value->value_string; }
        case VALUE_LIST: {
            std::string list = "[";
            if (value->value_list->size() > 0) {
                for (auto &element : *value->value_list) {
                    list += (element.type.type == VALUE_STRING ? '\'' + element.to_string() + '\'' : element.to_string()) + ", ";
                }
                list.pop_back(); list.pop_back(); // Pop the space and the comma
            }
            return list + "]";
        }
        case VALUE_DICT: {
            std::string dictionary = "{";
            if (value->value_dict->values.size() > 0) {
                for (auto element : value->value_dict->key_order) {
                    auto val = value->value_dict->values[element];
                    dictionary += element + ": " + (val.type.type == VALUE_STRING ? '\'' + val.to_string() + '\'' : val.to_string()) + ", ";
                }
                dictionary.pop_back(); dictionary.pop_back(); // Pop the space and the comma
            }
            return dictionary + "}";
        }
        case VALUE_FUN: {
            char fn[256];
            sprintf(fn, "<Function: 0x%llx>", reinterpret_cast<std::uintptr_t>(value->value_fun));
            return fn;
        }
        case VALUE_REF: {
            logger->error("Reference to_string() caught.");
            exit(EXIT_FAILURE);
        }
        default: { return "none"; }
    }
}

Value Value::cast(Type type)
{
    // Check if they are the same type.
    if (this->type.same_as(&type)) return *this;

    switch (this->type.type) {
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
                case VALUE_INT: { double ipart; modf(this->to_double(), &ipart); return Value(static_cast<int64_t>(ipart)); } // Will be converted to int in double constructor
                case VALUE_BOOL: { return Value(this->to_bool()); }
                case VALUE_STRING: { return Value(this->to_string()); }
                default: { break; }
            }
            break;
        }
        case VALUE_BOOL: {
            switch (type.type) {
                case VALUE_INT: { return Value(static_cast<int64_t>(this->to_bool())); }
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
        default: {
            logger->error("Invalid type conversion.");
            exit(EXIT_FAILURE);
        }
    }
    return Value();
}

Value *Value::get_value()
{
    auto value = this;
    while (value->type.type == VALUE_REF) value = value->value_ref;
    return value;
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

void Value::copy_to(Value *dest)
{
    // This could perhaps be skipped.
    // Simple check to see if there's no need to copy it.
    if (this == dest) return;
    // Deallocate the destination.
    dest->deallocate();
    // Copy the type.
    this->type.copy_to(&dest->type);
    // Copy the value.
    switch (this->type.type) {
        case VALUE_INT: { dest->value_int = this->value_int; break; }
        case VALUE_FLOAT: { dest->value_float = this->value_float; break; }
        case VALUE_BOOL: { dest->value_bool = this->value_bool; break; }
        case VALUE_STRING: { dest->value_string = this->value_string; break; }
        case VALUE_LIST: { dest->value_list = new std::vector<Value>(*this->value_list); break; }
        case VALUE_DICT: { dest->value_dict = new ValueDictionary(*this->value_dict); break; }
        case VALUE_FUN: { dest->value_fun = new ValueFunction(*this->value_fun); break; }
        case VALUE_REF: { dest->value_ref = this->value_ref; break; }
        default: { break; }
    }
}

void Value::deallocate()
{
    // Deallocate the type.
    this->type.deallocate();
    // Deallocate the data
    switch (this->type.type) {
        case VALUE_STRING: { delete this->value_string; break; }
        case VALUE_LIST: { delete this->value_list; break; }
        case VALUE_DICT: { delete this->value_dict; break; }
        case VALUE_FUN: { delete this->value_fun; break; }
        default: { /* Ignore, no deallocation needed. */ }
    }
}

void Value::op_minus(Value *dest, Value *src1)
{
    src1 = src1->get_value();
    if (src1->type.type == VALUE_STRING) {
        auto buffer = new std::string(*src1->value_string);
        std::reverse(buffer->begin(), buffer->end());
        dest->deallocate();
        dest->type.type = VALUE_STRING;
        dest->value_string = buffer;
        return;
    } else if (src1->type.type == VALUE_INT) {
        auto val = -src1->value_int;
        dest->deallocate();
        dest->type.type = VALUE_INT;
        dest->value_int = val;
        return;
    }

    auto val = -src1->to_double();
    dest->deallocate();
    dest->type.type = VALUE_FLOAT;
    dest->value_float = val;
}

void Value::op_not(Value *dest, Value *src1)
{
    dest->deallocate();
    dest->type.type = VALUE_BOOL;
    dest->value_bool = !src1->to_bool();
}

void Value::op_add(Value *dest, Value *src1, Value *src2)
{
    src1 = src1->get_value(), src2 = src2->get_value();

    if (src1->type.type == VALUE_STRING || src1->type.type == VALUE_STRING) {
        auto val = new std::string(src1->to_string() + src2->to_string());
        dest->deallocate();
        dest->type.type = VALUE_STRING;
        dest->value_string = val;
        return;
    }
    else if (src1->type.type == VALUE_INT && src2->type.type == VALUE_INT) {
        auto val = src1->value_int + src2->value_int;
        dest->deallocate();
        dest->type.type = VALUE_INT;
        dest->value_int = val;
        return;
    }

    auto val = src1->to_double() + src2->to_double();
    dest->deallocate();
    dest->type.type = VALUE_FLOAT;
    dest->value_float = val;
}

void Value::op_sub(Value *dest, Value *src1, Value *src2)
{
    src1 = src1->get_value(), src2 = src2->get_value();

    if (src1->type.type == VALUE_INT && src2->type.type == VALUE_INT) {
        auto val = src1->value_int - src2->value_int;
        dest->deallocate();
        dest->type.type = VALUE_INT;
        dest->value_int = val;
        return;
    }

    auto val = src1->to_double() - src2->to_double();
    dest->deallocate();
    dest->type.type = VALUE_FLOAT;
    dest->value_float = val;
}

void Value::op_mul(Value *dest, Value *src1, Value *src2)
{
    src1 = src1->get_value(), src2 = src2->get_value();

    if (src1->type.type == VALUE_INT && src2->type.type == VALUE_INT) {
        auto val = src1->value_int * src2->value_int;
        dest->deallocate();
        dest->type.type = VALUE_INT;
        dest->value_int = val;
        return;
    }

    auto val = src1->to_double() * src2->to_double();
    dest->deallocate();
    dest->type.type = VALUE_FLOAT;
    dest->value_float = val;
}

void Value::op_div(Value *dest, Value *src1, Value *src2)
{
    src1 = src1->get_value(), src2 = src2->get_value();

    auto bn = src2->to_double();
    if (bn == 0) { logger->error("Division by zero."); exit(EXIT_FAILURE); }

    auto val = src1->to_double() / bn;
    dest->deallocate();
    dest->type.type = VALUE_FLOAT;
    dest->value_float = val;
}

void Value::op_eq(Value *dest, Value *src1, Value *src2)
{
    src1 = src1->get_value(), src2 = src2->get_value();

    if (src1->type.type == VALUE_STRING && src2->type.type == VALUE_STRING) {
        auto val = src1->to_string() == src2->to_string();
        dest->deallocate();
        dest->type.type = VALUE_BOOL;
        dest->value_bool = val;
        return;
    }

    auto val = src1->to_double() == src2->to_double();
    dest->deallocate();
    dest->type.type = VALUE_BOOL;
    dest->value_bool = val;
}

void Value::op_neq(Value *dest, Value *src1, Value *src2)
{
    src1 = src1->get_value(), src2 = src2->get_value();

    if (src1->type.type == VALUE_STRING && src2->type.type == VALUE_STRING) {
        auto val = src1->to_string() != src2->to_string();
        dest->deallocate();
        dest->type.type = VALUE_BOOL;
        dest->value_bool = val;
        return;
    }

    auto val = src1->to_double() != src2->to_double();
    dest->deallocate();
    dest->type.type = VALUE_BOOL;
    dest->value_bool = val;
}

void Value::op_lt(Value *dest, Value *src1, Value *src2)
{
    src1 = src1->get_value(), src2 = src2->get_value();

    auto val = src1->to_double() < src2->to_double();
    dest->deallocate();
    dest->type.type = VALUE_BOOL;
    dest->value_bool = val;
}

void Value::op_lte(Value *dest, Value *src1, Value *src2)
{
    src1 = src1->get_value(), src2 = src2->get_value();

    auto val = src1->to_double() <= src2->to_double();
    dest->deallocate();
    dest->type.type = VALUE_BOOL;
    dest->value_bool = val;
}

void Value::op_ht(Value *dest, Value *src1, Value *src2)
{
    src1 = src1->get_value(), src2 = src2->get_value();

    auto val = src1->to_double() > src2->to_double();
    dest->deallocate();
    dest->type.type = VALUE_BOOL;
    dest->value_bool = val;
}

void Value::op_hte(Value *dest, Value *src1, Value *src2)
{
    src1 = src1->get_value(), src2 = src2->get_value();

    auto val = src1->to_double() >= src2->to_double();
    dest->deallocate();
    dest->type.type = VALUE_BOOL;
    dest->value_bool = val;
}

Value::~Value()
{
    this->deallocate();
}
