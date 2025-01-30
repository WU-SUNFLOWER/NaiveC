; ModuleID = 'Expr Module'
source_filename = "Expr Module"

@0 = private unnamed_addr constant [14 x i8] c"expr val: %d\0A\00", align 1
@1 = private unnamed_addr constant [14 x i8] c"expr val: %d\0A\00", align 1
@2 = private unnamed_addr constant [14 x i8] c"expr val: %d\0A\00", align 1
@3 = private unnamed_addr constant [14 x i8] c"expr val: %d\0A\00", align 1
@4 = private unnamed_addr constant [14 x i8] c"expr val: %d\0A\00", align 1
@5 = private unnamed_addr constant [14 x i8] c"expr val: %d\0A\00", align 1
@6 = private unnamed_addr constant [14 x i8] c"expr val: %d\0A\00", align 1

declare i32 @printf(ptr, ...)

define i32 @main() {
entry:
  %0 = call i32 (ptr, ...) @printf(ptr @0, i32 -2)
  %1 = call i32 (ptr, ...) @printf(ptr @1, i32 123)
  %2 = call i32 (ptr, ...) @printf(ptr @2, i32 -92)
  %3 = call i32 (ptr, ...) @printf(ptr @3, i32 -2)
  %4 = call i32 (ptr, ...) @printf(ptr @4, i32 787)
  %5 = call i32 (ptr, ...) @printf(ptr @5, i32 6)
  %6 = call i32 (ptr, ...) @printf(ptr @6, i32 -2600)
  ret i32 0
}
