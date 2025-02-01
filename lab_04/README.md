# Lab_03 Introduce Diagnostic Engine and Unit Testing into Our Minimalist Compiler

## Tasks

### Task 1

The first goal of this lab is to introduce **Diagnostic Engine** into our minimalist compiler.

So, what is Diagnostic Engine?

For example, if you input a text with grammar or semantic errors, like:

```c
int x = 1, y = 2;
z = 3;
``` 

Then, when our **Lexer** or **Parser** detect an error, we want them to report it to **Diagnostic Engine**, which will print this error to terminal in a **user-friendly manner**, and let our compiler exit directly.

Just like:
```shell
./test/expr.txt:2:1: error: undefined symbol 'z'
z = 3;
^
```

### Task 2

The second goal of this lab is to introduce **Unit Testing** into our project.