; ModuleID = 'math.bc'
target datalayout = "E-p:32:32:32-i1:8:8-i8:8:32-i16:16:32-i32:32:32-i64:32:32-f32:32:32-f64:32:32-v64:32:32-v128:32:32-a0:0:32-n32"
target triple = "le1"

%union.ieee_double_shape_type = type { %struct.anon }
%struct.anon = type { i32, i32 }

define float @__ieee754_acos(float %x) nounwind {
entry:
  %retval = alloca float, align 4
  %x.addr = alloca float, align 4
  %z = alloca float, align 4
  %p = alloca float, align 4
  %q = alloca float, align 4
  %r = alloca float, align 4
  %w = alloca float, align 4
  %s = alloca float, align 4
  %c = alloca float, align 4
  %df = alloca float, align 4
  %hx = alloca i32, align 4
  %ix = alloca i32, align 4
  %gh_u = alloca %union.ieee_double_shape_type, align 4
  %lx = alloca i32, align 4
  %gl_u = alloca %union.ieee_double_shape_type, align 4
  %sl_u = alloca %union.ieee_double_shape_type, align 4
  store float %x, float* %x.addr, align 4
  br label %do.body

do.body:                                          ; preds = %entry
  %0 = load float* %x.addr, align 4
  %value = bitcast %union.ieee_double_shape_type* %gh_u to float*
  store float %0, float* %value, align 4
  %parts = bitcast %union.ieee_double_shape_type* %gh_u to %struct.anon*
  %msw = getelementptr inbounds %struct.anon* %parts, i32 0, i32 0
  %1 = load i32* %msw, align 4
  store i32 %1, i32* %hx, align 4
  br label %do.end

do.end:                                           ; preds = %do.body
  %2 = load i32* %hx, align 4
  %and = and i32 %2, 2147483647
  store i32 %and, i32* %ix, align 4
  %3 = load i32* %ix, align 4
  %cmp = icmp sge i32 %3, 1072693248
  br i1 %cmp, label %if.then, label %if.end11

if.then:                                          ; preds = %do.end
  br label %do.body1

do.body1:                                         ; preds = %if.then
  %4 = load float* %x.addr, align 4
  %value2 = bitcast %union.ieee_double_shape_type* %gl_u to float*
  store float %4, float* %value2, align 4
  %parts3 = bitcast %union.ieee_double_shape_type* %gl_u to %struct.anon*
  %lsw = getelementptr inbounds %struct.anon* %parts3, i32 0, i32 1
  %5 = load i32* %lsw, align 4
  store i32 %5, i32* %lx, align 4
  br label %do.end4

do.end4:                                          ; preds = %do.body1
  %6 = load i32* %ix, align 4
  %sub = sub nsw i32 %6, 1072693248
  %7 = load i32* %lx, align 4
  %or = or i32 %sub, %7
  %cmp5 = icmp eq i32 %or, 0
  br i1 %cmp5, label %if.then6, label %if.end

if.then6:                                         ; preds = %do.end4
  %8 = load i32* %hx, align 4
  %cmp7 = icmp sgt i32 %8, 0
  br i1 %cmp7, label %if.then8, label %if.else

if.then8:                                         ; preds = %if.then6
  store float 0.000000e+00, float* %retval
  br label %return

if.else:                                          ; preds = %if.then6
  store float 0x400921FB60000000, float* %retval
  br label %return

if.end:                                           ; preds = %do.end4
  %9 = load float* %x.addr, align 4
  %10 = load float* %x.addr, align 4
  %sub9 = fsub float %9, %10
  %11 = load float* %x.addr, align 4
  %12 = load float* %x.addr, align 4
  %sub10 = fsub float %11, %12
  %div = fdiv float %sub9, %sub10
  store float %div, float* %retval
  br label %return

if.end11:                                         ; preds = %do.end
  %13 = load i32* %ix, align 4
  %cmp12 = icmp slt i32 %13, 1071644672
  br i1 %cmp12, label %if.then13, label %if.else40

if.then13:                                        ; preds = %if.end11
  %14 = load i32* %ix, align 4
  %cmp14 = icmp sle i32 %14, 1012924416
  br i1 %cmp14, label %if.then15, label %if.end16

if.then15:                                        ; preds = %if.then13
  store float 0x3FF921FB60000000, float* %retval
  br label %return

if.end16:                                         ; preds = %if.then13
  %15 = load float* %x.addr, align 4
  %16 = load float* %x.addr, align 4
  %mul = fmul float %15, %16
  store float %mul, float* %z, align 4
  %17 = load float* %z, align 4
  %18 = load float* %z, align 4
  %19 = load float* %z, align 4
  %20 = load float* %z, align 4
  %21 = load float* %z, align 4
  %22 = load float* %z, align 4
  %mul17 = fmul float %22, 0x3F023DE100000000
  %add = fadd float 0x3F49EFE080000000, %mul17
  %mul18 = fmul float %21, %add
  %add19 = fadd float 0xBFA48228C0000000, %mul18
  %mul20 = fmul float %20, %add19
  %add21 = fadd float 0x3FC9C15500000000, %mul20
  %mul22 = fmul float %19, %add21
  %add23 = fadd float 0xBFD4D61200000000, %mul22
  %mul24 = fmul float %18, %add23
  %add25 = fadd float 0x3FC5555560000000, %mul24
  %mul26 = fmul float %17, %add25
  store float %mul26, float* %p, align 4
  %23 = load float* %z, align 4
  %24 = load float* %z, align 4
  %25 = load float* %z, align 4
  %26 = load float* %z, align 4
  %mul27 = fmul float %26, 0x3FB3B8C5C0000000
  %add28 = fadd float 0xBFE6066C20000000, %mul27
  %mul29 = fmul float %25, %add28
  %add30 = fadd float 0x40002AE5A0000000, %mul29
  %mul31 = fmul float %24, %add30
  %add32 = fadd float 0xC0033A2720000000, %mul31
  %mul33 = fmul float %23, %add32
  %add34 = fadd float 1.000000e+00, %mul33
  store float %add34, float* %q, align 4
  %27 = load float* %p, align 4
  %28 = load float* %q, align 4
  %div35 = fdiv float %27, %28
  store float %div35, float* %r, align 4
  %29 = load float* %x.addr, align 4
  %30 = load float* %x.addr, align 4
  %31 = load float* %r, align 4
  %mul36 = fmul float %30, %31
  %sub37 = fsub float 0x3C91A62640000000, %mul36
  %sub38 = fsub float %29, %sub37
  %sub39 = fsub float 0x3FF921FB60000000, %sub38
  store float %sub39, float* %retval
  br label %return

if.else40:                                        ; preds = %if.end11
  %32 = load i32* %hx, align 4
  %cmp41 = icmp slt i32 %32, 0
  br i1 %cmp41, label %if.then42, label %if.else70

if.then42:                                        ; preds = %if.else40
  %33 = load float* %x.addr, align 4
  %add43 = fadd float 1.000000e+00, %33
  %mul44 = fmul float %add43, 5.000000e-01
  store float %mul44, float* %z, align 4
  %34 = load float* %z, align 4
  %35 = load float* %z, align 4
  %36 = load float* %z, align 4
  %37 = load float* %z, align 4
  %38 = load float* %z, align 4
  %39 = load float* %z, align 4
  %mul45 = fmul float %39, 0x3F023DE100000000
  %add46 = fadd float 0x3F49EFE080000000, %mul45
  %mul47 = fmul float %38, %add46
  %add48 = fadd float 0xBFA48228C0000000, %mul47
  %mul49 = fmul float %37, %add48
  %add50 = fadd float 0x3FC9C15500000000, %mul49
  %mul51 = fmul float %36, %add50
  %add52 = fadd float 0xBFD4D61200000000, %mul51
  %mul53 = fmul float %35, %add52
  %add54 = fadd float 0x3FC5555560000000, %mul53
  %mul55 = fmul float %34, %add54
  store float %mul55, float* %p, align 4
  %40 = load float* %z, align 4
  %41 = load float* %z, align 4
  %42 = load float* %z, align 4
  %43 = load float* %z, align 4
  %mul56 = fmul float %43, 0x3FB3B8C5C0000000
  %add57 = fadd float 0xBFE6066C20000000, %mul56
  %mul58 = fmul float %42, %add57
  %add59 = fadd float 0x40002AE5A0000000, %mul58
  %mul60 = fmul float %41, %add59
  %add61 = fadd float 0xC0033A2720000000, %mul60
  %mul62 = fmul float %40, %add61
  %add63 = fadd float 1.000000e+00, %mul62
  store float %add63, float* %q, align 4
  %44 = load float* %z, align 4
  %call = call float @__ieee754_sqrt(float %44)
  store float %call, float* %s, align 4
  %45 = load float* %p, align 4
  %46 = load float* %q, align 4
  %div64 = fdiv float %45, %46
  store float %div64, float* %r, align 4
  %47 = load float* %r, align 4
  %48 = load float* %s, align 4
  %mul65 = fmul float %47, %48
  %sub66 = fsub float %mul65, 0x3C91A62640000000
  store float %sub66, float* %w, align 4
  %49 = load float* %s, align 4
  %50 = load float* %w, align 4
  %add67 = fadd float %49, %50
  %mul68 = fmul float 2.000000e+00, %add67
  %sub69 = fsub float 0x400921FB60000000, %mul68
  store float %sub69, float* %retval
  br label %return

if.else70:                                        ; preds = %if.else40
  %51 = load float* %x.addr, align 4
  %sub71 = fsub float 1.000000e+00, %51
  %mul72 = fmul float %sub71, 5.000000e-01
  store float %mul72, float* %z, align 4
  %52 = load float* %z, align 4
  %call73 = call float @__ieee754_sqrt(float %52)
  store float %call73, float* %s, align 4
  %53 = load float* %s, align 4
  store float %53, float* %df, align 4
  br label %do.body74

do.body74:                                        ; preds = %if.else70
  %54 = load float* %df, align 4
  %value75 = bitcast %union.ieee_double_shape_type* %sl_u to float*
  store float %54, float* %value75, align 4
  %parts76 = bitcast %union.ieee_double_shape_type* %sl_u to %struct.anon*
  %lsw77 = getelementptr inbounds %struct.anon* %parts76, i32 0, i32 1
  store i32 0, i32* %lsw77, align 4
  %value78 = bitcast %union.ieee_double_shape_type* %sl_u to float*
  %55 = load float* %value78, align 4
  store float %55, float* %df, align 4
  br label %do.end79

do.end79:                                         ; preds = %do.body74
  %56 = load float* %z, align 4
  %57 = load float* %df, align 4
  %58 = load float* %df, align 4
  %mul80 = fmul float %57, %58
  %sub81 = fsub float %56, %mul80
  %59 = load float* %s, align 4
  %60 = load float* %df, align 4
  %add82 = fadd float %59, %60
  %div83 = fdiv float %sub81, %add82
  store float %div83, float* %c, align 4
  %61 = load float* %z, align 4
  %62 = load float* %z, align 4
  %63 = load float* %z, align 4
  %64 = load float* %z, align 4
  %65 = load float* %z, align 4
  %66 = load float* %z, align 4
  %mul84 = fmul float %66, 0x3F023DE100000000
  %add85 = fadd float 0x3F49EFE080000000, %mul84
  %mul86 = fmul float %65, %add85
  %add87 = fadd float 0xBFA48228C0000000, %mul86
  %mul88 = fmul float %64, %add87
  %add89 = fadd float 0x3FC9C15500000000, %mul88
  %mul90 = fmul float %63, %add89
  %add91 = fadd float 0xBFD4D61200000000, %mul90
  %mul92 = fmul float %62, %add91
  %add93 = fadd float 0x3FC5555560000000, %mul92
  %mul94 = fmul float %61, %add93
  store float %mul94, float* %p, align 4
  %67 = load float* %z, align 4
  %68 = load float* %z, align 4
  %69 = load float* %z, align 4
  %70 = load float* %z, align 4
  %mul95 = fmul float %70, 0x3FB3B8C5C0000000
  %add96 = fadd float 0xBFE6066C20000000, %mul95
  %mul97 = fmul float %69, %add96
  %add98 = fadd float 0x40002AE5A0000000, %mul97
  %mul99 = fmul float %68, %add98
  %add100 = fadd float 0xC0033A2720000000, %mul99
  %mul101 = fmul float %67, %add100
  %add102 = fadd float 1.000000e+00, %mul101
  store float %add102, float* %q, align 4
  %71 = load float* %p, align 4
  %72 = load float* %q, align 4
  %div103 = fdiv float %71, %72
  store float %div103, float* %r, align 4
  %73 = load float* %r, align 4
  %74 = load float* %s, align 4
  %mul104 = fmul float %73, %74
  %75 = load float* %c, align 4
  %add105 = fadd float %mul104, %75
  store float %add105, float* %w, align 4
  %76 = load float* %df, align 4
  %77 = load float* %w, align 4
  %add106 = fadd float %76, %77
  %mul107 = fmul float 2.000000e+00, %add106
  store float %mul107, float* %retval
  br label %return

return:                                           ; preds = %do.end79, %if.then42, %if.end16, %if.then15, %if.end, %if.else, %if.then8
  %78 = load float* %retval
  ret float %78
}

declare float @__ieee754_sqrt(float)
