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
    // Unsafe variants for public. Underlying for shared_ptr or reference.
    void copy_to(Type *type) const;
    bool same_as(const Type *type) const;
    public:
        // Stores the type.
        ValueType type;
        // Stores the inner type if needed (used as return type for functions).
        std::shared_ptr<Type> inner_type;
        // Class name
        std::string class_name;
        // Store the parameters of a function type.
        std::vector<std::shared_ptr<Type>> parameters;
        // Copy constructor
        // Type(const Type &t);
        // void operator =(const Type &t);
        // Create a value type given the type.
        Type() : type(VALUE_NO_TYPE) {}
        Type(const ValueType type)
            : type(type) {}
        // Create a type given a value type and the inner type.
        Type(const ValueType type, const std::shared_ptr<Type> &inner_type)
            : type(type), inner_type(inner_type) {}
        // Create a type given a string representation of it.
        Type(const std::string &name);
        // Create a type given an expression and a
        // list of code blocks to know the variable values.
        Type(const std::shared_ptr<Expression> &rule, const std::vector<std::shared_ptr<Block>> *blocks);
        // Create a function type given the function itself.
        Type(const std::shared_ptr<Function> &fun);
        // ~Type() { this->deallocate(); }
        // Returns true if the cast can be performed and stored the dest cast type to the pointer.
        bool cast(const std::shared_ptr<Type> &to, CastType *dest_casttype = nullptr);
        bool cast(const Type *to, CastType *dest_casttype = nullptr);
        // Returns true if the unary operation can be done and stores the dest type and the unary type on their pointers.
        bool unary(const Token &op, const std::shared_ptr<Type> &dest_type = std::shared_ptr<Type>(), UnaryType *dest_unarytype = nullptr);
        bool unary(const Token &op, Type *dest_type = nullptr, UnaryType *dest_unarytype = nullptr);
        // Returns true if the binary op can be done and stores the dest type and the binary type on their pointers.
        bool binary(const Token &op, const std::shared_ptr<Type> &t1, const std::shared_ptr<Type> &dest_type = std::shared_ptr<Type>(), BinaryType *dest_bintype = nullptr);
        bool binary(const Token &op, const Type *t1, Type *dest_type = nullptr, BinaryType *dest_bintype = nullptr);
        // Returns the unary type resulting of a unary operation.
        // UnaryType unary(Token &op, Type *dest);
        // Returns the string representation of the type.
        std::string to_string() const;
        // Copies the type from "this" to "type".
        void copy_to(std::shared_ptr<Type> &type) const;
        void copy_to(Type &type) const;
        // Compares if the type is the same as another provided type.
        // Recursive function that also checks inner types.
        bool same_as(const std::shared_ptr<Type> &type) const;
        bool same_as(const Type &type) const;
        // Prints the type as a string.
        void print() const ;
        // Prints the type as a string with a new line.
        void println() const;
};

#endif
