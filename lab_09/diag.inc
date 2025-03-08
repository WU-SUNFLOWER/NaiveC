// Copyright 2025 WU-SUNFLOWER. All rights reserved.
// Use of this source code is governed by a GPL-style license that can be
// found in the LICENSE file.

#ifndef NAIVEC_DIAG
#define NAIVEC_DIAG(id, kind, msg)
#endif

// lexer
NAIVEC_DIAG(ErrUnknownChar, Error, "unknown char '{0}'")

// parser
NAIVEC_DIAG(ErrExpected, Error, "expected '{0}', but found '{1}'")
NAIVEC_DIAG(ErrBreakStmt, Error, "'break' statement not in loop or switch statement")
NAIVEC_DIAG(ErrContinueStmt, Error, "'continue' statement not in loop statement")

// sema
NAIVEC_DIAG(ErrRedefined, Error, "redefined symbol '{0}'")
NAIVEC_DIAG(ErrUndefined, Error, "undefined symbol '{0}'")
NAIVEC_DIAG(ErrLValue, Error, "lvalue is required on the assign operation left side")
NAIVEC_DIAG(ErrType, Error, "typename expected")
NAIVEC_DIAG(ErrSameType, Error, "expected same type")
NAIVEC_DIAG(ErrExpectedType, Error, "expected {0}")
NAIVEC_DIAG(ErrExpectedLValue, Error, "expected lvalue")
NAIVEC_DIAG(ErrExpectedDeclare, Error, "expected {0}")

#undef NAIVEC_DIAG