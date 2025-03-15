# Lab_10 Struct and Union

## What about struct and union

### The basic usage of struct and union

```C
struct List {
		int val;
		struct List *next;
};

struct List n;
n.val = 10;
n.next = 0;

struct List head;
head.val = 0;
head.next = &n; 

/// union
union Value {
	float f;
	double d;
	int i;
};

union Value v;
v.f = 1.0f;
v.d = 2.0;
v.i = 10;
```

### The scope and name space of struct

First, we should look the code in struct's definition as a new scope.

Second, the type name(i.e. tag) of a defined struct is managed in a special name space, different with the name space of normal variable and function.

According to first and second, the following C program is valid:

```c
struct A {int A;};
int A;
```

Third, for the code in struct's definition, we assume that the struct has already been defined, so we can use it validly.

According to first and second, the following C program is valid:
```C
struct List {
    int val;
    struct List *next;
};
```

## Tasks

### Task1

Support the declaration of a struct type.

```ebnf
decl-spec  ::= "int" | struct-or-union-specifier
struct-or-union-specifier ::= struct-or-union identifier "{" (decl-spec declarator(, declarator)* ";")+ "}"
														  struct-or-union identifier
struct-or-union := "struct" | "union"
```

Just like array, in code generating stage, we should build the corresponding LLVM type for user's defined struct.

```C++
llvm::Type * CodeGen::VisitRecordType(CRecordType *ty) {
    llvm::StructType *structType = nullptr;
    structType = llvm::StructType::getTypeByName(context, ty->GetName());
    if (structType) {
        return structType;
    }
    structType = llvm::StructType::create(context, ty->GetName());
    // structType->setName(ty->GetName());

    TagKind tagKind = ty->GetTagKind();

    if (tagKind == TagKind::kStruct) {
        llvm::SmallVector<llvm::Type *> vec;
        for (const auto &m : ty->GetMembers()) {
            vec.push_back(m.ty->Accept(this));
        }
        structType->setBody(vec);
    }else {
        llvm::SmallVector<llvm::Type *> vec;
        const auto &members = ty->GetMembers();
        int idx = ty->GetMaxElementIdx();
        structType->setBody(members[idx].ty->Accept(this));
    }
    // structType->print(llvm::outs());
    return structType;
}
```

### Task2

Support the initialization of a struct-type variable.

Further refine the `Parser::ParseInitializer` and `CodeGen::VisitVariableDecl` function.

### Task3

Support the `.` and `->` operator for struct variable and struct pointer.

```ebnf
postfix     ::= primary | postfix ("++"|"--") | postfix "[" expr "]"
								| postfix "." identifier
								| postfix "->" identifier
```
