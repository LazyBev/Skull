# Tidal Programming Language

Tidal is a simple programming language written in Lua that compiles to x86 assembly. It features a clean syntax and basic programming constructs.

## Features

- Variables
- Basic arithmetic
- If/else statements
- While loops
- Function declarations

## Syntax

### Types
```
x :: int = 42;
y :: bool = false;
l :: char = "h";
t :: string = "hai";
a :: int; // Returns 0;
g :: void; // Returns NULL
f :: void = "f"; // This works but would return NULL no matter what the user puts
```

### Variables
```tidal
i := 0; // auto type assignment

OR 

i :: int = 0; // explict type assignment
```

### Write and read funcs
```
a :: string = "hello";
writeln(a);
write(a);

====== new file ===========

a := string = NULL; // or you can write null
read(a); // this asks user for input
write(a); // prints what the user put
```

### If Statements
```tidal
if (x > y) {
    write(x);
} else {
    writeln(y);
}
```

### For Statements
```tidal
for (var i = 0, i < 3, i++) {
}

OR
i := 0;
for (i, i < 3, i++) {
}
```

### While Loops
```tidal
i := 0;
while (i < 10) {
    writeln(i);
    i = i + 1;
}
```

### Functions
```tidal
// funcs use types BUT they need brackets to become funcs

// this is correct
add :: int(a :: int, b :: int) {
    return a + b;
}

// this is not correct
add :: int {
    return a + b;
}

// However you can have auto typed funcs

// Compielr figures out type of the func
add :: func(a :: int, b :: int) {
    return a +b;
}
```


### Main
```
// You need a main function to run (can be void or main)

main :: func
```

## Contributing

Feel free to contribute to this project by submitting issues or pull requests! 