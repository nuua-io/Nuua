Person: class = {
    name: string
    id, age: int

    constructor: fun = (name: string, id, age: int) {
        self.name = name
        self.id = id
        self.age = age
    }

    older_than: fun = (p: Person): bool -> self.age > p.age
}
