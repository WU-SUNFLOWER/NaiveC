; ModuleID = 'Expr Module'
source_filename = "Expr Module"

@0 = private unnamed_addr constant [14 x i8] c"expr val: %d\0A\00", align 1

declare i32 @printf(ptr, ...)

define i32 @main() {
entry:
  %e = alloca i32, align 4
  store i32 0, ptr %e, align 4
  %e1 = load i32, ptr %e, align 4
  br label %for.init

for.init:                                         ; preds = %entry
  %i = alloca i32, align 4
  store i32 0, ptr %i, align 4
  %i2 = load i32, ptr %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %for.init
  %i3 = load i32, ptr %i, align 4
  %0 = icmp slt i32 %i3, 100
  %1 = sext i1 %0 to i32
  %2 = icmp ne i32 %1, 0
  br i1 %2, label %for.body, label %for.final

for.inc:                                          ; preds = %for.final9, %then
  %i21 = load i32, ptr %i, align 4
  %3 = add nsw i32 %i21, 1
  store i32 %3, ptr %i, align 4
  %i22 = load i32, ptr %i, align 4
  br label %for.cond

for.body:                                         ; preds = %for.cond
  br label %cond

for.final:                                        ; preds = %for.cond
  %e23 = load i32, ptr %e, align 4
  %4 = call i32 (ptr, ...) @printf(ptr @0, i32 %e23)
  ret i32 0

cond:                                             ; preds = %for.body
  %i4 = load i32, ptr %i, align 4
  %5 = icmp eq i32 %i4, 10
  %6 = sext i1 %5 to i32
  %7 = icmp ne i32 %6, 0
  br i1 %7, label %then, label %final

then:                                             ; preds = %cond
  br label %for.inc

final:                                            ; preds = %for.continue.death, %cond
  br label %for.init5

for.continue.death:                               ; No predecessors!
  br label %final

for.init5:                                        ; preds = %final
  %j = alloca i32, align 4
  store i32 0, ptr %j, align 4
  %j10 = load i32, ptr %j, align 4
  br label %for.cond6

for.cond6:                                        ; preds = %for.inc7, %for.init5
  %j11 = load i32, ptr %j, align 4
  %8 = icmp slt i32 %j11, 10
  %9 = sext i1 %8 to i32
  %10 = icmp ne i32 %9, 0
  br i1 %10, label %for.body8, label %for.final9

for.inc7:                                         ; preds = %final17
  %j19 = load i32, ptr %j, align 4
  %11 = add nsw i32 %j19, 1
  store i32 %11, ptr %j, align 4
  %j20 = load i32, ptr %j, align 4
  br label %for.cond6

for.body8:                                        ; preds = %for.cond6
  %e12 = load i32, ptr %e, align 4
  %i13 = load i32, ptr %i, align 4
  %12 = add nsw i32 %e12, %i13
  store i32 %12, ptr %e, align 4
  %e14 = load i32, ptr %e, align 4
  br label %cond15

for.final9:                                       ; preds = %then16, %for.cond6
  br label %for.inc

cond15:                                           ; preds = %for.body8
  %j18 = load i32, ptr %j, align 4
  %13 = icmp sge i32 %j18, 8
  %14 = sext i1 %13 to i32
  %15 = icmp ne i32 %14, 0
  br i1 %15, label %then16, label %final17

then16:                                           ; preds = %cond15
  br label %for.final9

final17:                                          ; preds = %for.break.death, %cond15
  br label %for.inc7

for.break.death:                                  ; No predecessors!
  br label %final17
}
