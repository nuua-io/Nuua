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

Value::Value(std::unordered_map<std::string, Value> &a, std::vector<std::string> &b)
    : type({ VALUE_DICT }), value_dict(new ValueDictionary(a, b)) {}

Value::Value(size_t index, registers_size_t registers)
    : type({ VALUE_FUN }), value_fun(new ValueFunction(index, registers)) {}

Value::Value(Type *type)
{
    type->copy_to(&this->type);
    printf("Value of type: %s => [%p, %p -> %p, %p]\n", this->type.to_string().c_str(), type, type->inner_type, &this->type, this->type.inner_type);
    switch (type->type) {
        case VALUE_INT: { this->value_int = 0; break; }
        case VALUE_FLOAT: { this->value_float = 0.0; break; }
        case VALUE_BOOL: { this->value_bool = false; break; }
        case VALUE_STRING: { this->value_string = new std::string(""); break; }
        case VALUE_LIST: { this->value_list = new std::vector<Value>(); break; }
        case VALUE_DICT: { this->value_dict = new ValueDictionary(std::unordered_map<std::string, Value>(), std::vector<std::string>()); break; }
        case VALUE_FUN: { this->value_fun = new ValueFunction(0, 0); break; }
        default: {
            // logger->add_entity(this->file, LINE(), "Can't declare this value type without an initializer.");
            exit(logger->crash());
        }
    }
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
        case VALUE_STRING: { dest->value_string = new std::string(*this->value_string); break; }
        case VALUE_LIST: { dest->value_list = new std::vector<Value>(*this->value_list); break; }
        case VALUE_DICT: { dest->value_dict = new ValueDictionary(*this->value_dict); break; }
        case VALUE_FUN: { dest->value_fun = new ValueFunction(*this->value_fun); break; }
        default: { break; }
    }
}

void Value::deallocate()
{
    if (!this->deallocated) {
        // Deallocate the type.
        this->type.deallocate();
        // Deallocate the data
        switch (this->type.type) {
            case VALUE_STRING: {
                if (this->value_string) {
                    delete this->value_string;
                    this->value_string = nullptr;
                }
                break;
            }
            case VALUE_LIST: {
                if (this->value_list) {
                    delete this->value_list;
                    this->value_list = nullptr;
                }
                break;
            }
            case VALUE_DICT: {
                if (this->value_dict) {
                    delete this->value_dict;
                    this->value_dict = nullptr;
                }
                break;
            }
            case VALUE_FUN: {
                if (this->value_fun) {
                    delete this->value_fun;
                    this->value_fun = nullptr;
                }
                break;
            }
            default: { /* Ignore, no deallocation needed. */ }
        }
        this->deallocated = true;
    }
}

Value::~Value()
{
    this->deallocate();
}

void ValueDictionary::insert(std::string &key, Value value)
{
    this->values.insert({{ key, value }});
    if (std::find(this->key_order.begin(), this->key_order.end(), key) == this->key_order.end()) {
        this->key_order.push_back(key);
    }
}
