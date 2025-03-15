# Lab_08 Let's Support More Operators And Pointer Type.

## How to determine the type of an array in C language?

Ok, at first, one-dimensional array is easy to understand for everyone.

```C
int a[10] = {0,1,2};
```

However, what about the array with more than one dimension?  Just like this:

```C
int a[2][3] = { {1, 2, 3}, {4, 5, 6} };
```

While analyzing the dimensions of an array, we should start at its innermost dimension, i.e., we should **look from right to left**.

In this example:
- 1st dimension: We have 3 integers, i.e. `int[3]`
- 2nd dimension: We have 2 `int[3]`, i.e. `int [2][3]`.

⚠This example implies that we need to use **Recursion Method(Backtracing Method)** to determine the type of an array.

As a comparsion, let's recall how we determined the type of a pointer.

For example:

```C
int** p;
```

We just need to **look from left to right** simply.
- 1st dimension: We have a pointer which point to `int` data, i.e. `int*`.
- 2nd dimension: We have a pointer which point to `int*` data, i.e. `int**`.

## Tasks

### Task 1


### Task 2