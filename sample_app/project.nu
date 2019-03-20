import addition from operations

fun test(adder: int): |int: int| {
    return |x: int|: int -> x + adder
}

fun main() {
    a: int = 10
    a = a = 20
    print a as float as int
    if a == 10 => print "hello"
    adder: |int: int| = test(10)
    print adder(10)
    print addition(a + a)
}
