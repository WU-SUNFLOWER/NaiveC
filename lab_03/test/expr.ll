; ModuleID = 'Expr Module'
source_filename = "Expr Module"

@0 = private unnamed_addr constant [14 x i8] c"expr val: %d\0A\00", align 1

declare i32 @printf(ptr, ...)

define i32 @main() {
entry:
  %z = alloca i32, align 4
  %x = alloca i32, align 4
  store i32 3, ptr %x, align 4
  %y = alloca i32, align 4
  store i32 4, ptr %y, align 4

  %z1 = load i32, ptr %z, align 4
  
  %z2 = load i32, ptr %z, align 4
  %x3 = load i32, ptr %x, align 4
  %0 = add nsw i32 %z2, %x3

  %y4 = load i32, ptr %y, align 4
  %1 = mul nsw i32 %y4, 4

  %2 = add nsw i32 %0, %1

  %3 = add nsw i32 %2, 5
  
  %4 = call i32 (ptr, ...) @printf(ptr @0, i32 %3)
  ret i32 0
}
