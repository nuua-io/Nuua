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

#include "../../Analyzer/include/type.hpp"
#include <string>
#include <vector>

class Frame;
class ValueDictionary;
class ValueFunction;

// Base value class representing a nuua value.
class Value
{
    public:
        // The type of the value.
        Type type;
        // Using a union to avoid unessesary memory.
        union {
            // Stores the reporesentation of the VALUE_INT.
            int64_t value_int;
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
        Value() : type(VALUE_INT), value_int(0) {}
        Value(int64_t a)
            : type(Type(VALUE_INT)), value_int(a) {}
        // Float value (double in C/C++).
        Value(double a)
            : type(Type(VALUE_FLOAT)), value_float(a) {}
        // Boolean value.
        Value(bool a)
            : type(Type(VALUE_BOOL)), value_bool(a) {}
        // String value.
        Value(std::string &a)
            : type(Type(VALUE_STRING)), value_string(new std::string(a)) {}
        // List value.
        Value(std::vector<Value> &a)
            : type(Type(VALUE_LIST)), value_list(new std::vector(a)) {}
        // The following two constructors are basically defined in the value.cpp since
        // They make use of a forward declared constructor.
        Value(std::unordered_map<std::string, Value> &a, std::vector<std::string> &b);
        Value(uint64_t index, Frame *frame);
        // Create default initialized value, given the type.
        Value(Type type);
        ~Value();
        // Converts the current value to a valid double.
        double to_double();
        // Converts the current value to a valid boolean.
        bool to_bool();
        // Converts the current value to a valid string.
        std::string to_string();
        // Casts the current value to a diferent one if possible.
        Value cast(Type type);
        // Returns a new value representing the length of the current value.
        Value length();
        // Returns the current value (Derefers the reference if needed).
        Value *get_value();
        // Prints the value to the screen.
        void print();
        // Prints the value to the screen with a new line '\n' at the end.
        void println();
        // Copies the current value to the destnation.
        void copy_to(Value *dest);
        // Deallocates the space used by the value (frees the memory of the current value).
        void deallocate();
        // The following are the operations to perform diferent taks between values.
        static void op_minus(Value *dest, Value *src1);
        static void op_not(Value *dest, Value *src1);
        static void op_add(Value *dest, Value *src1, Value *src2);
        static void op_sub(Value *dest, Value *src1, Value *src2);
        static void op_mul(Value *dest, Value *src1, Value *src2);
        static void op_div(Value *dest, Value *src1, Value *src2);
        static void op_eq(Value *dest, Value *src1, Value *src2);
        static void op_neq(Value *dest, Value *src1, Value *src2);
        static void op_lt(Value *dest, Value *src1, Value *src2);
        static void op_lte(Value *dest, Value *src1, Value *src2);
        static void op_ht(Value *dest, Value *src1, Value *src2);
        static void op_hte(Value *dest, Value *src1, Value *src2);
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
