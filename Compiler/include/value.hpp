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
#include <string>
#include <vector>
#include <stdint.h>
#include <unordered_map>

// Determines the available native types in nuua.
typedef enum : uint8_t {
    VALUE_NUMBER, VALUE_BOOLEAN, VALUE_STRING, VALUE_LIST,
    VALUE_DICTIONARY, VALUE_FUNCTION, VALUE_NONE
} ValueType;

class Frame;
class Value;

// Defines how a dictionary value is.
class ValueDictionary
{
    public:
        // Represents the hashmap of the dictionary.
        std::unordered_map<std::string, Value*> values;

        // Represent the current key order of the hashmap.
        // Even with an ordered map, the key order is still
        // important.
        std::vector<std::string> key_order;

        // The basic constructor of the dictionary.
        ValueDictionary(std::unordered_map<std::string, Value*> values, std::vector<std::string> key_order)
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

// Base value class representing a nuua value.
class Value
{
    public:
        // The type of the value.
        ValueType type;

        // Using a union to avoid unessesary memory.
        union {
            // Stores the representation of the VALUE_NUMBER.
            double nvalue;

            // Stores the representation of the VALUE_BOOLEAN.
            bool bvalue;

            // Stores the representation of the VALUE_STRING.
            std::string *svalue;

            // Stores the representation of the VALUE_LIST.
            std::vector<Value *> *lvalues;

            // Stores the representation of the VALUE_DICTIONARY.
            ValueDictionary *dvalues;

            // Stores the representation of the VALUE_FUNCTION.
            ValueFunction *fvalue;
        };

        // The following are the basic constructors for the value. Each one respresents
        // a diferent value to be stored. They pretty much speak by themselves.
        Value()
            : type(VALUE_NONE) {}
        Value(double a)
            : type(VALUE_NUMBER), nvalue(a) {}
        Value(bool a)
            : type(VALUE_BOOLEAN), bvalue(a) {}
        Value(std::string a)
            : type(VALUE_STRING), svalue(new std::string(a)) {}
        Value(std::vector<Value *> a)
            : type(VALUE_LIST), lvalues(new std::vector(a)) {}
        Value(std::unordered_map<std::string, Value*> a, std::vector<std::string> b)
            : type(VALUE_DICTIONARY), dvalues(new ValueDictionary(a, b)) {}
        Value(uint64_t index, Frame *frame)
            : type(VALUE_FUNCTION), fvalue(new ValueFunction(index, frame)) {}

        // Converts the current value to a valid double.
        double to_double();

        // Converts the current value to a valid boolean.
        bool to_bool();

        // Converts the current value to a valid string.
        std::string to_string();

        // Prints the value to the screen.
        void print();

        // Prints the value to the screen with a new line (\n) at the end.
        void println();
};

#endif
