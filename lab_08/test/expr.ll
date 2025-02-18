; ModuleID = 'Expr Module'
source_filename = "Expr Module"

@0 = private unnamed_addr constant [34 x i8] c"last result isn't an expression!\0A\00", align 1

declare i32 @printf(ptr, ...)

define i32 @main() {
entry:
  %a = alloca i32, align 4
  %b = alloca i32, align 4
  %c = alloca i32, align 4
  %d = alloca i32, align 4
  %e = alloca i32, align 4
  %f = alloca i32, align 4
  %x = alloca i32, align 4
  %a1 = load i32, ptr %a, align 4
  %0 = icmp ne i32 %a1, 10
  %1 = sext i1 %0 to i32
  %2 = icmp ne i32 %1, 0
  br i1 %2, label %next_block, label %false_block

next_block:                                       ; preds = %entry
  %a2 = load i32, ptr %a, align 4
  %3 = icmp eq i32 %a2, 10
  %4 = sext i1 %3 to i32
  %5 = icmp ne i32 %4, 0
  %6 = zext i1 %5 to i32
  br label %merge_block

false_block:                                      ; preds = %entry
  br label %merge_block

merge_block:                                      ; preds = %false_block, %next_block
  %7 = phi i32 [ 0, %false_block ], [ %6, %next_block ]
  %8 = icmp ne i32 %7, 0
  br i1 %8, label %true_block, label %next_block3

next_block3:                                      ; preds = %merge_block
  %b5 = load i32, ptr %b, align 4
  %9 = icmp ne i32 %b5, 20
  %10 = sext i1 %9 to i32
  %11 = icmp ne i32 %10, 0
  br i1 %11, label %next_block6, label %false_block7

true_block:                                       ; preds = %merge_block
  br label %merge_block4

merge_block4:                                     ; preds = %true_block, %merge_block8
  %12 = phi i32 [ 1, %true_block ], [ %20, %merge_block8 ]
  %13 = icmp ne i32 %12, 0
  br i1 %13, label %true_block11, label %next_block10

next_block6:                                      ; preds = %next_block3
  %c9 = load i32, ptr %c, align 4
  %14 = icmp sle i32 %c9, 10
  %15 = sext i1 %14 to i32
  %16 = icmp ne i32 %15, 0
  %17 = zext i1 %16 to i32
  br label %merge_block8

false_block7:                                     ; preds = %next_block3
  br label %merge_block8

merge_block8:                                     ; preds = %false_block7, %next_block6
  %18 = phi i32 [ 0, %false_block7 ], [ %17, %next_block6 ]
  %19 = icmp ne i32 %18, 0
  %20 = zext i1 %19 to i32
  br label %merge_block4

next_block10:                                     ; preds = %merge_block4
  %c13 = load i32, ptr %c, align 4
  %21 = icmp slt i32 %c13, 10
  %22 = sext i1 %21 to i32
  %23 = icmp ne i32 %22, 0
  br i1 %23, label %next_block14, label %false_block15

true_block11:                                     ; preds = %merge_block4
  br label %merge_block12

merge_block12:                                    ; preds = %true_block11, %merge_block16
  %24 = phi i32 [ 1, %true_block11 ], [ %34, %merge_block16 ]
  %y = alloca i32, align 4
  %a18 = load i32, ptr %a, align 4
  %25 = icmp eq i32 %a18, 10
  %26 = sext i1 %25 to i32
  %27 = icmp ne i32 %26, 0
  br i1 %27, label %next_block19, label %false_block20

next_block14:                                     ; preds = %next_block10
  %b17 = load i32, ptr %b, align 4
  %28 = icmp eq i32 %b17, 20
  %29 = sext i1 %28 to i32
  %30 = icmp ne i32 %29, 0
  %31 = zext i1 %30 to i32
  br label %merge_block16

false_block15:                                    ; preds = %next_block10
  br label %merge_block16

merge_block16:                                    ; preds = %false_block15, %next_block14
  %32 = phi i32 [ 0, %false_block15 ], [ %31, %next_block14 ]
  %33 = icmp ne i32 %32, 0
  %34 = zext i1 %33 to i32
  br label %merge_block12

next_block19:                                     ; preds = %merge_block12
  %b22 = load i32, ptr %b, align 4
  %35 = icmp eq i32 %b22, 20
  %36 = sext i1 %35 to i32
  %37 = icmp ne i32 %36, 0
  %38 = zext i1 %37 to i32
  br label %merge_block21

false_block20:                                    ; preds = %merge_block12
  br label %merge_block21

merge_block21:                                    ; preds = %false_block20, %next_block19
  %39 = phi i32 [ 0, %false_block20 ], [ %38, %next_block19 ]
  %40 = icmp ne i32 %39, 0
  br i1 %40, label %next_block23, label %false_block24

next_block23:                                     ; preds = %merge_block21
  %c26 = load i32, ptr %c, align 4
  %41 = icmp sle i32 %c26, 10
  %42 = sext i1 %41 to i32
  %43 = icmp ne i32 %42, 0
  %44 = zext i1 %43 to i32
  br label %merge_block25

false_block24:                                    ; preds = %merge_block21
  br label %merge_block25

merge_block25:                                    ; preds = %false_block24, %next_block23
  %45 = phi i32 [ 0, %false_block24 ], [ %44, %next_block23 ]
  %46 = icmp ne i32 %45, 0
  br i1 %46, label %true_block28, label %next_block27

next_block27:                                     ; preds = %merge_block25
  %a30 = load i32, ptr %a, align 4
  %47 = icmp ne i32 %a30, 10
  %48 = sext i1 %47 to i32
  %49 = icmp ne i32 %48, 0
  %50 = zext i1 %49 to i32
  br label %merge_block29

true_block28:                                     ; preds = %merge_block25
  br label %merge_block29

merge_block29:                                    ; preds = %true_block28, %next_block27
  %51 = phi i32 [ 1, %true_block28 ], [ %50, %next_block27 ]
  %52 = icmp ne i32 %51, 0
  br i1 %52, label %true_block32, label %next_block31

next_block31:                                     ; preds = %merge_block29
  %b34 = load i32, ptr %b, align 4
  %53 = icmp ne i32 %b34, 20
  %54 = sext i1 %53 to i32
  %55 = icmp ne i32 %54, 0
  %56 = zext i1 %55 to i32
  br label %merge_block33

true_block32:                                     ; preds = %merge_block29
  br label %merge_block33

merge_block33:                                    ; preds = %true_block32, %next_block31
  %57 = phi i32 [ 1, %true_block32 ], [ %56, %next_block31 ]
  %58 = icmp ne i32 %57, 0
  br i1 %58, label %true_block36, label %next_block35

next_block35:                                     ; preds = %merge_block33
  %c38 = load i32, ptr %c, align 4
  %59 = icmp slt i32 %c38, 10
  %60 = sext i1 %59 to i32
  %61 = icmp ne i32 %60, 0
  %62 = zext i1 %61 to i32
  br label %merge_block37

true_block36:                                     ; preds = %merge_block33
  br label %merge_block37

merge_block37:                                    ; preds = %true_block36, %next_block35
  %63 = phi i32 [ 1, %true_block36 ], [ %62, %next_block35 ]
  %d39 = load i32, ptr %d, align 4
  %d40 = load i32, ptr %d, align 4
  %64 = shl i32 %d40, 2
  %d41 = load i32, ptr %d, align 4
  %d42 = load i32, ptr %d, align 4
  %65 = ashr i32 %d42, 5
  %d43 = load i32, ptr %d, align 4
  %d44 = load i32, ptr %d, align 4
  %e45 = load i32, ptr %e, align 4
  %66 = and i32 %d44, %e45
  %d46 = load i32, ptr %d, align 4
  %d47 = load i32, ptr %d, align 4
  %f48 = load i32, ptr %f, align 4
  %67 = xor i32 %d47, %f48
  %d49 = load i32, ptr %d, align 4
  %d50 = load i32, ptr %d, align 4
  %68 = srem i32 %d50, 10
  %d51 = load i32, ptr %d, align 4
  %d52 = load i32, ptr %d, align 4
  %b53 = load i32, ptr %b, align 4
  %69 = or i32 %d52, %b53
  %d54 = load i32, ptr %d, align 4
  %d55 = load i32, ptr %d, align 4
  %x56 = load i32, ptr %x, align 4
  %70 = add nsw i32 %d55, %x56
  %y57 = load i32, ptr %y, align 4
  %71 = add nsw i32 %70, %y57
  %72 = call i32 (ptr, ...) @printf(ptr @0)
  ret i32 0
}
