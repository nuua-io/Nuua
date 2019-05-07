fun aaa() => print 30

fun test(): int -> 20

fun add(a: int, b: int): int -> a + b

fun main() {
    a: int = 10
    print a
    l: [int] = [1, 2, 3, 4, 5, a]
    b: (-> int) = test
    print b
}
