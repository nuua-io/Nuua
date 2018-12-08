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

// #include "program.hpp"
#include <string>
#include <vector>
#include <stdint.h>
#include <unordered_map>

// Determines the available native types in nuua.
typedef enum : uint8_t {
    VALUE_INT, VALUE_FLOAT, VALUE_BOOLEAN, VALUE_STRING,
    VALUE_LIST, VALUE_DICTIONARY, VALUE_FUNCTION, VALUE_NONE
} ValueType;

class Frame;
class ValueDictionary;
class ValueFunction;

// Base value class representing a nuua value.
class Value
{
    public:
        // The type of the value.
        ValueType type;

        // Using a union to avoid unessesary memory.
        union {
            // Stores the reporesentation of the VALUE_INT.
            int64_t value_int;

            // Stores the representation of the VALUE_FLOAT.
            double value_float;

            // Stores the representation of the VALUE_BOOLEAN.
            bool value_bool;

            // Stores the representation of the VALUE_STRING.
            std::string *value_string;

            // Stores the representation of the VALUE_LIST.
            std::vector<Value> *value_list;

            // Stores the representation of the VALUE_DICTIONARY.
            ValueDictionary *value_dict;

            // Stores the representation of the VALUE_FUNCTION.
            ValueFunction *value_fun;
        };

        // The following are the basic constructors for the value. Each one respresents
        // a diferent value to be stored. They pretty much speak by themselves.

        // None value.
        Value()
            : type(VALUE_NONE) {}

        // Integer (int) value.
        Value(int64_t a)
            : type(VALUE_INT), value_int(a) {}

        // Float value (double in C/C++) implemented in the .cpp due to the
        // conversion to int value if double can be safely converted to int.
        Value(double a);

        // Boolean value.
        Value(bool a)
            : type(VALUE_BOOLEAN), value_bool(a) {}

        // String value.
        Value(std::string a)
            : type(VALUE_STRING), value_string(new std::string(a)) {}

        // List value.
        Value(std::vector<Value> a)
            : type(VALUE_LIST), value_list(new std::vector(a)) {}

        // The following two constructors are basically defined in the value.cpp since
        // They make use of a forward declared constructor.
        Value(std::unordered_map<std::string, Value> a, std::vector<std::string> b);
        Value(uint64_t index, Frame *frame);

        // returns true if the Value is of the given type.
        bool is(ValueType type);

        // Converts the current value to a valid double.
        double to_double();

        // Converts the current value to a valid boolean.
        bool to_bool();

        // Converts the current value to a valid string.
        std::string to_string();

        // Returns a new value representing the length of the current value.
        Value length();

        // Prints the value to the screen.
        void print();

        // Prints the value to the screen with a new line '\n' at the end.
        void println();

        // The following are the operations to perform diferent taks between values.
        // They simply overload the default C++ operators.
        Value operator -(); // -Value
        Value operator !(); // !Value
        Value operator +(Value &b); // Value + Value
        Value operator -(Value &b); // Value - Value
        Value operator *(Value &b); // Value * Value
        Value operator /(Value &b); // Value / Value
        Value operator ==(Value &b); // Value == Value
        Value operator !=(Value &b); // Value != Value
        Value operator <(Value &b); // Value < Value
        Value operator <=(Value &b); // Value <= Value
        Value operator >(Value &b); // Value > Value
        Value operator >=(Value &b); // Value >= Value
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

        // The basic constructor of the dictionary.
        ValueDictionary(std::unordered_map<std::string, Value> values, std::vector<std::string> key_order)
            : values(values), key_order(key_order) {}
};

// Defines how a function value is.
class ValueFunction
{
    public:
        // Stores the function index where it's code begin.
        uint64_t index;

        // Stores the frame where the function relies on.
        Frame *frame;

        // Basic constructor for the function value.
        ValueFunction(uint64_t index, Frame *frame)
            : index(index), frame(frame) {}
};

#endif
