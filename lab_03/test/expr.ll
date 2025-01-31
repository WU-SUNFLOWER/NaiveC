; ModuleID = 'Expr Module'
source_filename = "Expr Module"

@0 = private unnamed_addr constant [14 x i8] c"expr val: %d\0A\00", align 1

declare i32 @printf(ptr, ...)

define i32 @main() {
entry:
  %a = alloca i32, align 4
  %b = alloca i32, align 4
  store i32 4, ptr %b, align 4
  %b1 = load i32, ptr %b, align 4
  %c = alloca i32, align 4
  store i32 0, ptr %c, align 4
  %c2 = load i32, ptr %c, align 4
  store i32 1000, ptr %b, align 4
  %b3 = load i32, ptr %b, align 4
  %b4 = load i32, ptr %b, align 4
  %0 = add nsw i32 %b4, 3
  store i32 %0, ptr %c, align 4
  %c5 = load i32, ptr %c, align 4
  store i32 %c5, ptr %a, align 4
  %a6 = load i32, ptr %a, align 4
  %a7 = load i32, ptr %a, align 4
  %b8 = load i32, ptr %b, align 4
  %1 = mul nsw i32 %b8, 4
  %2 = add nsw i32 %a7, %1
  %c9 = load i32, ptr %c, align 4
  %3 = add nsw i32 5, %c9
  %4 = sub nsw i32 %2, %3
  %5 = call i32 (ptr, ...) @printf(ptr @0, i32 %4)
  ret i32 0
}
