// Copyright 2024 WU-SUNFLOWER. All rights reserved.
// Use of this source code is governed by a GPL-style license that can be
// found in the LICENSE file.

#include <iostream>

#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Verifier.h"

int main() {
    llvm::LLVMContext context;
    llvm::Module module("My First Module", context);
    llvm::IRBuilder ir_builder(context);

    llvm::FunctionType* puts_func_type = llvm::FunctionType::get(
                                                ir_builder.getInt32Ty(),
                                                { ir_builder.getInt8PtrTy() },
                                                false);
    llvm::Function* puts_func = llvm::Function::Create(
                                        puts_func_type, 
                                        llvm::GlobalValue::LinkageTypes::ExternalLinkage, 
                                        "puts", 
                                        module);

    llvm::Constant* str = llvm::ConstantDataArray::getString(context, "Hello, NaiveC");
    llvm::GlobalVariable* global_var = new llvm::GlobalVariable(
                            module,
                            str->getType(),
                            true,
                            llvm::GlobalValue::LinkageTypes::PrivateLinkage,
                            str,
                            "kString");

    llvm::FunctionType* main_func_type = llvm::FunctionType::get(ir_builder.getInt32Ty(), false);
    llvm::Function* main_func = llvm::Function::Create(
                                main_func_type, 
                                llvm::GlobalValue::LinkageTypes::ExternalLinkage, 
                                "main", 
                                module);

    llvm::BasicBlock* entry_block = llvm::BasicBlock::Create(context, "entry", main_func);
    ir_builder.SetInsertPoint(entry_block);

    // Create a "Get Element Pointer" instruction.
    llvm::Value* gep = ir_builder.CreateGEP(
                                    global_var->getType(), 
                                    global_var, 
                                    { ir_builder.getInt64(0), ir_builder.getInt64(0) });

    ir_builder.CreateCall(puts_func, { gep });
    ir_builder.CreateRet(ir_builder.getInt32(0));

    llvm::verifyFunction(*main_func, &llvm::errs());
    llvm::verifyModule(module, &llvm::errs());

    module.print(llvm::outs(), nullptr);

    return 0;
}
