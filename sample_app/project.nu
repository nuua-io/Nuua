import addition from operations

fun test(adder: int): |int: int| {
    return |x: int|: int -> x + adder
}

fun main() {
    a: int = 10
    adder: |int: int| = test(10)
    print adder(10)
    print addition(a + a)
}
