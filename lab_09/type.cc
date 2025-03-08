// Copyright 2025 WU-SUNFLOWER. All rights reserved.
// Use of this source code is governed by a GPL-style license that can be
// found in the LICENSE file.

#include "type.h"

std::shared_ptr<CType> const CType::kIntType = std::make_shared<CPrimaryType>(TypeKind::kInt, 4, 4);
