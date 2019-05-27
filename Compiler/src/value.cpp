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

Value::Value(const std::unordered_map<std::string, Value> &a, const std::vector<std::string> &b, const std::shared_ptr<Type> &inner_type)
    : type({ VALUE_DICT, inner_type }), value(std::make_shared<ndict_t>(a, b)) {}

Value::Value(const size_t index, const registers_size_t registers, const Type &type)
    : type(type), value(nfun_t(index, registers)) {}

Value::Value(const std::shared_ptr<Type> &type)
{
    this->build_from_type(type.get());
}

Value::Value(const Type &type)
{
    this->build_from_type(&type);
}

Value::Value(const Value &value)
{
    // Copy the type.
    value.type.copy_to(this->type);
    // Copy the value.
    this->value = value.value;
    /*
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
    */
}

void Value::build_from_type(const Type *type)
{
    type->copy_to(this->type);
    switch (type->type) {
        case VALUE_INT: { this->value = 0LL; break; }
        case VALUE_FLOAT: { this->value = 0.0; break; }
        case VALUE_BOOL: { this->value = false; break; }
        case VALUE_STRING: { this->value = std::string(); break; }
        case VALUE_LIST: { this->value = std::make_shared<std::vector<Value>>(); break; }
        case VALUE_DICT: { this->value = std::make_shared<ValueDictionary>(std::unordered_map<std::string, Value>(), std::vector<std::string>()); break; }
        case VALUE_FUN: { this->value = ValueFunction(0, 0); break; }
        default: {
            // logger->add_entity(this->file, LINE(), "Can't declare this value type without an initializer.");
            exit(logger->crash());
        }
    }
}


void Value::retype(ValueType new_type, const std::shared_ptr<Type> &new_inner_type)
{
    this->type.type = new_type;
    if (new_inner_type) this->type.inner_type = new_inner_type;
}

void Value::copy_to(Value *dest) const
{
    // printf("COPY TO: <from> : %s\n", this->to_string().c_str());
    // Simple check to see if there's no need to copy it.
    if (this == dest) return;
    // Copy the type.
    this->type.copy_to(dest->type);
    // Copy the value.
    dest->value = this->value;
    /*
    switch (this->type.type) {
        case VALUE_INT: { dest->value = this->value_int; break; }
        case VALUE_FLOAT: { dest->value_float = this->value_float; break; }
        case VALUE_BOOL: { dest->value_bool = this->value_bool; break; }
        case VALUE_STRING: {
            auto s = *this->value_string;
            dest->value_string.reset(new std::string(s)); break; }
        case VALUE_LIST: { dest->value_list = std::make_unique<std::vector<Value>>(*this->value_list); break; }
        case VALUE_DICT: { dest->value_dict = std::make_unique<ValueDictionary>(*this->value_dict); break; }
        case VALUE_FUN: { dest->value_fun = std::make_unique<ValueFunction>(*this->value_fun); break; }
        default: { break; }
    }
    */
}

std::string Value::to_string() const
{
    std::string r;
    switch (this->type.type) {
        case VALUE_INT: { r = std::to_string(GETV(this->value, nint_t)); break; }
        case VALUE_FLOAT: { r = std::to_string(GETV(this->value, nfloat_t)); break; }
        case VALUE_BOOL: { r = GETV(this->value, nbool_t) ? "true" : "false"; break; }
        case VALUE_STRING: { r = GETV(this->value, nstring_t); break; }
        case VALUE_LIST: {
            const std::shared_ptr<nlist_t> &list = GETV(this->value, std::shared_ptr<nlist_t>);
            r += "[";
            if (list->size() > 0) {
                for (const Value &el : *list) {
                    r += el.to_string() + ", ";
                }
                r.pop_back(); r.pop_back(); // the ',' and the ' ' of the last element.
            }
            r += "]";
            break;
        }
        case VALUE_DICT: {
            const std::shared_ptr<ndict_t> &dict = GETV(this->value, std::shared_ptr<ndict_t>);
            r += "{";
            if (dict->values.size() > 0) {
                for (const auto &[key, value] : dict->values) {
                    r += key + ": " + value.to_string() + ", ";
                }
                r.pop_back(); r.pop_back(); // the ',' and the ' ' of the last element.
            }
            r += "}";
            break;
        }
        case VALUE_FUN: {
            // const nfun_t &fun = GETV(this->value, nfun_t);
            r += this->type.to_string();
            break;
        }
        default: { break; }
    }
    return r;
}

void ValueDictionary::insert(const std::string &key, const Value &value)
{
    value.copy_to(&this->values[key]);
    // this->values.insert({{ key, std::move(value) }});
    if (std::find(this->key_order.begin(), this->key_order.end(), key) == this->key_order.end()) {
        this->key_order.push_back(key);
    }
}
