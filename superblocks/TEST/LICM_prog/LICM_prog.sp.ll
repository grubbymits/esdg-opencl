; ModuleID = 'LICM_prog.sp.bc'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-f128:128:128-n8:16:32:64"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [7 x i8] c"x = %d\00", align 1

define i32 @main() nounwind {
entry:
  %retval = alloca i32
  %0 = alloca i32
  %i = alloca i32
  %step = alloca i32
  %x = alloca i32
  %"alloca point" = bitcast i32 0 to i32
  store i32 0, i32* %i, align 4
  store i32 0, i32* %step, align 4
  store i32 0, i32* %x, align 4
  store i32 0, i32* %i, align 4
  br label %bb3

bb3:                                              ; preds = %bb1, %bb2, %entry
  %1 = load i32* %i, align 4
  %2 = icmp sle i32 %1, 99999999
  br i1 %2, label %bb, label %bb4

bb:                                               ; preds = %bb3
  %3 = load i32* %i, align 4
  %4 = and i32 %3, 255
  %5 = icmp eq i32 %4, 0
  br i1 %5, label %bb1, label %bb2

bb2:                                              ; preds = %bb
  %6 = load i32* %step, align 4
  %7 = sub nsw i32 %6, 1
  %8 = shl i32 %7, 2
  store i32 %8, i32* %x, align 4
  %9 = load i32* %i, align 4
  %10 = add nsw i32 %9, 1
  store i32 %10, i32* %i, align 4
  br label %bb3

bb4:                                              ; preds = %bb3
  %11 = load i32* %x, align 4
  %12 = call i32 (i8*, ...)* @printf(i8* noalias getelementptr inbounds ([7 x i8]* @.str, i64 0, i64 0), i32 %11) nounwind
  store i32 0, i32* %0, align 4
  %13 = load i32* %0, align 4
  store i32 %13, i32* %retval, align 4
  br label %return

return:                                           ; preds = %bb4
  %retval5 = load i32* %retval
  ret i32 %retval5

bb1:                                              ; preds = %bb
  %14 = load i32* %i, align 4
  store i32 %14, i32* %step, align 4
  %15 = load i32* %step, align 4
  %16 = sub nsw i32 %15, 1
  %17 = shl i32 %16, 2
  store i32 %17, i32* %x, align 4
  %18 = load i32* %i, align 4
  %19 = add nsw i32 %18, 1
  store i32 %19, i32* %i, align 4
  br label %bb3
}

declare i32 @printf(i8* noalias, ...) nounwind
