export fun add(a: int, b: int): int -> a + b

fun add2(a: int, b: int): int => return a + b

fun add3(a: int, b: int): int {
    return a + b
}

print add(10, add2(10, add3(10, 10)))
