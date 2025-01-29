# Lab_01 Hello NaiveC

In this lab, I finished my first program based on LLVM, which can output a `.ll` IR file.

We can use LLVM to interpret this IR file, and LLVM will print a string "Hello, NaiveC".

```shell
# Output IR file
./bin/NaiveC > test.ll

# Print "Hello, NaiveC"
../../build-llvm/llvm_install_dir/bin/lli test.ll
```