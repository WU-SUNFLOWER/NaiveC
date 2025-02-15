; ModuleID = 'Expr Module'
source_filename = "Expr Module"

@0 = private unnamed_addr constant [14 x i8] c"expr val: %d\0A\00", align 1

declare i32 @printf(ptr, ...)

define i32 @main() {
entry:
  %a = alloca i32, align 4
  store i32 0, ptr %a, align 4
  %a1 = load i32, ptr %a, align 4
  %b = alloca i32, align 4
  store i32 4, ptr %b, align 4
  %b2 = load i32, ptr %b, align 4
  %c = alloca i32, align 4
  store i32 -100, ptr %c, align 4
  %c3 = load i32, ptr %c, align 4
  store i32 1000, ptr %b, align 4
  %b4 = load i32, ptr %b, align 4
  br label %cond

cond:                                             ; preds = %entry
  %c5 = load i32, ptr %c, align 4
  %0 = icmp ne i32 %c5, 0
  br i1 %0, label %then, label %else

then:                                             ; preds = %cond
  br label %cond6

final:                                            ; preds = %else, %final8
  %a14 = load i32, ptr %a, align 4
  %b15 = load i32, ptr %b, align 4
  %1 = add nsw i32 %a14, %b15
  %c16 = load i32, ptr %c, align 4
  %2 = add nsw i32 %1, %c16
  %3 = call i32 (ptr, ...) @printf(ptr @0, i32 %2)
  ret i32 0

else:                                             ; preds = %cond
  store i32 200, ptr %a, align 4
  %a13 = load i32, ptr %a, align 4
  br label %final

cond6:                                            ; preds = %then
  %a10 = load i32, ptr %a, align 4
  %4 = icmp ne i32 %a10, 0
  br i1 %4, label %then7, label %else9

then7:                                            ; preds = %cond6
  store i32 13, ptr %a, align 4
  %a11 = load i32, ptr %a, align 4
  br label %final8

final8:                                           ; preds = %else9, %then7
  br label %final

else9:                                            ; preds = %cond6
  store i32 23, ptr %a, align 4
  %a12 = load i32, ptr %a, align 4
  br label %final8
}
