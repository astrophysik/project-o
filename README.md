# Project-O

An object-oriented programming language with type inference and LLVM backend.

## Language Overview

Project-O is a class-based object-oriented language with a clean syntax inspired by Pascal and modern OO languages. It features type inference, single inheritance, and methods as the primary abstraction.

## Syntax

### Classes

```o
class ClassName is
    var field : initial_value
    this() is
        // constructor body
    end
    
    method name(param: Type) : ReturnType is
        // method body
    end
end
```

### Inheritance

```o
class Child extends Parent is
    this() : super() is
        // constructor body 
    end
end
```

### Variables

Variables are declared with `var` and types are inferred from initializers:

```o
var x : 42           // Integer
var pi : 3.14        // Real
var flag : true      // Boolean
var obj : MyClass()  // MyClass instance
```

### Control Flow

**Conditionals:**
```o
if condition then
    // true branch
else
    // false branch
end
```

**Loops:**
```o
while condition loop
    // body
end
```

### Methods

Methods can have a full body or use expression shorthand:

```o
method square(x: Integer) : Integer is
    return x.Mult(x)
end

method get() : Integer => value
```

### Assignments

```o
x := newValue
obj.field := otherValue
```

## Built-in Types

### Integer

64-bit signed integer with arithmetic and comparison operations:

```o
var a : 10
var b : 3

a.Plus(b)      // 13
a.Minus(b)     // 7
a.Mult(b)      // 30
a.Div(b)       // 3
a.Rem(b)       // 1

a.Less(b)      // false
a.Equal(b)     // false

a.toReal()     // 10.0
a.toBoolean()  // true (non-zero)
```

Constants: `Integer.Min`, `Integer.Max`

### Real

Double-precision floating point:

```o
var x : 3.14
var y : 2.0

x.Plus(y)      // 5.14
x.Div(y)       // 1.57
x.toInteger()  // 3
```

Constants: `Real.Min`, `Real.Max`, `Real.Epsilon`

### Boolean

```o
var a : true
var b : false

a.Or(b)    // true
a.And(b)   // false
a.Xor(b)   // true
a.Not()    // false

a.toInteger()  // 1
```

### ArrayInteger

Dynamic array of integers:

```o
var arr : ArrayInteger(100)  // array of size 100

arr.Len()        // 100
arr.Set(0, 42)   // arr[0] = 42
arr.Get(0)       // 42
```

### IO

Input/output operations:

```o
var io : IO()

io.Print(42)      // prints integer
io.Print(3.14)    // prints real
io.Print(true)    // prints boolean
```

### Unit

Represents void/no value, useful for procedures:

```o
method doSomething() : Unit is
    // ...
    return Unit()
end
```

## Complete Example

```o
class Factorial is
    method fact(n: Integer) : Integer is
        if n.Less(2) then
            return 1
        else
            return n.Mult(this.fact(n.Minus(1)))
        end
    end
end

class Main is
    this() is
        var f : Factorial()
        var result : f.fact(5)
        var io : IO()
        io.Print(result)
    end
end
```

## Compilation

```bash
./compiler source.po
```

Outputs:
- `source.ll` — LLVM IR
- `source.o` — Native object file
