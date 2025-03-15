# Lab_12 Run More Complex Test Cases

In this lab, we need to fix some problems of our compiler, and enable it to run more complex test cases in `test` directory.

## Tasks

### Task 1

Support `void` type in C language, since user may define a function without returned value.

### Task 2

Support insert default return IR instrcution in the tail(i.e. the final basic block) of a function, when the user don't write an explicit define statement in the input program.

### Task 3

Support array definition without explicit size.

For example:

```C
int ar[] = { 0, 1, 2, 3, 4, 5 };
```

### Task 4

Let our compiler to allow user declare a function (without body) at first, and define it later.

For example, the following code should be legal:
```C
int sum(int a, int b);

int sum(int a, int b) {
    return a + b;
}
```

### Task 5

Support some implicit type conversion:
- Convert pointer type into integer type.
- Convert integer type into pointer type.
- Convert array type into pointer type.

Then, the following programs should be legal:
```C
int square(int* p) {
    if (p != 0) {
        return (*p) * (*p);
    }
    return 0;
}
```

```C
int get_ar_sum(int a[], int len) {
    int sum = 0;
    for (int i = 0; i < len; ++i) {
        sum += a[i];
    }
    return sum;
}

int main() {
    int b[] = { 1, 2, 3, 4 };
    return get_ar_sum(b, sizeof(b) / sizeof(int));
}
```