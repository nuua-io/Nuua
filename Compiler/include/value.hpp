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
    public:
        // The type of the value.
        Type type;
        bool deallocated = false;
        // Using a union to avoid unessesary memory.
        union {
            // Stores the reporesentation of the VALUE_INT.
            int64_t value_int = 0;
            // Stores the representation of the VALUE_FLOAT.
            double value_float;
            // Stores the representation of the VALUE_BOOL.
            bool value_bool;
            // Stores the representation of the VALUE_STRING.
            std::string *value_string;
            // Stores the representation of the VALUE_LIST.
            std::vector<Value> *value_list;
            // Stores the representation of the VALUE_DICT.
            ValueDictionary *value_dict;
            // Stores the representation of the VALUE_FUN.
            ValueFunction *value_fun;
            // Stores the ref representation.
            Value *value_ref;
        };
        // The following are the basic constructors for the value. Each one respresents
        // a diferent value to be stored. They pretty much speak by themselves.
        // Integer (int) value.
        Value() : type({ VALUE_INT }), value_int(0) {}
        Value(int64_t a)
            : type({ VALUE_INT }), value_int(a) {}
        // Float value (double in C/C++).
        Value(double a)
            : type({ VALUE_FLOAT }), value_float(a) {}
        // Boolean value.
        Value(bool a)
            : type({ VALUE_BOOL }), value_bool(a) {}
        // String value.
        Value(std::string &a)
            : type({ VALUE_STRING }), value_string(new std::string(a)) {}
        // List value.
        Value(std::vector<Value> &a)
            : type({ VALUE_LIST }), value_list(new std::vector<Value>(a)) {}
        // The following two constructors are basically defined in the value.cpp since
        // They make use of a forward declared constructor.
        Value(std::unordered_map<std::string, Value> &a, std::vector<std::string> &b);
        Value(size_t index, registers_size_t registers);
        // Create default initialized value, given the type.
        Value(Type *type);
        ~Value();
        // Copies the current value to the destnation.
        void copy_to(Value *dest);
        // Deallocates the space used by the value (frees the memory of the current value).
        void deallocate();
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
        void insert(std::string &key, Value value);
        // The basic constructor of the dictionary.
        ValueDictionary(std::unordered_map<std::string, Value> values, std::vector<std::string> key_order)
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
