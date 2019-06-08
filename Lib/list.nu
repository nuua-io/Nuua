export fun list_int_sum(l: [int]): int {
    sum := 0
    for num in l => sum = sum + num
    return sum
}

export fun list_int_map(l: [int], f: (int -> int)) {
    for num, index in l => l[index] = f(num)
}

export fun list_float_sum(l: [float]): float {
    sum := 0.0
    for num in l => sum = sum + num
    return sum
}

export fun list_float_map(l: [float], f: (float -> float)) {
    for num, index in l => l[index] = f(num)
}
