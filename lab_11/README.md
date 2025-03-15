# Lab_11 Function and Global Variable

## Function

### The usage of function

```C
// The definition of function
int add(int a, int b) {
		return a+b;
}

int main() {
	// Call function.
	return add(1,2);
}
```

### The symbol of function

In our compiler, we treat function as first-class citizen **in an approximate sense**.

And we use **the symbol table of root scope** to maintain the symbol of function and global variable.

### Grammar

1. We treat that our program consists of functions.
```EBNF
prog       ::= func-def +
```

2. The declaration and definition of a function.
```EBNF
func-def   ::= decl-spec declarator block-stmt

declarator ::= "*"* direct-declarator
direct-declarator ::= identifier | direct-declarator "[" assign "]" 
											| direct-declarator "(" parameter-type-list? ")"
parameter-type-list ::= decl-spec declarator ("," decl-spec declarator)* (", " "...")?
```

3. The call expression for function
```EBNF
postfix     ::= primary | postfix ("++"|"--") | postfix "[" expr "]"
								| postfix "." identifier
								| postfix "->" identifier 
								| postfix "(" arg-expr-list? ")"
arg-expr-list := assign ("," assign)*
```

### Build LLVM type

```CPP
llvm::FunctionType * funcTy = llvm::dyn_cast<llvm::FunctionType>(decl->ty->Accept(this));
func = Function::Create(funcTy, GlobalValue::ExternalLinkage, cFuncTy->GetName(), module.get());
```

## Global variable

### The usage of global variable

```C
int a;  // The declaration of a global variable.

int main(){
	a = 3;
	return a;
}
```

### The symbol of global variable

We use **the symbol table of root scope** to maintain the symbol of function and global variable.

### Grammar

1. Now, we treat that our program consists of a serials of functions and global variables.
```ebnf
prog          ::= (external-decl)+
external-decl ::= func-def | decl-stmt
```

2. We reuse the rules for parsing normal variable, to parse global variable.
```ebnf
decl-stmt  ::= decl-spec init-declarator-list? ";"
```

### Build LLVM type

```cpp
llvm::GlobalVariable *globalVar = new llvm::GlobalVariable(
                                                *module, 
                                                ty, 
                                                false, 
                                                llvm::GlobalValue::ExternalLinkage, 
                                                nullptr, 
                                                text);
```