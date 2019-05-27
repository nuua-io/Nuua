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
#include <variant>

// Forward declaration.
class ValueDictionary;

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

typedef int64_t nint_t;
typedef double nfloat_t;
typedef bool nbool_t;
typedef std::string nstring_t;
typedef std::vector<Value> nlist_t;
typedef ValueDictionary ndict_t;
typedef ValueFunction nfun_t;

#define GETV(value, variant) (std::get<variant>(value))

// Base value class representing a nuua value.
class Value
{
    void build_from_type(const Type *type);
    public:
        // The type of the value.
        Type type;
        // Using a variant to avoid unessesary memory.
        std::variant<
            // Stores the reporesentation of the VALUE_INT.
            nint_t,
            // Stores the representation of the VALUE_FLOAT.
            nfloat_t,
            // Stores the representation of the VALUE_BOOL.
            nbool_t,
            // Stores the representation of the VALUE_STRING.
            nstring_t,
            // Stores the representation of the VALUE_LIST.
            std::shared_ptr<nlist_t>,
            // Stores the representation of the VALUE_DICT.
            std::shared_ptr<ndict_t>,
            // Stores the representation of the VALUE_FUN.
            nfun_t
        > value;
        // The following are the basic constructors for the value. Each one respresents
        // a diferent value to be stored. They pretty much speak by themselves.
        // Integer (int) value.
        Value() : type({ VALUE_INT }), value(0LL) {}
        Value(const nint_t a)
            : type({ VALUE_INT }), value(a) {}
        // Float value (double in C/C++).
        Value(const nfloat_t a)
            : type({ VALUE_FLOAT }), value(a) {}
        // Boolean value.
        Value(const nbool_t a)
            : type({ VALUE_BOOL }), value(a) {}
        // String value.
        Value(const nstring_t &a)
            : type({ VALUE_STRING }), value(a) {}
        // List value.
        Value(const nlist_t &a, const std::shared_ptr<Type> &inner_type)
            : type({ VALUE_LIST, inner_type }), value(std::make_shared<nlist_t>(a)) {}
        // The following two constructors are basically defined in the value.cpp since
        // They make use of a forward declared constructor.
        Value(const std::unordered_map<std::string, Value> &a, const std::vector<std::string> &b, const std::shared_ptr<Type> &inner_type);
        Value(const size_t index, const registers_size_t registers, const Type &type);
        // Create default initialized value, given the type.
        Value(const std::shared_ptr<Type> &type);
        Value(const Type &type);
        Value(const Value &value);
        ~Value() {}
        // Retypes the value.
        void retype(ValueType new_type, const std::shared_ptr<Type> &new_inner_type = std::shared_ptr<Type>());
        // Copies the current value to the destnation.
        void copy_to(Value *dest) const;
        // Gets a string representation of the value.
        std::string to_string() const;
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

#endif
