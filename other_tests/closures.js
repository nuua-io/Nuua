function test() {
    let a = 10
    console.log("Outside before:", a)
    let closure = function () {
        console.log("Before:", a)
        a += 10
        console.log("After:", a)
    }
    closure()
    console.log("Outside after:", a)
    return closure
}

let c = test()
c()
c()
