use addition from "operations"

fun test(a: int) -> a + 10

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

    // First class functions
    adder: (int -> int) = test

    // Function call
    print adder(10)

    // Imported function call
    print addition(a + a)
}
