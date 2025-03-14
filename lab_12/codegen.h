// Copyright 2025 WU-SUNFLOWER. All rights reserved.
// Use of this source code is governed by a GPL-style license that can be
// found in the LICENSE file.

#ifndef CODEGEN_H_
#define CODEGEN_H_

#include <memory>

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/ADT/DenseMap.h"

#include "ast.h"
#include "parser.h"
#include "type.h"

class CodeGen : public Visitor, public TypeVisitor {
 private:
    llvm::LLVMContext context_;
    llvm::IRBuilder<> ir_builder_ { context_ };

    std::unique_ptr<llvm::Module> module_ { nullptr };

    llvm::Function* cur_func_ { nullptr };

    void SetCurrentFunc(llvm::Function* func) {
        cur_func_ = func;
    }

    llvm::Function* GetCurrentFunc() const {
        return cur_func_;
    }

    llvm::DenseMap<AstNode*, llvm::BasicBlock*> break_block_map_;
    llvm::DenseMap<AstNode*, llvm::BasicBlock*> continue_block_map_;

 private:
    llvm::StringMap<std::pair<llvm::Value*, llvm::Type*>> global_variable_map_;
    llvm::SmallVector<llvm::StringMap<std::pair<llvm::Value*, llvm::Type*>>> local_variable_map_;    

    void AddLocalVariable(llvm::StringRef name, llvm::Value* addr, llvm::Type* llvm_type);
    void AddGlobalVariable(llvm::StringRef name, llvm::Value* addr, llvm::Type* llvm_type);
    std::pair<llvm::Value*, llvm::Type*> GetVariableByName(llvm::StringRef name);

    void PushScope();
    void PopScope();
    void ClearVariableScope();

 public:
    explicit CodeGen(std::shared_ptr<Program> prog);

    std::unique_ptr<llvm::Module>& GetModule() {
        return module_;
    }

 public:
    llvm::Value* VisitProgram(Program*) override;

    llvm::Value* VisitDeclStmt(DeclStmt*) override;
    llvm::Value* VisitBlockStmt(BlockStmt*) override;
    llvm::Value* VisitIfStmt(IfStmt*) override;
    llvm::Value* VisitForStmt(ForStmt*) override;
    llvm::Value* VisitBreakStmt(BreakStmt*) override;
    llvm::Value* VisitContinueStmt(ContinueStmt*) override;

    llvm::Value* VisitUnaryExpr(UnaryExpr*) override;
    llvm::Value* VisitBinaryExpr(BinaryExpr*) override;
    llvm::Value* VisitTernaryExpr(TernaryExpr*) override;

    llvm::Value* VisitNumberExpr(NumberExpr*) override;
    llvm::Value* VisitVariableAccessExpr(VariableAccessExpr*) override;
    llvm::Value* VisitVariableDecl(VariableDecl*) override;

 private:
    std::shared_ptr<VariableDecl::InitValue> GetInitValueStructByIndexList(
                                                        const VariableDecl* decl_node, 
                                                        const std::vector<int>& index);
    llvm::Constant* GetInitialValueForGlobalVariable(
                                    const VariableDecl* decl_node, 
                                    llvm::Type*, 
                                    std::vector<int>&);
    llvm::Value* VisitLocalVariableDecl(VariableDecl*);
    llvm::Value* VisitGlobalVariableDecl(VariableDecl*);

 public:
    llvm::Value* VisitSizeofExpr(SizeofExpr*) override;

    llvm::Value* VisitPostIncExpr(PostIncExpr*) override;
    llvm::Value* VisitPostDecExpr(PostDecExpr*) override;

    llvm::Value* VisitPostSubscript(PostSubscriptExpr*) override;

    llvm::Value* VisitPostMemberDotExpr(PostMemberDotExpr*) override;
    llvm::Value* VisitPostMemberArrowExpr(PostMemberArrowExpr*) override;

    llvm::Value* VisitFuncDecl(FuncDecl*) override;
    llvm::Value* VisitPostFuncCallExpr(PostFuncCallExpr*) override;
    llvm::Value* VisitReturnStmt(ReturnStmt*) override;

    // Convert NaiveC type object to LLVM type object by these methods.
    llvm::Type* VisitPrimaryType(CPrimaryType*) override;
    llvm::Type* VisitPointerType(CPointerType*) override;
    llvm::Type* VisitArrayType(CArrayType*) override;
    llvm::Type* VisitRecordType(CRecordType*) override;
    llvm::Type* VisitFuncType(CFuncType*) override;

 private:
    void CastValue(llvm::Value** value, llvm::Type* dest_type);
};

#endif  // CODEGEN_H_
