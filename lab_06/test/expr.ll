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
  store i32 0, ptr %c, align 4
  %c3 = load i32, ptr %c, align 4
  store i32 1000, ptr %b, align 4
  %b4 = load i32, ptr %b, align 4
  br label %cond

cond:                                             ; preds = %entry
  %c5 = load i32, ptr %c, align 4
  %0 = icmp ne i32 %c5, 0
  %1 = sext i1 %0 to i32
  %2 = icmp ne i32 %1, 0
  br i1 %2, label %then, label %else

then:                                             ; preds = %cond
  br label %cond6

final:                                            ; preds = %final17, %final8
  %e = alloca i32, align 4
  store i32 0, ptr %e, align 4
  %e22 = load i32, ptr %e, align 4
  br label %for.init

else:                                             ; preds = %cond
  br label %cond15

cond6:                                            ; preds = %then
  %a10 = load i32, ptr %a, align 4
  %3 = icmp sle i32 %a10, 0
  %4 = sext i1 %3 to i32
  %5 = icmp ne i32 %4, 0
  br i1 %5, label %then7, label %else9

then7:                                            ; preds = %cond6
  store i32 13, ptr %a, align 4
  %a11 = load i32, ptr %a, align 4
  store i32 64, ptr %c, align 4
  %c12 = load i32, ptr %c, align 4
  br label %final8

final8:                                           ; preds = %else9, %then7
  br label %final

else9:                                            ; preds = %cond6
  store i32 23, ptr %a, align 4
  %a13 = load i32, ptr %a, align 4
  store i32 89, ptr %c, align 4
  %c14 = load i32, ptr %c, align 4
  br label %final8

cond15:                                           ; preds = %else
  %c19 = load i32, ptr %c, align 4
  %6 = icmp slt i32 %c19, 0
  %7 = sext i1 %6 to i32
  %8 = icmp ne i32 %7, 0
  br i1 %8, label %then16, label %else18

then16:                                           ; preds = %cond15
  store i32 200, ptr %a, align 4
  %a20 = load i32, ptr %a, align 4
  br label %final17

final17:                                          ; preds = %else18, %then16
  br label %final

else18:                                           ; preds = %cond15
  store i32 1, ptr %a, align 4
  %a21 = load i32, ptr %a, align 4
  br label %final17

for.init:                                         ; preds = %final
  %i = alloca i32, align 4
  store i32 0, ptr %i, align 4
  %i23 = load i32, ptr %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %for.init
  %i24 = load i32, ptr %i, align 4
  %9 = icmp slt i32 %i24, 100
  %10 = sext i1 %9 to i32
  %11 = icmp ne i32 %10, 0
  br i1 %11, label %for.body, label %for.final

for.inc:                                          ; preds = %for.body
  %i28 = load i32, ptr %i, align 4
  %12 = add nsw i32 %i28, 1
  store i32 %12, ptr %i, align 4
  %i29 = load i32, ptr %i, align 4
  br label %for.cond

for.body:                                         ; preds = %for.cond
  %e25 = load i32, ptr %e, align 4
  %i26 = load i32, ptr %i, align 4
  %13 = add nsw i32 %e25, %i26
  store i32 %13, ptr %e, align 4
  %e27 = load i32, ptr %e, align 4
  br label %for.inc

for.final:                                        ; preds = %for.cond
  %a30 = load i32, ptr %a, align 4
  %b31 = load i32, ptr %b, align 4
  %14 = add nsw i32 %a30, %b31
  %c32 = load i32, ptr %c, align 4
  %15 = add nsw i32 %14, %c32
  %e33 = load i32, ptr %e, align 4
  %16 = add nsw i32 %15, %e33
  %i34 = load i32, ptr %i, align 4
  %17 = add nsw i32 %16, %i34
  %18 = call i32 (ptr, ...) @printf(ptr @0, i32 %17)
  ret i32 0
}
