#ifndef TYPE_HPP
#define TYPE_HPP

#include "rules.hpp"
#include "block.hpp"
#include <stdint.h>
#include <unordered_map>
#include <utility>
#include <vector>

// Determines the available native types in nuua.
typedef enum : uint8_t {
    VALUE_INT, VALUE_FLOAT, VALUE_BOOL,
    VALUE_STRING, VALUE_LIST, VALUE_DICT, VALUE_FUN,
    VALUE_CLASS, VALUE_NO_TYPE
} ValueType;

class Type
{
    static const std::unordered_map<std::string, ValueType> value_types;
    static const std::vector<std::string> types_string;
    public:
        // Stores the type.
        ValueType type;
        // Stores the inner type if needed (used as return type for functions).
        Type *inner_type = nullptr;
        // Class name
        std::string class_name;
        // Store the parameters of a function type.
        std::vector<Type *> parameters;
        // Create a value type given the type.
        Type() : type(VALUE_NO_TYPE) {}
        Type(ValueType type)
            : type(type) {}
        // Create a type given a value type and the inner type.
        Type(ValueType type, Type *inner_type)
            : type(type), inner_type(inner_type) {}
        // Create a type given a string representation of it.
        Type(std::string name);
        // Create a type given an expression and a
        // list of code blocks to know the variable values.
        Type(Expression *rule, std::vector<Block *> *blocks);
        // Create a function type given the function itself.
        Type(Function *fun);
        // Returns true if the cast can be performed and stored the dest cast type to the pointer.
        bool cast(Type *to, CastType *dest_casttype = nullptr);
        // Returns true if the unary operation can be done and stores the dest type and the unary type on their pointers.
        bool unary(Token &op, Type *dest_type = nullptr, UnaryType *dest_unarytype = nullptr);
        // Returns true if the binary op can be done and stores the dest type and the binary type on their pointers.
        bool binary(Token &op, Type *t1, Type *dest_type = nullptr, BinaryType *dest_bintype = nullptr);
        // Returns the unary type resulting of a unary operation.
        // UnaryType unary(Token &op, Type *dest);
        // Returns the string representation of the type.
        std::string to_string();
        // Copies the type from "this" to "type".
        void copy_to(Type *type);
        // Compares if the type is the same as another provided type.
        // Recursive function that also checks inner types.
        bool same_as(Type *type);
        // Prints the type as a string.
        void print();
        // Prints the type as a string with a new line.
        void println();
        // Deallocates the type.
        void deallocate();
};

#endif
