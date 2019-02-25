fun add_one(a: ref[int]): int => a = a + 1
number: int = 10
add_one(ref number1)
print number
