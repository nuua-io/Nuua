def test():
    a = 10
    fun1 = lambda: a
    fun1()
    print(a)

    a += 1
    fun1()
    print(a)

    return fun1

fun = test()

print(f"Fun: {fun()}")
