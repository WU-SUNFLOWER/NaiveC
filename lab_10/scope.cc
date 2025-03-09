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

std::shared_ptr<Symbol> Scope::FindObjectSymbol(llvm::StringRef name) {
    for (auto iter = envs_.rbegin(); iter != envs_.rend(); ++iter) {
        auto& table = (*iter)->GetObjectSymbolTable();
        if (table.count(name) > 0) {
            return table[name];
        }
    }
    return nullptr;
}

std::shared_ptr<Symbol> Scope::FindObjectSymbolInCurrentEnv(llvm::StringRef name) {
    auto& table = envs_.back()->GetObjectSymbolTable();
    if (table.count(name) > 0) {
        return table[name];
    }
    return nullptr;
}

void Scope::AddObjectSymbol(llvm::StringRef name, std::shared_ptr<CType> ctype) {
    auto symbol = std::make_shared<Symbol>(SymbolKind::kObject, ctype, name);
    envs_.back()->GetObjectSymbolTable().insert({ name, symbol });
}

std::shared_ptr<Symbol> Scope::FindTagSymbol(llvm::StringRef name) {
    for (auto iter = envs_.rbegin(); iter != envs_.rend(); ++iter) {
        auto& table = (*iter)->GetTagSymbolTable();
        if (table.count(name) > 0) {
            return table[name];
        }
    }
    return nullptr;
}

std::shared_ptr<Symbol> Scope::FindTagSymbolInCurrentEnv(llvm::StringRef name) {
    auto& table = envs_.back()->GetTagSymbolTable();
    if (table.count(name) > 0) {
        return table[name];
    }
    return nullptr;
}

void Scope::AddTagSymbol(llvm::StringRef name, std::shared_ptr<CType> ctype) {
    auto symbol = std::make_shared<Symbol>(SymbolKind::kTag, ctype, name);
    envs_.back()->GetTagSymbolTable().insert({ name, symbol });
}
