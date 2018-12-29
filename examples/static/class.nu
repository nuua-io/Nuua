Person: class = {
    name: string
    id, age: int

    constructor: fun = (name: string, id, age: int): none {
        self.name = name
        self.id = id
        self.age = age
    }

    older_than: fun = (p: Self): bool -> self.age > p.age
}
