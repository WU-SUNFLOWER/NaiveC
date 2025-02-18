// Copyright 2025 WU-SUNFLOWER. All rights reserved.
// Use of this source code is governed by a GPL-style license that can be
// found in the LICENSE file.

#ifndef TYPE_H_
#define TYPE_H_

#include <cstdlib>
#include <memory>

class CType {
 public:
    enum class TypeKind {
        kInt,
        kPointer,
    };

 private:
    TypeKind kind_;
    size_t size_;
    size_t align_;

 public:
    CType(TypeKind kind, size_t size, size_t align)
        : kind_(kind), size_(size), align_(align) {}

    virtual ~CType() {}

    TypeKind GetKind() const {
        return kind_;
    }

    static std::shared_ptr<CType> const kIntType;
};

class CPrimaryType : public CType {
 private:
    std::shared_ptr<CType> base_type_;

 public:
    CPrimaryType(TypeKind kind, size_t size, size_t align)
        : CType(kind, size, align) {}

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

    static bool classof(const CType* ctype) {
        return ctype->GetKind() == TypeKind::kPointer;
    }
};

#endif  // TYPE_H_
