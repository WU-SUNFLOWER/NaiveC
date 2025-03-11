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
    // Object is something that we need allocate memory for it,
    // including variable and function.
    kObject,
    // Tag is a typename defined by user,
    // including the typename of a struct or union.
    kTag,
};

class Symbol {
 private:
    SymbolKind kind_;
    std::shared_ptr<CType> ctype_;
    llvm::StringRef name_;

 public:
    Symbol(SymbolKind kind, std::shared_ptr<CType> ctype, llvm::StringRef name)
        : kind_(kind), ctype_(ctype), name_(name) {}

    SymbolKind GetSymbolKind() const {
        return kind_;
    }

    std::shared_ptr<CType> GetCType() const {
        return ctype_;
    }

    const llvm::StringRef& GetSymbolName() const {
        return name_;
    }
};

class Env {
 private:
    llvm::StringMap<std::shared_ptr<Symbol>> obj_symbol_table_;
    llvm::StringMap<std::shared_ptr<Symbol>> tag_symbol_table_;

 public:
    llvm::StringMap<std::shared_ptr<Symbol>>& GetObjectSymbolTable() {
        return obj_symbol_table_;
    }

    llvm::StringMap<std::shared_ptr<Symbol>>& GetTagSymbolTable() {
        return tag_symbol_table_;
    }
};

class Scope {
 private:
    std::vector<std::shared_ptr<Env>> envs_;

 public:
    Scope();

    void EnterScope();
    void ExitScope();

    std::shared_ptr<Symbol> FindObjectSymbol(llvm::StringRef name);
    std::shared_ptr<Symbol> FindObjectSymbolInCurrentEnv(llvm::StringRef name);
    void AddObjectSymbol(llvm::StringRef name, std::shared_ptr<CType> ctype);

    std::shared_ptr<Symbol> FindTagSymbol(llvm::StringRef name);
    std::shared_ptr<Symbol> FindTagSymbolInCurrentEnv(llvm::StringRef name);
    void AddTagSymbol(llvm::StringRef name, std::shared_ptr<CType> ctype);
};

#endif  // SCOPE_H_
