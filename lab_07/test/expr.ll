; ModuleID = 'Expr Module'
source_filename = "Expr Module"

@0 = private unnamed_addr constant [14 x i8] c"expr val: %d\0A\00", align 1

declare i32 @printf(ptr, ...)

define i32 @main() {
entry:
  %a = alloca i32, align 4
  store i32 10, ptr %a, align 4
  %a1 = load i32, ptr %a, align 4
  %b = alloca i32, align 4
  store i32 20, ptr %b, align 4
  %b2 = load i32, ptr %b, align 4
  %c = alloca i32, align 4
  store i32 10, ptr %c, align 4
  %c3 = load i32, ptr %c, align 4
  %d = alloca i32, align 4
  store i32 5, ptr %d, align 4
  %d4 = load i32, ptr %d, align 4
  %e = alloca i32, align 4
  store i32 2, ptr %e, align 4
  %e5 = load i32, ptr %e, align 4
  %f = alloca i32, align 4
  store i32 3, ptr %f, align 4
  %f6 = load i32, ptr %f, align 4
  %x = alloca i32, align 4
  %a7 = load i32, ptr %a, align 4
  %0 = icmp ne i32 %a7, 10
  %1 = sext i1 %0 to i32
  %2 = icmp ne i32 %1, 0
  br i1 %2, label %next_block, label %false_block

next_block:                                       ; preds = %entry
  %a8 = load i32, ptr %a, align 4
  %3 = icmp eq i32 %a8, 10
  %4 = sext i1 %3 to i32
  %5 = icmp ne i32 %4, 0
  %6 = zext i1 %5 to i32
  br label %merge_block

false_block:                                      ; preds = %entry
  br label %merge_block

merge_block:                                      ; preds = %false_block, %next_block
  %7 = phi i32 [ 0, %false_block ], [ %6, %next_block ]
  %8 = icmp ne i32 %7, 0
  br i1 %8, label %true_block, label %next_block9

next_block9:                                      ; preds = %merge_block
  %b11 = load i32, ptr %b, align 4
  %9 = icmp ne i32 %b11, 20
  %10 = sext i1 %9 to i32
  %11 = icmp ne i32 %10, 0
  br i1 %11, label %next_block12, label %false_block13

true_block:                                       ; preds = %merge_block
  br label %merge_block10

merge_block10:                                    ; preds = %true_block, %merge_block14
  %12 = phi i32 [ 1, %true_block ], [ %20, %merge_block14 ]
  %13 = icmp ne i32 %12, 0
  br i1 %13, label %true_block17, label %next_block16

next_block12:                                     ; preds = %next_block9
  %c15 = load i32, ptr %c, align 4
  %14 = icmp sle i32 %c15, 10
  %15 = sext i1 %14 to i32
  %16 = icmp ne i32 %15, 0
  %17 = zext i1 %16 to i32
  br label %merge_block14

false_block13:                                    ; preds = %next_block9
  br label %merge_block14

merge_block14:                                    ; preds = %false_block13, %next_block12
  %18 = phi i32 [ 0, %false_block13 ], [ %17, %next_block12 ]
  %19 = icmp ne i32 %18, 0
  %20 = zext i1 %19 to i32
  br label %merge_block10

next_block16:                                     ; preds = %merge_block10
  %c19 = load i32, ptr %c, align 4
  %21 = icmp slt i32 %c19, 10
  %22 = sext i1 %21 to i32
  %23 = icmp ne i32 %22, 0
  br i1 %23, label %next_block20, label %false_block21

true_block17:                                     ; preds = %merge_block10
  br label %merge_block18

merge_block18:                                    ; preds = %true_block17, %merge_block22
  %24 = phi i32 [ 1, %true_block17 ], [ %34, %merge_block22 ]
  store i32 %24, ptr %x, align 4
  %x24 = load i32, ptr %x, align 4
  %y = alloca i32, align 4
  %a25 = load i32, ptr %a, align 4
  %25 = icmp eq i32 %a25, 10
  %26 = sext i1 %25 to i32
  %27 = icmp ne i32 %26, 0
  br i1 %27, label %next_block26, label %false_block27

next_block20:                                     ; preds = %next_block16
  %b23 = load i32, ptr %b, align 4
  %28 = icmp eq i32 %b23, 20
  %29 = sext i1 %28 to i32
  %30 = icmp ne i32 %29, 0
  %31 = zext i1 %30 to i32
  br label %merge_block22

false_block21:                                    ; preds = %next_block16
  br label %merge_block22

merge_block22:                                    ; preds = %false_block21, %next_block20
  %32 = phi i32 [ 0, %false_block21 ], [ %31, %next_block20 ]
  %33 = icmp ne i32 %32, 0
  %34 = zext i1 %33 to i32
  br label %merge_block18

next_block26:                                     ; preds = %merge_block18
  %b29 = load i32, ptr %b, align 4
  %35 = icmp eq i32 %b29, 20
  %36 = sext i1 %35 to i32
  %37 = icmp ne i32 %36, 0
  %38 = zext i1 %37 to i32
  br label %merge_block28

false_block27:                                    ; preds = %merge_block18
  br label %merge_block28

merge_block28:                                    ; preds = %false_block27, %next_block26
  %39 = phi i32 [ 0, %false_block27 ], [ %38, %next_block26 ]
  %40 = icmp ne i32 %39, 0
  br i1 %40, label %next_block30, label %false_block31

next_block30:                                     ; preds = %merge_block28
  %c33 = load i32, ptr %c, align 4
  %41 = icmp sle i32 %c33, 10
  %42 = sext i1 %41 to i32
  %43 = icmp ne i32 %42, 0
  %44 = zext i1 %43 to i32
  br label %merge_block32

false_block31:                                    ; preds = %merge_block28
  br label %merge_block32

merge_block32:                                    ; preds = %false_block31, %next_block30
  %45 = phi i32 [ 0, %false_block31 ], [ %44, %next_block30 ]
  %46 = icmp ne i32 %45, 0
  br i1 %46, label %true_block35, label %next_block34

next_block34:                                     ; preds = %merge_block32
  %a37 = load i32, ptr %a, align 4
  %47 = icmp ne i32 %a37, 10
  %48 = sext i1 %47 to i32
  %49 = icmp ne i32 %48, 0
  %50 = zext i1 %49 to i32
  br label %merge_block36

true_block35:                                     ; preds = %merge_block32
  br label %merge_block36

merge_block36:                                    ; preds = %true_block35, %next_block34
  %51 = phi i32 [ 1, %true_block35 ], [ %50, %next_block34 ]
  %52 = icmp ne i32 %51, 0
  br i1 %52, label %true_block39, label %next_block38

next_block38:                                     ; preds = %merge_block36
  %b41 = load i32, ptr %b, align 4
  %53 = icmp ne i32 %b41, 20
  %54 = sext i1 %53 to i32
  %55 = icmp ne i32 %54, 0
  %56 = zext i1 %55 to i32
  br label %merge_block40

true_block39:                                     ; preds = %merge_block36
  br label %merge_block40

merge_block40:                                    ; preds = %true_block39, %next_block38
  %57 = phi i32 [ 1, %true_block39 ], [ %56, %next_block38 ]
  %58 = icmp ne i32 %57, 0
  br i1 %58, label %true_block43, label %next_block42

next_block42:                                     ; preds = %merge_block40
  %c45 = load i32, ptr %c, align 4
  %59 = icmp slt i32 %c45, 10
  %60 = sext i1 %59 to i32
  %61 = icmp ne i32 %60, 0
  %62 = zext i1 %61 to i32
  br label %merge_block44

true_block43:                                     ; preds = %merge_block40
  br label %merge_block44

merge_block44:                                    ; preds = %true_block43, %next_block42
  %63 = phi i32 [ 1, %true_block43 ], [ %62, %next_block42 ]
  store i32 %63, ptr %y, align 4
  %y46 = load i32, ptr %y, align 4
  %d47 = load i32, ptr %d, align 4
  %64 = shl i32 %d47, 2
  store i32 %64, ptr %d, align 4
  %d48 = load i32, ptr %d, align 4
  %d49 = load i32, ptr %d, align 4
  %65 = ashr i32 %d49, 5
  store i32 %65, ptr %d, align 4
  %d50 = load i32, ptr %d, align 4
  %d51 = load i32, ptr %d, align 4
  %e52 = load i32, ptr %e, align 4
  %66 = and i32 %d51, %e52
  store i32 %66, ptr %d, align 4
  %d53 = load i32, ptr %d, align 4
  %d54 = load i32, ptr %d, align 4
  %f55 = load i32, ptr %f, align 4
  %67 = xor i32 %d54, %f55
  store i32 %67, ptr %d, align 4
  %d56 = load i32, ptr %d, align 4
  %d57 = load i32, ptr %d, align 4
  %68 = srem i32 %d57, 10
  store i32 %68, ptr %d, align 4
  %d58 = load i32, ptr %d, align 4
  %d59 = load i32, ptr %d, align 4
  %b60 = load i32, ptr %b, align 4
  %69 = or i32 %d59, %b60
  store i32 %69, ptr %d, align 4
  %d61 = load i32, ptr %d, align 4
  %d62 = load i32, ptr %d, align 4
  %x63 = load i32, ptr %x, align 4
  %70 = add nsw i32 %d62, %x63
  %y64 = load i32, ptr %y, align 4
  %71 = add nsw i32 %70, %y64
  store i32 %71, ptr %d, align 4
  %d65 = load i32, ptr %d, align 4
  %72 = call i32 (ptr, ...) @printf(ptr @0, i32 %d65)
  ret i32 0
}
