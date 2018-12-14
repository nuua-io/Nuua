#ifndef TYPE_HPP
#define TYPE_HPP

#include <stdint.h>
#include <unordered_map>
#include <utility>
#include <vector>

// Determines the available native types in nuua.
typedef enum : uint8_t {
    VALUE_NONE, VALUE_INT, VALUE_FLOAT, VALUE_BOOL,
    VALUE_STRING, VALUE_LIST, VALUE_DICT, VALUE_FUN
} ValueType;

class Type
{
    static const std::unordered_map<std::string, ValueType> value_types;
    static const std::vector<std::string> types_string;

    public:
        ValueType type;

        union {
            // Stores the type of a VALUE_LIST.
            Type *listType;

            // Stores the pair of value of a VALUE_DICT.
            std::pair<Type *, Type *> *dictType;
        };

        Type()
            : type(VALUE_NONE) {}
        Type(ValueType type)
            : type(type) {}
        Type(ValueType type, Type *listType)
            : type(type), listType(listType) {}
        Type(ValueType type, std::pair<Type *, Type *> *dictType)
            : type(type), dictType(dictType) {}
        Type(std::string name);

        // Checks to see if the current type is a given ValueType.
        // No recursion is done since it does not accept another type.
        bool is(ValueType type);

        // Compares if the type is the same as another provided type.
        // Recursive function that also checks list and dict types.
        bool same_as(Type *type);

        // Prints the type as a string.
        void print();

        // Prints the type as a string with a new line.
        void println();
};

#endif
