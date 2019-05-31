fun fibonacci(n: int): int {
  if (n <= 1) => return n
  return fibonacci(n - 2) + fibonacci(n - 1)
}

fun main() {
    a: [int] = [1, 2, 3]
    print a
    b: int = a[2]
    print b
    for i in 0..25 => print fibonacci(i)
}
