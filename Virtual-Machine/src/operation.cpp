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
    Value *iminus(Value *a)
    {
        if (a->type == VALUE_STRING) {
            std::reverse(a->svalue->begin(), a->svalue->end());
            return new Value(*a->svalue);
        }

        auto result = new Value(-a->to_double());

        delete a;

        return result;
    }

    Value *inot(Value *a)
    {
        auto result = new Value(!a->to_bool());

        delete a;

        return result;
    }

    Value *iadd(Value *a, Value *b)
    {
        Value *result;
        if (a->type == VALUE_STRING || b->type == VALUE_STRING) result = new Value(a->to_string() + b->to_string());
        else result = new Value(a->to_double() + b->to_double());

        delete a; delete b;

        return result;
    }

    Value *isub(Value *a, Value *b)
    {
        auto result = new Value(a->to_double() - b->to_double());
        delete a; delete b;

        return result;
    }

    Value *imul(Value *a, Value *b)
    {
        auto result = new Value(a->to_double() * b->to_double());
        delete a; delete b;

        return result;
    }

    Value *idiv(Value *a, Value *b)
    {
        auto bn = b->to_double();
        if (bn == 0) { logger->error("Division by zero."); exit(EXIT_FAILURE); }
        auto result = new Value(a->to_double() / bn);
        delete a; delete b;

        return result;
    }

    Value *ieq(Value *a, Value *b)
    {
        Value *result;
        if (a->type == VALUE_STRING && b->type == VALUE_STRING) result = new Value(a->to_string() == b->to_string());
        else result = new Value(a->to_double() == b->to_double());
        delete a; delete b;

        return result;
    }

    Value *ineq(Value *a, Value *b)
    {
        Value *result;
        if (a->type == VALUE_STRING && b->type == VALUE_STRING) result = new Value(*a->svalue != *b->svalue);
        else result = new Value(a->to_double() != b->to_double());
        delete a; delete b;

        return result;
    }

    Value *ilt(Value *a, Value *b)
    {
        auto result = new Value(a->to_double() < b->to_double());
        delete a; delete b;

        return result;
    }

    Value *ilte(Value *a, Value *b)
    {
        auto result = new Value(a->to_double() <= b->to_double());
        delete a; delete b;

        return result;
    }

    Value *iht(Value *a, Value *b)
    {
        auto result = new Value(a->to_double() > b->to_double());
        delete a; delete b;

        return result;
    }

    Value *ihte(Value *a, Value *b)
    {
        auto result = new Value(a->to_double() >= b->to_double());
        delete a; delete b;

        return result;
    }

    Value *ilen(Value *a)
    {
        auto result = new Value(a->to_double());
        delete a;

        return result;
    }
}
