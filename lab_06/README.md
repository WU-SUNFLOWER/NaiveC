# Lab_06 Introduce Relationship Expressions & FOR Statement into Our Compiler

## Tasks

### Task 1

The first goal of this lab is to let our compiler to support Relationship Expressions, which including `==`, `!=`, `<`, `<=`, `>` and `>=`.

After we finish this task, the following code can be processed by our compiler.

```C
int a = 0, b = 4, c = 0;
b = 1000;

if (c != 0) {
    if (a <= 0) {
        a = 13;
        c = 64;
    } else {
        a = 23;
        c = 89;
    }
} 
else if (c < 0) {
    a = 200;
}
else {
    a = 1;
}

a + b + c;
```

### Task 2

The second goal of this lab is to enable our compiler to support **FOR Statement**.

### Task3

The final goal of this lab is to let our compiler to support the `continue` and `break` keyword.