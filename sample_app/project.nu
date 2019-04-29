use addition from "operations"

fun test(a: int): int {
    return a + 10
}

fun test2(a: string): string -> a + " World"

fun test3() => print "Hello World"

fun main() {
    // Variable declaration
    a: int = 10

    // Assignment
    a = a = 20

    // Slice access
    lis: [int] = [1, 2, 3, 4, 5]
    print lis[:2]

    // Function call 1
    test2()

    // Casting
    print a as float as int

    // Conditional
    if a == 10 => print "hello"

    // Looping
    for element, index in 0...3 {
        print element
        print index
    }

    // Join and split.
    // ["1", "2", "3"]|'x'| // "1x2x3"
    // "1x2x3"|'x'| // ["1", "2", "3"]

    // First class functions
    adder: (int -> int) = test
    test2: (-> int) = test2

    // Function call 2
    print adder(10)

    // Imported function call
    print addition(a + a)
}
