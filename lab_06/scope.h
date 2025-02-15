// Copyright 2025 WU-SUNFLOWER. All rights reserved.
// Use of this source code is governed by a GPL-style license that can be
// found in the LICENSE file.

#ifndef SCOPE_H_
#define SCOPE_H_

#include <vector>
#include <memory>

#include "llvm/ADT/StringMap.h"

#include "type.h"

enum class SymbolKind {
    kLocalVariable,
};

class Symbol {
 private:
    SymbolKind kind_;
    CType* ctype_;
    llvm::StringRef name_;

 public:
    Symbol(SymbolKind kind, CType* ctype, llvm::StringRef name)
        : kind_(kind), ctype_(ctype), name_(name) {}

    SymbolKind GetSymbolKind() const {
        return kind_;
    }

    CType* GetCType() const {
        return ctype_;
    }

    const llvm::StringRef& GetSymbolName() const {
        return name_;
    }
};

class Env {
 private:
    llvm::StringMap<std::shared_ptr<Symbol>> variable_symbol_table_;

 public:
    llvm::StringMap<std::shared_ptr<Symbol>>& GetVariableSymbolTable() {
        return variable_symbol_table_;
    }
};

class Scope {
 private:
    std::vector<std::shared_ptr<Env>> envs_;

 public:
    Scope();

    void EnterScope();
    void ExitScope();
    
    std::shared_ptr<Symbol> FindVarSymbol(llvm::StringRef name);
    std::shared_ptr<Symbol> FindVarSymbolInCurrentEnv(llvm::StringRef name);

    void AddSymbol(llvm::StringRef name, SymbolKind kind, CType* ctype);
};

#endif  // SCOPE_H_
