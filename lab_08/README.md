# Lab_08 Let's Support More Operators And Pointer Type.

## Tasks

### Task 1

Support commonly used **unary, binary, and ternary expressions** in C language.

Like `i++`, `i--`, `++i`, `--i`, `i += 3`, `i *= 3`, `a ? b : c`, `a, b, c`, `sizeof(a)`, ...

### Task 2

Introduce **the pointer type** into our compiler.

This task can be spilt into the following steps:
- Define **the corresponding class** of C language pointer type in our compiler.
- Enable our Lexer and Parser to process **variable declaration with pointer type**, like `int** p` or `int *q`.
- Enable our Lexer and Parser to process **the address operator** and **the dereference operator**, like `&a` or `*a`.
    - **NOTE:** 
    - We can only get the address of a lvalue with the address operator in C language.
    - Which means we have to add an additional field to our AST node, to mark up the output result of an AST node is lvalue or rvalue. 
    - So that our **semantic checker** can check whether an address acquisition operation is legal.
- Enable our code generator to generate the correct LLVM IR for **the address operator** and **the dereference operator**.
    - **NOTE:**
    - Although we have implement some add/sub operators, including `+`, `-`, `+=` and `-=`, we shouldn't forget that the add/sub operation for pointer is different with normal variable. 
    - For example, Assuming `p` is an int pointer, then `p+1` means adding a four-byte offset to the address `p`.

When all the works are done, the program like this can be processed by our compiler:

```C
int a = 10;
int *p = &a;
*p++;
++*p;
p++;
p--
++p;
--p;
```