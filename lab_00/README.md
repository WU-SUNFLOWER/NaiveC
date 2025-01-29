# Build Development Environment

In this lab, we should compile the source code of LLVM.

## Basic Information

OS: Ubuntu 22.04 x86-64

Disk: 72GB

> We need reserve enough space to compile LLVM!!!

## Steps

### 1. Download and install the development tools

```shell
sudo apt -y install gcc g++ git cmake ninja-build zlib1g-dev vim curl
```

### 2. Download the source code of LLVM

https://github.com/llvm/llvm-project/releases/tag/llvmorg-17.0.6

Please download `llvm-project-17.0.6.src.tar.xz`!

### 3. Unpack and compile the source code of LLVM

```shell
mkdir -p build-llvm && cd build-llvm
mkdir -p build llvm_install_dir

# Unpack the source code of LLVM
tar -I 'xz -T0 -d' -xvf <the path of llvm-project-17.0.6.src.tar.xz> -C .

# Rename the folder of the source code
mv llvm-project-17.0.6 llvm-project
```

> Don't try to use the share folder of VMware to store the folder of the source code of LLVM!
> Otherwise, you might make some mistakes!

```shell
# Preparation before compiling
cmake ../llvm-project/llvm  -GNinja -DLLVM_OPTIMIZED_TABLEGEN=ON -DLLVM_TARGETS_TO_BUILD=X86 -DCMAKE_BUILD_TYPE=RelWithDebInfo -DBUILD_SHARED_LIBS=ON -DLLVM_USE_LINKER=gold -DLLVM_ENABLE_PROJECTS=clang -DCMAKE_INSTALL_PREFIX=../llvm_install_dir

# Run compiling task, which will cost about 1~2 hours.
# For example: ninja -j8
ninja -j<the number of cpu cores of your computer>

# Output the binary files of LLVM to `llvm_install_dir` folder
ninja install
```