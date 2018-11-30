/**
 * |---------------------------------|
 * | Nuua Virtual Machine Operations |
 * |---------------------------------|
 *
 * Copyright 2019 Erik Campobadal <soc@erik.cat>
 * https://nuua.io
 */
#include "../include/operation.hpp"
#include "../../Logger/include/logger.hpp"
#include <algorithm>

namespace Operation
{
    Value iminus(Value *a)
    {
        if (a->type == VALUE_STRING) {
            std::reverse(a->svalue->begin(), a->svalue->end());
            return Value(*a->svalue);
        }

        return Value(-a->to_double());
    }

    Value inot(Value *a)
    {
        return Value(!a->to_bool());
    }

    Value iadd(Value *a, Value *b)
    {
        if (a->type == VALUE_STRING || b->type == VALUE_STRING) return Value(a->to_string() + b->to_string());

        return Value(a->to_double() + b->to_double());
    }

    Value isub(Value *a, Value *b)
    {
        return Value(a->to_double() - b->to_double());
    }

    Value imul(Value *a, Value *b)
    {
        return Value(a->to_double() * b->to_double());
    }

    Value idiv(Value *a, Value *b)
    {
        auto bn = b->to_double();
        if (bn == 0) { logger->error("Division by zero."); exit(EXIT_FAILURE); }

        return Value(a->to_double() / bn);
    }

    Value ieq(Value *a, Value *b)
    {
        if (a->type == VALUE_STRING && b->type == VALUE_STRING) return Value(a->to_string() == b->to_string());

        return Value(a->to_double() == b->to_double());
    }

    Value ineq(Value *a, Value *b)
    {
        if (a->type == VALUE_STRING && b->type == VALUE_STRING) return Value(*a->svalue != *b->svalue);

        return Value(a->to_double() != b->to_double());
    }

    Value ilt(Value *a, Value *b)
    {
        return Value(a->to_double() < b->to_double());
    }

    Value ilte(Value *a, Value *b)
    {
        return Value(a->to_double() <= b->to_double());
    }

    Value iht(Value *a, Value *b)
    {
        return Value(a->to_double() > b->to_double());
    }

    Value ihte(Value *a, Value *b)
    {
        return Value(a->to_double() >= b->to_double());
    }

    Value ilen(Value *a)
    {
        return Value(a->to_double());
    }
}
