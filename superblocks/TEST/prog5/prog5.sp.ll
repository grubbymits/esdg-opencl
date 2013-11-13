; ModuleID = 'prog5/prog5.sp.bc'
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

bb22:                                             ; preds = %bb21.cloned, %bb20, %entry
  %2 = load i32* %idx, align 4
  %3 = load i32* %MAXITER, align 4
  %4 = icmp slt i32 %2, %3
  br i1 %4, label %bb, label %bb23

bb:                                               ; preds = %bb22
  %5 = call i32 @rand() nounwind
  %6 = load i32* %THRESHOLD, align 4
  %7 = srem i32 %5, %6
  %8 = sub nsw i32 %6, 10
  %9 = icmp sgt i32 %7, %8
  br i1 %9, label %bb1, label %bb2

bb2:                                              ; preds = %bb
  store i32 10, i32* %num, align 4
  %10 = call i32 @rand() nounwind
  %11 = load i32* %THRESHOLD, align 4
  %12 = srem i32 %10, %11
  %13 = sub nsw i32 %11, 10
  %14 = icmp sgt i32 %12, %13
  br i1 %14, label %bb4, label %bb5

bb5:                                              ; preds = %bb2
  store i32 10, i32* %num, align 4
  %15 = call i32 @rand() nounwind
  %16 = load i32* %THRESHOLD, align 4
  %17 = srem i32 %15, %16
  %18 = sub nsw i32 %16, 10
  %19 = icmp sgt i32 %17, %18
  br i1 %19, label %bb7, label %bb8

bb8:                                              ; preds = %bb5
  store i32 10, i32* %num, align 4
  %20 = call i32 @rand() nounwind
  %21 = load i32* %THRESHOLD, align 4
  %22 = srem i32 %20, %21
  %23 = sub nsw i32 %21, 10
  %24 = icmp sgt i32 %22, %23
  br i1 %24, label %bb10, label %bb11

bb11:                                             ; preds = %bb8
  store i32 10, i32* %num, align 4
  %25 = call i32 @rand() nounwind
  %26 = load i32* %THRESHOLD, align 4
  %27 = srem i32 %25, %26
  %28 = sub nsw i32 %26, 10
  %29 = icmp sgt i32 %27, %28
  br i1 %29, label %bb13, label %bb14

bb14:                                             ; preds = %bb11
  store i32 10, i32* %num, align 4
  %30 = call i32 @rand() nounwind
  %31 = load i32* %THRESHOLD, align 4
  %32 = srem i32 %30, %31
  %33 = sub nsw i32 %31, 10
  %34 = icmp sgt i32 %32, %33
  br i1 %34, label %bb16, label %bb17

bb17:                                             ; preds = %bb14
  store i32 10, i32* %num, align 4
  %35 = call i32 @rand() nounwind
  %36 = load i32* %THRESHOLD, align 4
  %37 = srem i32 %35, %36
  %38 = sub nsw i32 %36, 10
  %39 = icmp sgt i32 %37, %38
  br i1 %39, label %bb19, label %bb20

bb20:                                             ; preds = %bb17
  store i32 10, i32* %num, align 4
  store i32 20, i32* %val, align 4
  %40 = load i32* %sum, align 4
  %41 = load i32* %idx, align 4
  %42 = add nsw i32 %40, %41
  store i32 %42, i32* %sum, align 4
  %43 = load i32* %idx, align 4
  %44 = add nsw i32 %43, 1
  store i32 %44, i32* %idx, align 4
  br label %bb22

bb23:                                             ; preds = %bb22
  %45 = load i32* %sum, align 4
  %46 = call i32 (i8*, ...)* @printf(i8* noalias getelementptr inbounds ([10 x i8]* @.str, i64 0, i64 0), i32 %45) nounwind
  store i32 0, i32* %0, align 4
  store i32 0, i32* %retval, align 4
  br label %return

return:                                           ; preds = %bb23
  ret i32 0

bb1:                                              ; preds = %bb
  %47 = load i32* %num, align 4
  %48 = load i32* %val, align 4
  %49 = add nsw i32 %47, %48
  store i32 %49, i32* %num, align 4
  %50 = call i32 @rand() nounwind
  %51 = load i32* %THRESHOLD, align 4
  %52 = srem i32 %50, %51
  %53 = sub nsw i32 %51, 10
  %54 = icmp sgt i32 %52, %53
  br i1 %54, label %bb4, label %bb5.cloned

bb5.cloned:                                       ; preds = %bb1
  store i32 10, i32* %num, align 4
  br label %bb6.cloned

bb4:                                              ; preds = %bb1, %bb2
  %55 = load i32* %num, align 4
  %56 = load i32* %val, align 4
  %57 = add nsw i32 %55, %56
  store i32 %57, i32* %num, align 4
  br label %bb6.cloned

