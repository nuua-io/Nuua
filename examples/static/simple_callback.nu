my_callback: fun = (input: int): int -> input * 2

sample: fun = (callback: fun, number: int): int {
    return callback(number)
}

print sample(my_callback, 10)
