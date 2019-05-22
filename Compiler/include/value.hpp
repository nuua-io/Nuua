/**
 * |--------------------|
 * | Nuua Native Values |
 * |--------------------|
 *
 * Copyright 2019 Erik Campobadal <soc@erik.cat>
 * https://nuua.io
 */
#ifndef VALUE_HPP
#define VALUE_HPP

#include "program.hpp"
#include "../../Analyzer/include/module.hpp"
#include <string>
#include <vector>

class ValueDictionary;
class ValueFunction;

// Base value class representing a nuua value.
class Value
{
    void build_from_type(const Type *type);
    public:
        // The type of the value.
        Type type;
        // Using a union to avoid unessesary memory.
        union {
            // Stores the reporesentation of the VALUE_INT.
            int64_t value_int = NULL;
            // Stores the representation of the VALUE_FLOAT.
            double value_float;
            // Stores the representation of the VALUE_BOOL.
            bool value_bool;
            // Stores the representation of the VALUE_STRING.
            std::unique_ptr<std::string> value_string;
            // Stores the representation of the VALUE_LIST.
            std::unique_ptr<std::vector<Value>> value_list;
            // Stores the representation of the VALUE_DICT.
            std::unique_ptr<ValueDictionary> value_dict;
            // Stores the representation of the VALUE_FUN.
            std::unique_ptr<ValueFunction> value_fun;
        };
        // The following are the basic constructors for the value. Each one respresents
        // a diferent value to be stored. They pretty much speak by themselves.
        // Integer (int) value.
        Value() : type({ VALUE_INT }), value_int(0) {}
        Value(const int64_t a)
            : type({ VALUE_INT }), value_int(a) {}
        // Float value (double in C/C++).
        Value(const double a)
            : type({ VALUE_FLOAT }), value_float(a) {}
        // Boolean value.
        Value(const bool a)
            : type({ VALUE_BOOL }), value_bool(a) {}
        // String value.
        Value(const std::string &a)
            : type({ VALUE_STRING }), value_string(std::make_unique<std::string>(a)) {}
        // List value.
        Value(const std::vector<Value> &a, const std::shared_ptr<Type> &inner_type)
            : type({ VALUE_LIST, inner_type }), value_list(std::make_unique<std::vector<Value>>(a)) {}
        // The following two constructors are basically defined in the value.cpp since
        // They make use of a forward declared constructor.
        Value(const std::unordered_map<std::string, Value> &a, const std::vector<std::string> &b, const std::shared_ptr<Type> &inner_type);
        Value(const size_t index, const registers_size_t registers);
        // Create default initialized value, given the type.
        Value(const std::shared_ptr<Type> &type);
        Value(const Type &type);
        Value(const Value &value);
        ~Value() {}
        // Copies the current value to the destnation.
        void copy_to(Value *dest) const;
};

// Defines how a dictionary value is.
class ValueDictionary
{
    public:
        // Represents the hashmap of the dictionary.
        std::unordered_map<std::string, Value> values;
        // Represent the current key order of the hashmap.
        // Even with an ordered map, the key order is still
        // important.
        std::vector<std::string> key_order;
        // Adds a new element to the dictionary.
        void insert(const std::string &key, const Value &value);
        // The basic constructor of the dictionary.
        ValueDictionary(const std::unordered_map<std::string, Value> &values, const std::vector<std::string> &key_order)
            : values(values), key_order(key_order) {}
};

// Defines how a function value is.
class ValueFunction
{
    public:
        // Stores the function index where it's code begin.
        size_t index;
        // Stores the ammount of registers needed
        registers_size_t registers;
        // Basic constructor for the function value.
        ValueFunction(size_t index, registers_size_t registers)
            : index(index), registers(registers) {}
};

#endif
