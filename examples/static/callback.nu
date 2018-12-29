for_each: fun = (callback: fun): int {
    i: int = 0
    result: int = 0
    a: int = 0
    while (i < 5) {
        a = callback(i)
        print "Index: " + i
        print "Current Result: " + a
        print "Addition: " + result + " + " + a
        result = result + a
        print "Result: " + result
        i = i + 1
    }
    return result
}
print for_each((input: int): int -> input * 2)
