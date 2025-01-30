# Lab_02 Build a Minimalist Compiler Without Variables

The goal of this lab is to build a minimalist compiler without variables, which can compute some simple expressions like this:
```
1+3;
3*(4-3)+5*4;
100/25*4;
``` 

As we known, this is a kind of the most simple programming language, which follows the following grammar rules:
```
prog : (expr? ";")*
expr : term (("+" | "-") term)* ;
term : factor (("*" | "/") factor)* ;
factor : number | "(" expr ")" ;
number: ([0-9])+ ;
```

To achieve this goal, I broke it into these small tasks and finished them step by step:
- Implement a **Lexer**, which can read **the text file** and convert it into **token stream**.
- Implement a **Parser**, which can convert token stream into **abstract syntax tree**.
- Implement a **Code Generator**, which can convert abstract syntax tree into **LLVM IR**, and let LLVM to interpret it.