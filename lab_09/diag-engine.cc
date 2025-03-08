// Copyright 2025 WU-SUNFLOWER. All rights reserved.
// Use of this source code is governed by a GPL-style license that can be
// found in the LICENSE file.

#include "diag-engine.h"

#include "llvm/Support/SourceMgr.h"

static const char* kDiagMsg[] = {
#define NAIVEC_DIAG(id, kind, msg) msg,
#include "diag.inc"
};

static llvm::SourceMgr::DiagKind kDiagKind[] = {
#define NAIVEC_DIAG(id, kind, msg) llvm::SourceMgr::DK_##kind,
#include "diag.inc"
};

llvm::SourceMgr::DiagKind DiagEngine::GetDiagKind(Diag id) {
    assert(0 <= id && id < sizeof(kDiagMsg) / sizeof(const char*));
    return kDiagKind[id];
}

const char* DiagEngine::GetDiagMsg(Diag id) {
    assert(0 <= id && id < sizeof(kDiagMsg) / sizeof(const char*));
    return kDiagMsg[id];
}
