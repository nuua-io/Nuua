/**
 * |---------------------------------|
 * | Nuua Virtual Machine Operations |
 * |---------------------------------|
 *
 * Copyright 2019 Erik Campobadal <soc@erik.cat>
 * https://nuua.io
 */
#ifndef OPERATION_HPP
#define OPERATION_HPP

#include "../../Compiler/include/value.hpp"

namespace Operation
{
    Value iminus(Value *a);
    Value inot(Value *a);
    Value iadd(Value *a, Value *b);
    Value isub(Value *a, Value *b);
    Value imul(Value *a, Value *b);
    Value idiv(Value *a, Value *b);
    Value ieq(Value *a, Value *b);
    Value ineq(Value *a, Value *b);
    Value ilt(Value *a, Value *b);
    Value ilte(Value *a, Value *b);
    Value iht(Value *a, Value *b);
    Value ihte(Value *a, Value *b);
    Value ilen(Value *a);
};

#endif
