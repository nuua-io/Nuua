use addition from operations

fun test(adder: int): |int: int| {
    return |x: int|: int -> x + adder
}

fun main() {
    // Variable declaration
    a: int = 10

    // Assignment
    a = a = 20

    // Casting
    print a as float as int

    // Conditional
    if a == 10 => print "hello"

    // Looping
    for element, index in [1, 2, 3] {
        print element
        print index
    }

    // Function call
    adder: |int: int| = test(10)

    // Closure call
    print adder(10)

    // Imported function call
    print addition(a + a)
}
