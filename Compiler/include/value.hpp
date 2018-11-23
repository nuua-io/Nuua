#ifndef VALUE_HPP
#define VALUE_HPP

#include "program.hpp"
#include <string>
#include <vector>
#include <stdint.h>
#include <unordered_map>

typedef enum : uint8_t {
    VALUE_NUMBER, VALUE_BOOLEAN, VALUE_STRING, VALUE_LIST,
    VALUE_DICTIONARY, VALUE_FUNCTION, VALUE_NONE
} ValueType;

class Frame;
class Value;

class ValueDictionary
{
    public:
        std::unordered_map<std::string, Value*> values;
        std::vector<std::string> key_order;

        ValueDictionary(std::unordered_map<std::string, Value*> values, std::vector<std::string> key_order)
            : values(values), key_order(key_order) {}
};

class ValueFunction
{
    public:
        uint64_t index;
        Frame *frame;

        ValueFunction(uint64_t index, Frame *frame)
            : index(index), frame(frame) {}
};

class Value
{
    public:
        ValueType type;
        union {
            double nvalue;
            bool bvalue;
            std::string *svalue;
            std::vector<Value> *lvalues;
            ValueDictionary *dvalues;
            ValueFunction *fvalue;
        };

        Value()
            : type(VALUE_NONE) {}
        Value(double a)
            : type(VALUE_NUMBER), nvalue(a) {}
        Value(bool a)
            : type(VALUE_BOOLEAN), bvalue(a) {}
        Value(std::string a)
            : type(VALUE_STRING), svalue(new std::string(a)) {}
        Value(std::vector<Value> a)
            : type(VALUE_LIST), lvalues(new std::vector(a)) {}
        Value(std::unordered_map<std::string, Value*> a, std::vector<std::string> b)
            : type(VALUE_DICTIONARY), dvalues(new ValueDictionary(a, b)) {}
        Value(uint64_t index, Frame *frame)
            : type(VALUE_FUNCTION), fvalue(new ValueFunction(index, frame)) {}

        double to_double();
        std::string to_string();
        void print();
};

#endif
