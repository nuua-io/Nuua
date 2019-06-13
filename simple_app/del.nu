class A {
    list: [int]
    dict: {int}
    str: string
}

fun main(argv: [string]) {
    list := [1, 2, 3, 4, 5]
    print list
    delete list[2]
    print list
    dict := {a: 1, b: 2, c: 3}
    print dict
    delete dict['b']
    print dict
    str := "ABCDEFG"
    print str
    delete str[2]
    print str
    a := A!{list: [1, 2, 3, 4, 5], dict: {a: 1, b: 2, c: 3}, str: "ABCDEFG"}
    print a.list
    delete a.list[2]
    print a.list
    print a.dict
    delete a.dict['b']
    print a.dict
    print a.str
    delete a.str[2]
    print a.str
}
