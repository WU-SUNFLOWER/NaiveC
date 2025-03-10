; ModuleID = 'Expr Module'
source_filename = "Expr Module"

declare i32 @printf(ptr, ...)

define i32 @main() {
entry:
  %my_union_1 = alloca { ptr }, align 8
  %my_union_2 = alloca { ptr }, align 8
  %my_union_3 = alloca { ptr }, align 8

  %0 = getelementptr inbounds { ptr }, ptr %my_union_1, i32 0, i32 0
  store i32 1, ptr %0, align 4
  
  %1 = getelementptr inbounds { ptr }, ptr %my_union_2, i32 0, i32 0
  store i32 2, ptr %1, align 4
  
  %2 = getelementptr inbounds { ptr }, ptr %my_union_3, i32 0, i32 0
  store i32 3, ptr %2, align 4
  
  ret i32 3
}