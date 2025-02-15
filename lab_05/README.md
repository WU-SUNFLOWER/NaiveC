# Lab_05 Introduce IF Statement into Our Minimalist Compiler

## Tasks

### Task 1

The first goal of this lab is to enable our **Lexer** and **Parser** to recognize **the simplest IF Statement**, and generate the correct AST node for it.

The simplest if statement is something like, which only has one statement in its *then block* and *else block*:

```C
int a, b = 4, c = 0;
b = 1000;

if (c)
    a = 10;
else
    a = 100;

a + b;
```

### Task 2

The second goal of this lab is to enable our **Code Generator** to generate the correct LLVM IR for the simplest if statement.

### Task3

The final goal of this lab is to let our compiler to support the if statement with **code block**.