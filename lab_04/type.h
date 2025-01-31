// Copyright 2025 WU-SUNFLOWER. All rights reserved.
// Use of this source code is governed by a GPL-style license that can be
// found in the LICENSE file.

#ifndef TYPE_H_
#define TYPE_H_

#include <cstdlib>

enum class TypeKind {
    kInt,
};

class CType {
 private:
    TypeKind kind_;
    size_t size_;
    size_t align_;

 public:
    CType(TypeKind kind, size_t size, size_t align)
        : kind_(kind), size_(size), align_(align) {}

    static CType* GetIntType();
};

#endif  // TYPE_H_
