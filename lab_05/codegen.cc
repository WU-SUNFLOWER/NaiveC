// Copyright 2025 WU-SUNFLOWER. All rights reserved.
// Use of this source code is governed by a GPL-style license that can be
// found in the LICENSE file.

#include "codegen.h"

#include <memory>

#include "llvm/IR/Verifier.h"

CodeGen::CodeGen(std::shared_ptr<Program> prog) {
    module_ = std::make_shared<llvm::Module>("Expr Module", context_);
    VisitProgram(prog.get());
}

llvm::Value* CodeGen::VisitProgram(Program *prog) {
    // Build "print" function
    auto print_func_type = llvm::FunctionType::get(
                                    ir_builder_.getInt32Ty(), 
                                    { ir_builder_.getInt8PtrTy() }, 
                                    true);
    auto print_func = llvm::Function::Create(
                                print_func_type, 
                                llvm::GlobalValue::ExternalLinkage, 
                                "printf", 
                                module_.get());

    // Build "main" function
    auto func_type = llvm::FunctionType::get(ir_builder_.getInt32Ty(), false);
    auto func = llvm::Function::Create(func_type, llvm::GlobalValue::ExternalLinkage, "main", module_.get());

    llvm::BasicBlock* entry_block = llvm::BasicBlock::Create(context_, "entry", func);
    ir_builder_.SetInsertPoint(entry_block);

    llvm::Value* result;
    for (auto& node : prog->nodes_) {
        result = node->Accept(this);
    }
    ir_builder_.CreateCall(print_func, { ir_builder_.CreateGlobalStringPtr("expr val: %d\n"), result });

    llvm::Value* ret = ir_builder_.CreateRet(ir_builder_.getInt32(0));
    llvm::verifyFunction(*func);

    module_->print(llvm::outs(), nullptr);

    return ret;
}

llvm::Value* CodeGen::VisitBinaryExpr(BinaryExpr *binary_expr) {
    llvm::Value* left = binary_expr->left_->Accept(this);
    llvm::Value* right = binary_expr->right_->Accept(this);

    switch (binary_expr->op_) {
        case OpCode::kAdd:
            return ir_builder_.CreateNSWAdd(left, right);
        case OpCode::kSub:
            return ir_builder_.CreateNSWSub(left, right);
        case OpCode::kMul:
            return ir_builder_.CreateNSWMul(left, right);
        case OpCode::kDiv:
            return ir_builder_.CreateSDiv(left, right);
        default:
            llvm::errs() << "Unknown opcode: " 
                         << static_cast<int>(binary_expr->op_) 
                         << "\n";
    }

    return nullptr;
}

llvm::Value* CodeGen::VisitVariableAccessExpr(VariableAccessExpr* access_node) {
    const llvm::StringRef& variable_name = access_node->GetVariableName();
    auto& variable_info = variable_map_[variable_name];

    llvm::Value* variable_addr = variable_info.first;
    llvm::Type* variable_ir_type = variable_info.second;

    auto ret = ir_builder_.CreateLoad(variable_ir_type, variable_addr, variable_name);

    return ret;
}

llvm::Value* CodeGen::VisitVariableDecl(VariableDecl* decl_node) {
    llvm::Type* ir_type = nullptr;
    const llvm::StringRef& variable_name = decl_node->GetVariableName();
    if (decl_node->GetCType() == CType::GetIntType()) {
        ir_type = ir_builder_.getInt32Ty();
    }
    else {
        llvm::errs() << "Try to delcare variable with unknown type: " << variable_name;
        return nullptr;
    }

    llvm::Value* value = ir_builder_.CreateAlloca(ir_type, nullptr, variable_name);
    variable_map_.insert({ variable_name, { value, ir_type } });

    return value;
}

llvm::Value* CodeGen::VisitAssignExpr(AssignExpr* assign_node) {
    auto access_node = assign_node->GetLeftChild();

    assert(llvm::isa<VariableAccessExpr>(access_node.get()));

    auto variable_name = static_cast<VariableAccessExpr*>(access_node.get())->GetVariableName();
    auto& variable_info = variable_map_[variable_name];
    llvm::Value* variable_addr = variable_info.first;
    llvm::Type* variable_ir_type = variable_info.second;

    // Compute the right node at first.
    auto right_node = assign_node->GetRightChild();
    llvm::Value* right_value = right_node->Accept(this);

    // Generate store ir instruction.
    ir_builder_.CreateStore(right_value, variable_addr);

    // Return our target variable as a lvalue.
    // auto ret = ir_builder_.CreateLoad(variable_ir_type, variable_addr, variable_name);
    return ir_builder_.CreateLoad(variable_ir_type, variable_addr, variable_name);
}

llvm::Value* CodeGen::VisitNumberExpr(NumberExpr *factor_expr) {
    return ir_builder_.getInt32(factor_expr->GetNumber());
}
