// Copyright 2025 WU-SUNFLOWER. All rights reserved.
// Use of this source code is governed by a GPL-style license that can be
// found in the LICENSE file.

#include "type.h"

CType* CType::GetIntType() {
    static CType type(TypeKind::kInt, 4, 4);
    return &type;
}
