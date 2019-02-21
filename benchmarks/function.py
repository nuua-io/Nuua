def add (a: int, b: int) -> int: return a + b
i: int = 0

while i < 6000000:
    add(10, 10)
    i = i + 1

print(i)
