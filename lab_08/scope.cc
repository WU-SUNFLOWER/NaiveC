// Copyright 2025 WU-SUNFLOWER. All rights reserved.
// Use of this source code is governed by a GPL-style license that can be
// found in the LICENSE file.

#include "scope.h"

#include <memory>

Scope::Scope() {
    EnterScope();
}

void Scope::EnterScope() {
    envs_.emplace_back(std::make_shared<Env>());
}

void Scope::ExitScope() {
    envs_.pop_back();
}

std::shared_ptr<Symbol> Scope::FindVarSymbol(llvm::StringRef name) {
    for (auto iter = envs_.rbegin(); iter != envs_.rend(); ++iter) {
        auto& table = (*iter)->GetVariableSymbolTable();
        if (table.count(name) > 0) {
            return table[name];
        }
    }
    return nullptr;
}

std::shared_ptr<Symbol> Scope::FindVarSymbolInCurrentEnv(llvm::StringRef name) {
    auto& table = envs_.back()->GetVariableSymbolTable();
    if (table.count(name) > 0) {
        return table[name];
    }
    return nullptr;
}

void Scope::AddSymbol(llvm::StringRef name, SymbolKind kind, std::shared_ptr<CType> ctype) {
    auto symbol = std::make_shared<Symbol>(kind, ctype, name);
    envs_.back()->GetVariableSymbolTable().insert({ name, symbol });
}