bb6.cloned:                                       ; preds = %bb4, %bb5.cloned
  %58 = call i32 @rand() nounwind
  %59 = load i32* %THRESHOLD, align 4
  %60 = srem i32 %58, %59
  %61 = sub nsw i32 %59, 10
  %62 = icmp sgt i32 %60, %61
  br i1 %62, label %bb7, label %bb8.cloned

bb8.cloned:                                       ; preds = %bb6.cloned
  store i32 10, i32* %num, align 4
  br label %bb9.cloned

bb7:                                              ; preds = %bb6.cloned, %bb5
  %63 = load i32* %num, align 4
  %64 = load i32* %val, align 4
  %65 = add nsw i32 %63, %64
  store i32 %65, i32* %num, align 4
  br label %bb9.cloned

bb9.cloned:                                       ; preds = %bb7, %bb8.cloned
  %66 = call i32 @rand() nounwind
  %67 = load i32* %THRESHOLD, align 4
  %68 = srem i32 %66, %67
  %69 = sub nsw i32 %67, 10
  %70 = icmp sgt i32 %68, %69
  br i1 %70, label %bb10, label %bb11.cloned

bb11.cloned:                                      ; preds = %bb9.cloned
  store i32 10, i32* %num, align 4
  br label %bb12.cloned

bb10:                                             ; preds = %bb9.cloned, %bb8
  %71 = load i32* %num, align 4
  %72 = load i32* %val, align 4
  %73 = add nsw i32 %71, %72
  store i32 %73, i32* %num, align 4
  br label %bb12.cloned

bb12.cloned:                                      ; preds = %bb10, %bb11.cloned
  %74 = call i32 @rand() nounwind
  %75 = load i32* %THRESHOLD, align 4
  %76 = srem i32 %74, %75
  %77 = sub nsw i32 %75, 10
  %78 = icmp sgt i32 %76, %77
  br i1 %78, label %bb13, label %bb14.cloned

bb14.cloned:                                      ; preds = %bb12.cloned
  store i32 10, i32* %num, align 4
  br label %bb15.cloned

bb13:                                             ; preds = %bb12.cloned, %bb11
  %79 = load i32* %num, align 4
  %80 = load i32* %val, align 4
  %81 = add nsw i32 %79, %80
  store i32 %81, i32* %num, align 4
  br label %bb15.cloned

bb15.cloned:                                      ; preds = %bb13, %bb14.cloned
  %82 = call i32 @rand() nounwind
  %83 = load i32* %THRESHOLD, align 4
  %84 = srem i32 %82, %83
  %85 = sub nsw i32 %83, 10
  %86 = icmp sgt i32 %84, %85
  br i1 %86, label %bb16, label %bb17.cloned

bb17.cloned:                                      ; preds = %bb15.cloned
  store i32 10, i32* %num, align 4
  br label %bb18.cloned

bb16:                                             ; preds = %bb15.cloned, %bb14
  %87 = load i32* %num, align 4
  %88 = load i32* %val, align 4
  %89 = add nsw i32 %87, %88
  store i32 %89, i32* %num, align 4
  br label %bb18.cloned

bb18.cloned:                                      ; preds = %bb16, %bb17.cloned
  %90 = call i32 @rand() nounwind
  %91 = load i32* %THRESHOLD, align 4
  %92 = srem i32 %90, %91
  %93 = sub nsw i32 %91, 10
  %94 = icmp sgt i32 %92, %93
  br i1 %94, label %bb19, label %bb20.cloned

bb20.cloned:                                      ; preds = %bb18.cloned
  store i32 10, i32* %num, align 4
  br label %bb21.cloned

bb19:                                             ; preds = %bb18.cloned, %bb17
  %95 = load i32* %num, align 4
  %96 = load i32* %val, align 4
  %97 = add nsw i32 %95, %96
  store i32 %97, i32* %num, align 4
  br label %bb21.cloned

bb21.cloned:                                      ; preds = %bb19, %bb20.cloned
  %98 = load i32* %num, align 4
  %99 = add nsw i32 %98, %98
  store i32 %99, i32* %val, align 4
  %100 = load i32* %sum, align 4
  %101 = load i32* %idx, align 4
  %102 = add nsw i32 %100, %101
  store i32 %102, i32* %sum, align 4
  %103 = load i32* %idx, align 4
  %104 = add nsw i32 %103, 1
  store i32 %104, i32* %idx, align 4
  br label %bb22
}

declare i32 @time(...)

declare void @srand(i32) nounwind

declare i32 @rand() nounwind

declare i32 @printf(i8* noalias, ...) nounwind
