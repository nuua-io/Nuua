add: fun = (a: int, b: int): int -> a + b

add2: fun = (a: int, b: int): int => return a + b

add3: fun = (a: int, b: int): int {
    return a + b
}

add(1, add2(2, add3(3, 4)))
