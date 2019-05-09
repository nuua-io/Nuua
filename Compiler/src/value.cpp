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
// #include <cmath>
// #include <iostream>

Value::Value(std::unordered_map<std::string, Value> &a, std::vector<std::string> &b, std::shared_ptr<Type> &inner_type)
    : type({ VALUE_DICT, inner_type }), value_dict(std::make_unique<ValueDictionary>(a, b)) {}

Value::Value(size_t index, registers_size_t registers)
    : type({ VALUE_FUN }), value_fun(std::make_unique<ValueFunction>(index, registers)) {}

Value::Value(std::shared_ptr<Type> &type)
{
    this->build_from_type(type.get());
}

Value::Value(Type &type)
{
    this->build_from_type(&type);
}

Value::Value(const Value &value)
{
    // Copy the type.
    value.type.copy_to(this->type);
    // Copy the value.
    switch (value.type.type) {
        case VALUE_INT: { this->value_int = value.value_int; break; }
        case VALUE_FLOAT: { this->value_float = value.value_float; break; }
        case VALUE_BOOL: { this->value_bool = value.value_bool; break; }
        case VALUE_STRING: { this->value_string = std::make_unique<std::string>(*value.value_string); break; }
        case VALUE_LIST: { this->value_list = std::make_unique<std::vector<Value>>(*value.value_list); break; }
        case VALUE_DICT: { this->value_dict = std::make_unique<ValueDictionary>(*value.value_dict); break; }
        case VALUE_FUN: { this->value_fun = std::make_unique<ValueFunction>(*value.value_fun); break; }
        default: { break; }
    }
}

void Value::build_from_type(Type *type)
{
    type->copy_to(this->type);
    switch (type->type) {
        case VALUE_INT: { this->value_int = 0; break; }
        case VALUE_FLOAT: { this->value_float = 0.0; break; }
        case VALUE_BOOL: { this->value_bool = false; break; }
        case VALUE_STRING: { this->value_string = std::make_unique<std::string>(""); break; }
        case VALUE_LIST: { this->value_list = std::make_unique<std::vector<Value>>(); break; }
        case VALUE_DICT: { this->value_dict = std::make_unique<ValueDictionary>(std::unordered_map<std::string, Value>(), std::vector<std::string>()); break; }
        case VALUE_FUN: { this->value_fun = std::make_unique<ValueFunction>(0, 0); break; }
        default: {
            // logger->add_entity(this->file, LINE(), "Can't declare this value type without an initializer.");
            exit(logger->crash());
        }
    }
}

void Value::copy_to(Value *dest) const
{
    // Simple check to see if there's no need to copy it.
    if (this == dest) return;
    // Copy the type.
    this->type.copy_to(dest->type);
    // Copy the value.
    switch (this->type.type) {
        case VALUE_INT: { dest->value_int = this->value_int; break; }
        case VALUE_FLOAT: { dest->value_float = this->value_float; break; }
        case VALUE_BOOL: { dest->value_bool = this->value_bool; break; }
        case VALUE_STRING: { dest->value_string.reset(new std::string(*this->value_string)); break; }
        case VALUE_LIST: { dest->value_list.reset(new std::vector<Value>(*this->value_list)); break; }
        case VALUE_DICT: { dest->value_dict.reset(new ValueDictionary(*this->value_dict)); break; }
        case VALUE_FUN: { dest->value_fun.reset(new ValueFunction(*this->value_fun)); break; }
        default: { break; }
    }
}

void ValueDictionary::insert(const std::string &key, const Value &value)
{
    value.copy_to(&this->values[key]);
    // this->values.insert({{ key, std::move(value) }});
    if (std::find(this->key_order.begin(), this->key_order.end(), key) == this->key_order.end()) {
        this->key_order.push_back(key);
    }
}
