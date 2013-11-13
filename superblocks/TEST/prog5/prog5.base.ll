; ModuleID = 'prog5/prog5.base.bc'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-f128:128:128-n8:16:32:64"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [10 x i8] c"Sum = %d\0A\00", align 1

define i32 @main() nounwind {
entry:
  %retval = alloca i32
  %0 = alloca i32
  %MAXITER = alloca i32
  %THRESHOLD = alloca i32
  %val = alloca i32
  %idx = alloca i32
  %num = alloca i32
  %sum = alloca i32
  %1 = call i32 (...)* @time(i8* null) nounwind
  call void @srand(i32 %1) nounwind
  store i32 100000000, i32* %MAXITER, align 4
  store i32 100, i32* %THRESHOLD, align 4
  store i32 40, i32* %val, align 4
  store i32 0, i32* %sum, align 4
  store i32 0, i32* %idx, align 4
  br label %bb22

bb:                                               ; preds = %bb22
  %2 = call i32 @rand() nounwind
  %3 = load i32* %THRESHOLD, align 4
  %4 = srem i32 %2, %3
  %5 = sub nsw i32 %3, 10
  %6 = icmp sgt i32 %4, %5
  br i1 %6, label %bb1, label %bb2

bb1:                                              ; preds = %bb
  %7 = load i32* %num, align 4
  %8 = load i32* %val, align 4
  %9 = add nsw i32 %7, %8
  store i32 %9, i32* %num, align 4
  br label %bb3

bb2:                                              ; preds = %bb
  store i32 10, i32* %num, align 4
  br label %bb3

bb3:                                              ; preds = %bb2, %bb1
  %10 = call i32 @rand() nounwind
  %11 = load i32* %THRESHOLD, align 4
  %12 = srem i32 %10, %11
  %13 = sub nsw i32 %11, 10
  %14 = icmp sgt i32 %12, %13
  br i1 %14, label %bb4, label %bb5

bb4:                                              ; preds = %bb3
  %15 = load i32* %num, align 4
  %16 = load i32* %val, align 4
  %17 = add nsw i32 %15, %16
  store i32 %17, i32* %num, align 4
  br label %bb6

bb5:                                              ; preds = %bb3
  store i32 10, i32* %num, align 4
  br label %bb6

bb6:                                              ; preds = %bb5, %bb4
  %18 = call i32 @rand() nounwind
  %19 = load i32* %THRESHOLD, align 4
  %20 = srem i32 %18, %19
  %21 = sub nsw i32 %19, 10
  %22 = icmp sgt i32 %20, %21
  br i1 %22, label %bb7, label %bb8

bb7:                                              ; preds = %bb6
  %23 = load i32* %num, align 4
  %24 = load i32* %val, align 4
  %25 = add nsw i32 %23, %24
  store i32 %25, i32* %num, align 4
  br label %bb9

bb8:                                              ; preds = %bb6
  store i32 10, i32* %num, align 4
  br label %bb9

bb9:                                              ; preds = %bb8, %bb7
  %26 = call i32 @rand() nounwind
  %27 = load i32* %THRESHOLD, align 4
  %28 = srem i32 %26, %27
  %29 = sub nsw i32 %27, 10
  %30 = icmp sgt i32 %28, %29
  br i1 %30, label %bb10, label %bb11

bb10:                                             ; preds = %bb9
  %31 = load i32* %num, align 4
  %32 = load i32* %val, align 4
  %33 = add nsw i32 %31, %32
  store i32 %33, i32* %num, align 4
  br label %bb12

bb11:                                             ; preds = %bb9
  store i32 10, i32* %num, align 4
  br label %bb12

bb12:                                             ; preds = %bb11, %bb10
  %34 = call i32 @rand() nounwind
  %35 = load i32* %THRESHOLD, align 4
  %36 = srem i32 %34, %35
  %37 = sub nsw i32 %35, 10
  %38 = icmp sgt i32 %36, %37
  br i1 %38, label %bb13, label %bb14

bb13:                                             ; preds = %bb12
  %39 = load i32* %num, align 4
  %40 = load i32* %val, align 4
  %41 = add nsw i32 %39, %40
  store i32 %41, i32* %num, align 4
  br label %bb15

bb14:                                             ; preds = %bb12
  store i32 10, i32* %num, align 4
  br label %bb15

bb15:                                             ; preds = %bb14, %bb13
  %42 = call i32 @rand() nounwind
  %43 = load i32* %THRESHOLD, align 4
  %44 = srem i32 %42, %43
  %45 = sub nsw i32 %43, 10
  %46 = icmp sgt i32 %44, %45
  br i1 %46, label %bb16, label %bb17

bb16:                                             ; preds = %bb15
  %47 = load i32* %num, align 4
  %48 = load i32* %val, align 4
  %49 = add nsw i32 %47, %48
  store i32 %49, i32* %num, align 4
  br label %bb18

bb17:                                             ; preds = %bb15
  store i32 10, i32* %num, align 4
  br label %bb18

bb18:                                             ; preds = %bb17, %bb16
  %50 = call i32 @rand() nounwind
  %51 = load i32* %THRESHOLD, align 4
  %52 = srem i32 %50, %51
  %53 = sub nsw i32 %51, 10
  %54 = icmp sgt i32 %52, %53
  br i1 %54, label %bb19, label %bb20

bb19:                                             ; preds = %bb18
  %55 = load i32* %num, align 4
  %56 = load i32* %val, align 4
  %57 = add nsw i32 %55, %56
  store i32 %57, i32* %num, align 4
  br label %bb21

bb20:                                             ; preds = %bb18
  store i32 10, i32* %num, align 4
  br label %bb21

bb21:                                             ; preds = %bb20, %bb19
  %58 = load i32* %num, align 4
  %59 = add nsw i32 %58, %58
  store i32 %59, i32* %val, align 4
  %60 = load i32* %sum, align 4
  %61 = load i32* %idx, align 4
  %62 = add nsw i32 %60, %61
  store i32 %62, i32* %sum, align 4
  %63 = load i32* %idx, align 4
  %64 = add nsw i32 %63, 1
  store i32 %64, i32* %idx, align 4
  br label %bb22

bb22:                                             ; preds = %bb21, %entry
  %65 = load i32* %idx, align 4
  %66 = load i32* %MAXITER, align 4
  %67 = icmp slt i32 %65, %66
  br i1 %67, label %bb, label %bb23

bb23:                                             ; preds = %bb22
  %68 = load i32* %sum, align 4
  %69 = call i32 (i8*, ...)* @printf(i8* noalias getelementptr inbounds ([10 x i8]* @.str, i64 0, i64 0), i32 %68) nounwind
  store i32 0, i32* %0, align 4
  store i32 0, i32* %retval, align 4
  br label %return

return:                                           ; preds = %bb23
  ret i32 0
}

declare i32 @time(...)

declare void @srand(i32) nounwind

declare i32 @rand() nounwind

declare i32 @printf(i8* noalias, ...) nounwind
