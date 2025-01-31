// Copyright 2025 WU-SUNFLOWER. All rights reserved.
// Use of this source code is governed by a GPL-style license that can be
// found in the LICENSE file.

#ifndef DIAG_ENGINE_H_
#define DIAG_ENGINE_H_

#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/FormatVariadic.h"

enum Diag {
#define NAIVEC_DIAG(id, kind, msg) k##id,
#include "diag.inc"
};

class DiagEngine {
 private:
    llvm::SourceMgr& mgr_;

    llvm::SourceMgr::DiagKind GetDiagKind(Diag id);
    const char* GetDiagMsg(Diag id);

 public:
    explicit DiagEngine(llvm::SourceMgr& mgr) : mgr_(mgr) {}

    // NOTE: Template function must be defined in header file!!!
    template <typename... Args>
    void Report(llvm::SMLoc loc, Diag diag_id, Args... args) {
        auto diag_kind = GetDiagKind(diag_id);
        const char* fmt = GetDiagMsg(diag_id);
        auto formated = llvm::formatv(fmt, std::forward<Args>(args)...).str();
        mgr_.PrintMessage(loc, diag_kind, formated);

        if (diag_kind == llvm::SourceMgr::DK_Error) {
            exit(-1);
        }
    }
};

#endif  // DIAG_ENGINE_H_
