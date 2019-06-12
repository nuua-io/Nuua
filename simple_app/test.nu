use Length, Rect, create_rec from 'rect'

class Person {
    name: string
    age: int
    fun show(): string {
        return self.name
    }
}

fun main(argv: [string]) {
    p: Person = Person!{name: "Erik", age: 22}
    print p.show()
    r := create_rec()
    r2 := Rect!{x: Length!{value: 10}}
    print r2.x
    print r2.y
    print r2.x.value
}
