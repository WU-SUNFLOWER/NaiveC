// Copyright 2025 WU-SUNFLOWER. All rights reserved.
// Use of this source code is governed by a GPL-style license that can be
// found in the LICENSE file.

#ifndef TYPE_H_
#define TYPE_H_

#include <cstdlib>
#include <memory>

#include "llvm/IR/Type.h"

class CType;
class CPrimaryType;
class CPointerType;
class CArrayType;
class CRecordType;
class CFuncType;

class TypeVisitor {
 public:
    virtual ~TypeVisitor() {}
    
    virtual llvm::Type* VisitPrimaryType(CPrimaryType*) = 0;
    virtual llvm::Type* VisitPointerType(CPointerType*) = 0;
    virtual llvm::Type* VisitArrayType(CArrayType*) = 0;
    virtual llvm::Type* VisitRecordType(CRecordType*) = 0;
    virtual llvm::Type* VisitFuncType(CFuncType*) = 0;
};

class CType {
 public:
    enum class TypeKind {
        kInt,
        kPointer,
        kArray,
        kRecord,
        kFunc,
        kVoid,
    };

    enum class TagKind {
        kStruct,
        kUnion,
    };

 protected:
    TypeKind kind_;
    size_t size_;
    size_t align_;

 public:
    CType(TypeKind kind, size_t size, size_t align)
        : kind_(kind), size_(size), align_(align) {}

    virtual ~CType() {}

    virtual llvm::Type* Accept(TypeVisitor* vis) = 0;

    TypeKind GetKind() const {
        return kind_;
    }

    size_t GetSize() const {
        return size_;
    }

    size_t GetAlign() const {
        return align_;
    }

    static std::shared_ptr<CType> const kIntType;
    static std::shared_ptr<CType> const kVoidType;

    static llvm::StringRef GenAnonyRecordName(TagKind tag_kind);
};

class CPrimaryType : public CType {
 private:
    std::shared_ptr<CType> base_type_;

 public:
    CPrimaryType(TypeKind kind, size_t size, size_t align)
        : CType(kind, size, align) {}

    llvm::Type* Accept(TypeVisitor* vis) {
        return vis->VisitPrimaryType(this);
    }

    static bool classof(const CType* ctype) {
        return ctype->GetKind() == TypeKind::kInt;
    }
};

class CPointerType : public CType {
 private:
    std::shared_ptr<CType> base_type_;

 public:
    explicit CPointerType(std::shared_ptr<CType> base_type) 
        : CType(TypeKind::kPointer, 8, 8), base_type_(base_type) {}

    std::shared_ptr<CType> GetBaseType() const {
        return base_type_;
    }

    llvm::Type* Accept(TypeVisitor* vis) {
        return vis->VisitPointerType(this);
    }

    static bool classof(const CType* ctype) {
        return ctype->GetKind() == TypeKind::kPointer;
    }
};

class CArrayType : public CType {
 private:
    std::shared_ptr<CType> element_type_;
    int element_count_;

 public:
    CArrayType(std::shared_ptr<CType> element_type, int element_count)
        : CType(TypeKind::kArray, element_count * element_type->GetSize(), element_type->GetAlign()),
          element_type_(element_type), element_count_(element_count) {}

    std::shared_ptr<CType> GetElementType() const {
        return element_type_;
    }

    int GetElementCount() const {
        return element_count_;
    }

    llvm::Type* Accept(TypeVisitor* vis) {
        return vis->VisitArrayType(this);
    }

    static bool classof(const CType* ctype) {
        return ctype->GetKind() == TypeKind::kArray;
    }
};

class CRecordType : public CType {
 public:
    struct Member {
        std::shared_ptr<CType> type;
        llvm::StringRef name;
        // The offset relative to the starting address of the structure.
        size_t offset;
        // Which member it is in the structure.
        int rank;

        Member() {}

        Member(std::shared_ptr<CType> type, llvm::StringRef name)
            : type(type), name(name), offset(0), rank(0) {}
    };

 private:
    llvm::StringRef name_;
    std::vector<Member> members_;
    TagKind tag_kind_;

    int max_size_member_rank_;

    void ComputeMemberOffsets();
    void ComputeStructMemberOffsets();
    void ComputeUnionMemberOffsets();

 public:
    CRecordType(llvm::StringRef name, TagKind tag_kind);

    llvm::StringRef GetName() const {
        return name_;
    }

    const std::vector<Member>& GetMembers() const {
        return members_;
    }

    void SetMembers(std::vector<Member>&& members);

    int GetMaxSizeMemberRank() const {
        return max_size_member_rank_;
    }

    TagKind GetTagKind() const {
        return tag_kind_;
    }

    llvm::Type* Accept(TypeVisitor* vis) {
        return vis->VisitRecordType(this);
    }

    static bool classof(const CType* ctype) {
        return ctype->GetKind() == TypeKind::kRecord;
    }
};

class CFuncType : public CType {
 public:
    struct Param {
        std::shared_ptr<CType> type;
        llvm::StringRef name;

        Param(std::shared_ptr<CType> type, llvm::StringRef name)
             : type(type), name(name) {}
    };

 private:
    llvm::StringRef func_name_;
    std::shared_ptr<CType> ret_type_;
    std::vector<Param> params_;

 public:
    bool has_body_ { false };

    CFuncType(llvm::StringRef func_name, std::shared_ptr<CType> ret_type, std::vector<Param>&& params)
        : CType(TypeKind::kFunc, 1, 1), func_name_(func_name), ret_type_(ret_type), params_(std::move(params)) {}

    std::shared_ptr<CType> GetRetType() const {
        return ret_type_;
    }

    const std::vector<Param>& GetParams() const {
        return params_;
    }

    llvm::StringRef GetFuncName() const {
        return func_name_;
    }

    llvm::Type* Accept(TypeVisitor* vis) override {
        return vis->VisitFuncType(this);
    }

    static bool classof(const CType* ctype) {
        return ctype->GetKind() == TypeKind::kFunc;
    }
};

#endif  // TYPE_H_
