# Lab_07 Introduce Comment & Logical Operator & Bitwise Operator into Our Compiler

## Tasks

### Task 1

The first goal of this lab is to let our compiler to support **Comment**.

After we finish this task, the following code can be processed by our compiler.

```C
// SOMETHING
int e = 0;
for (int i = 0; i < 100; i = i + 1) {
    if (i == 10) {
        continue;
    }
    /*
        SOMETHING
        SOMETHING
        SOMETHING
    */
    for (int j = 0; j < 10; j = j + 1) {
        e = e + i;
        // SOMETHING               
        if (j >= 8) {
            break;  // SOMETHING
        }
    }
}
e;/*Hello*/
```

### Task 2

The second goal of this lab is to enable our compiler to support **Logical Operator**, which including `&&` and `||`.

### Task3

The final goal of this lab is to let our compiler to **Bitwise Operator**, which including `|`, `^`, `&`, `<<`, `>>` and `%`.

```C
int a = 10;
int b = 20;
int c = 10;
int d = 5;
int e = 2;
int f = 3;

int x = a != 10 && a == 10 || b != 20 && c <= 10 || c < 10  && b == 20 ;
int y = a == 10 && b == 20 && c <= 10 || a != 10 || b != 20 || c < 10;

d = d << 2;
d = d >> 5;
d = d & e;
d = d ^ f;
d = d % 10;
d = d | b;
d = d + x + y;
```