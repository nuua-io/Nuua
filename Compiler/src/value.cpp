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
    : type(type), value(std::make_shared<nfun_t>(index, registers)) {}

Value::Value(const std::string &class_name, const std::vector<std::string> &props)
    : type({ class_name }), value(std::make_shared<nobject_t>(props)) {}

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
        case VALUE_FUN: { this->value = std::shared_ptr<nfun_t>(); break; }
        case VALUE_OBJECT: { this->value = std::shared_ptr<nobject_t>(); break; }
        default: {
            logger->add_entity(std::shared_ptr<const std::string>(), 0, 0, "Can't declare a value given only the type: '" + type->to_string() + "'.");
            exit(logger->crash());
        }
    }
}


void Value::retype(ValueType new_type, const std::shared_ptr<Type> &new_inner_type)
{
    this->type.reset(new_type, new_inner_type);
}

void Value::copy_to(Value *dest) const
{
    // Simple check to see if there's no need to copy it.
    if (this == dest) return;
    // Copy the type.
    this->type.copy_to(dest->type);
    // Copy the value.
    dest->value = this->value;
}

bool Value::same_as(const Value &value) const
{
    // Check if they match in type.
    if (!this->type.same_as(value.type)) return false;
    // Check the value.
    switch (this->type.type) {
        case VALUE_INT:
        case VALUE_FLOAT:
        case VALUE_BOOL:
        case VALUE_STRING: { return this->value == value.value; }
        case VALUE_LIST: {
            const std::shared_ptr<nlist_t> &a = GETV(this->value, std::shared_ptr<nlist_t>);
            const std::shared_ptr<nlist_t> &b = GETV(value.value, std::shared_ptr<nlist_t>);
            // Do they match in length?
            if (a->size() != b->size()) return false;
            // Does it have elements?
            if (a->size() == 0) return true;
            // Check each element.
            for (size_t i = 0; i < a->size(); i++) {
                // Check if their inner element match at position i.
                if (!(*a)[i].same_as((*b)[i])) return false;
            }
            // They are equal.
            return true;
        }
        case VALUE_DICT: {
            const std::shared_ptr<ndict_t> &a = GETV(this->value, std::shared_ptr<ndict_t>);
            const std::shared_ptr<ndict_t> &b = GETV(value.value, std::shared_ptr<ndict_t>);
            // Do they match in length?
            if (a->values.size() != b->values.size()) return false;
            // Does it have elements?
            if (a->values.size() == 0) return true;
            // Compare their keys and values.
            for (const auto &[k, e] : a->values) {
                // Check if b have that key.
                if (b->values.find(k) == b->values.end()) return false;
                // Check if the value matches at that key.
                if (!e.same_as(b->values[k])) return false;
            }
            // They are equal.
            return true;
        }
        case VALUE_FUN: { return GETV(this->value, std::shared_ptr<nfun_t>).get() == GETV(value.value, std::shared_ptr<nfun_t>).get(); }
        case VALUE_OBJECT: {
            const std::shared_ptr<nobject_t> &a = GETV(this->value, std::shared_ptr<nobject_t>);
            const std::shared_ptr<nobject_t> &b = GETV(value.value, std::shared_ptr<nobject_t>);
            // Check the number of props.
            if (a->props.size() != b->props.size()) return false;
            // Are there any props at all?
            if (a->props.size() == 0) return true;
            // Check the props names and values.
            for (size_t i = 0; i < a->props.size(); i++) {
                // Check for the prop name.
                if (a->props[i] != b->props[i]) return false;
                // Check for the prop value.
                if (!a->registers[i].same_as(b->registers[i])) return false;
            }
            return true;
        }
    }
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
        case VALUE_OBJECT: {
            const std::shared_ptr<nobject_t> &object = GETV(this->value, std::shared_ptr<nobject_t>);
            r += this->type.to_string() + "!{";
            if (object) {
                for (registers_size_t i = 0; i < object->props.size(); i++) {
                    r += object->props[i] + ": " + (object->registers.get() + i)->to_string() + ", ";
                }
                if (object->props.size() > 0) { r.pop_back(); r.pop_back(); }
            } else {
                r += "<Uninitialized>";
            }
            r += "}";
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

ValueObject::ValueObject(const std::vector<std::string> &props) : props(props)
{
    this->registers = std::make_unique<Value[]>(props.size());
}
