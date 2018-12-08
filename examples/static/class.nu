Person: class = {
    name: string
    id, age: int

    constructor: fun = (name: string, id, age: int) {
        self.name = name
        self.id = id
        self.age = age
    }

    older_than: fun = (p: Person) -> self.age > p.age
}
