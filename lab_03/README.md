# Lab_03 Introduce Variables into Our Minimalist Compiler

The goal of this lab is to introduce variables into our minimalist compiler, which can compute some simple expressions like this:
```
int a, b = 4, c = 0;
b = 1000;
a = c = b + 3;
a + b * 4 - (5 + c);
``` 

## Step by step

The grammar rules in this lab:
```
prog : stmt*     
stmt : decl-stmt | expr-stmt | null-stmt      
null-stmt : ";"      
decl-stmt : "int" identifier ("," identifier (= expr)?)* ";"
expr-stmt : expr ";"
expr : assign-expr | add-expr
assign-expr: identifier "=" expr
add-expr : mult-expr (("+" | "-") mult-expr)* 
mult-expr : primary-expr (("*" | "/") primary-expr)* 
primary-expr : identifier | number | "(" expr ")" 
number: ([0-9])+ 
identifier : (a-zA-Z_)(a-zA-Z0-9_)*
```

To achieve this goal, I broke it into these small tasks and finished them step by step:
1. Modify our **Lexer**, so that it can identify:
    - **The variable declare statement**, like `int x, y = 1, z = 2`.
    - **The variable access expression**, like `x * 3 + 1;`.
    - **The variable assignment expression**, like `x = 123;`.
1. Define some new kinds of AST nodes:
    - **The variable declare node**
    - **The variable access node**
    - **The variable assignment node**
1. Build our **Scope Manager**, which can manager the (variable) symbol information in each scope. And we can use it to check some mistakes of the input text, like:
    - Redefining a variable.
    - Try to access an undefined variable.
1. Build our **Semantic Analyzer**, which provides the following services:
    - Check the token information provided by **Parser**, with the help from **Scope Manager** , to make sure the semantics of the input text are correct.
    - If passed check, allocate and initialize the corresponding AST node, and return it to **Parser**.
1. Modify our **Parser**, so that it can build AST, according to the token stream, with the help from **Semantic Analyzer**.
1. Modify our **Code Generator**, so that it can process these new nodes correctly.

## Test commands

Output LLVM IR from our finished program:
```shell
./bin/NaiveC ./test/expr.txt > ./test/expr.ll
```

Use LLVM to interpret IR:
```shell
../../build-llvm/build/bin/lli ./test/expr.ll
```

So, if the content of your `test/expr.txt` is:
```
int a, b = 4, c = 0;
b = 1000;
a = c = b + 3;
a + b * 4 - (5 + c);
```

Then you will get the result:
```
expr val: 3995
```