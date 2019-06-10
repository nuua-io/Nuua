export class Length {
    value: int
}

export class Rect {
    x: Length
    y: Length
}

export fun create_rec(): Rect {
    return Rect!{x: Length!{value: 10}, y: Length!{value: 10}}
}
