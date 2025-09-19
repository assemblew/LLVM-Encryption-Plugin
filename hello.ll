; ModuleID = '/Users/assemblew/Downloads/llvm-project-llvmorg-20.1.2/plugins/hello.cpp'
source_filename = "/Users/assemblew/Downloads/llvm-project-llvmorg-20.1.2/plugins/hello.cpp"
target datalayout = "e-m:o-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx15.0.0"

@_ZZ2aaiE8int32_ex = internal global i32 -1752969839, align 4
@.str = private unnamed_addr constant [15 x i8] c"%i %.2f %i %i\0A\00", align 1
@_ZL8float_ex = internal global float 0xBC416BF060000000, align 4
@_ZL3arr = internal global [2 x i32] [i32 4334234, i32 23], align 4
@.str.1 = private unnamed_addr constant [8 x i8] c"gooon !\00", align 1
@.str.2 = private unnamed_addr constant [10 x i8] c"olegovich\00", align 1
@_ZZ4mainE4cnst = internal constant i32 1403768209, align 4
@.str.3 = private unnamed_addr constant [19 x i8] c"xd %s    %i    %s\0A\00", align 1
@.str.4 = private unnamed_addr constant [7 x i8] c"%i %i\0A\00", align 1
@.str.5 = private unnamed_addr constant [10 x i8] c"%i %i %p\0A\00", align 1
@.str.6 = private unnamed_addr constant [4 x i8] c"%p\0A\00", align 1

; Function Attrs: mustprogress noinline optnone ssp uwtable
define noundef i32 @_Z2aai(i32 noundef %0) #0 {
  %2 = alloca i32, align 4
  store i32 %0, ptr %2, align 4
  %3 = load i32, ptr @_ZZ2aaiE8int32_ex, align 4
  %4 = alloca i32, align 4
  store i32 %3, ptr %4, align 4
  call void @decrypt(ptr %4, i32 4)
  %5 = load i32, ptr %4, align 4
  %6 = xor i32 %5, 1758295668
  %7 = load float, ptr @_ZL8float_ex, align 4
  %8 = bitcast float %7 to i32
  %9 = alloca i32, align 4
  store i32 %8, ptr %9, align 4
  call void @decrypt(ptr %9, i32 4)
  %10 = load i32, ptr %9, align 4
  %11 = bitcast i32 %10 to float
  %12 = bitcast float %11 to i32
  %13 = xor i32 %12, 1758295668
  %14 = bitcast i32 %13 to float
  %15 = fpext float %14 to double
  %16 = load i32, ptr @_ZL3arr, align 4
  %17 = load i32, ptr getelementptr inbounds ([2 x i32], ptr @_ZL3arr, i64 0, i64 1), align 4
  %18 = call i32 (ptr, ...) @printf(ptr noundef @.str, i32 noundef %6, double noundef %15, i32 noundef %16, i32 noundef %17)
  %19 = load i32, ptr %2, align 4
  ret i32 %19
}

declare i32 @printf(ptr noundef, ...) #1

; Function Attrs: mustprogress noinline norecurse optnone ssp uwtable
define noundef i32 @main() #2 {
  %1 = alloca i32, align 4
  %2 = alloca i32, align 4
  %3 = alloca ptr, align 8
  %4 = alloca ptr, align 8
  %5 = alloca i32, align 4
  %6 = alloca ptr, align 8
  store i32 0, ptr %1, align 4
  store i32 0, ptr %2, align 4
  store ptr @.str.1, ptr %3, align 8
  store ptr @.str.2, ptr %4, align 8
  %7 = load ptr, ptr %3, align 8
  %8 = load ptr, ptr %4, align 8
  %9 = call i32 (ptr, ...) @printf(ptr noundef @.str.3, ptr noundef %7, i32 noundef 69, ptr noundef %8)
  store i32 0, ptr %5, align 4
  br label %10

10:                                               ; preds = %16, %0
  %11 = load i32, ptr %5, align 4
  %12 = icmp slt i32 %11, 10
  br i1 %12, label %13, label %21

13:                                               ; preds = %10
  %14 = load i32, ptr %5, align 4
  %15 = call i32 (ptr, ...) @printf(ptr noundef @.str.4, i32 noundef 100, i32 noundef %14)
  br label %16

16:                                               ; preds = %13
  %17 = load i32, ptr %5, align 4
  %18 = add nsw i32 %17, 1
  store i32 %18, ptr %5, align 4
  %19 = load i32, ptr %2, align 4
  %20 = add nsw i32 %19, 1
  store i32 %20, ptr %2, align 4
  br label %10, !llvm.loop !6

21:                                               ; preds = %10
  %22 = load i32, ptr %2, align 4
  %23 = call noundef i32 @_Z2aai(i32 noundef 52)
  %24 = load ptr, ptr %6, align 8
  %25 = call i32 (ptr, ...) @printf(ptr noundef @.str.5, i32 noundef %22, i32 noundef %23, ptr noundef %24)
  store ptr null, ptr %6, align 8
  %26 = load ptr, ptr %6, align 8
  %27 = call i32 (ptr, ...) @printf(ptr noundef @.str.6, ptr noundef %26)
  ret i32 0
}

define private void @decrypt(ptr %0, i64 %1) {
  %3 = alloca i32, align 4
  store i32 1758295668, ptr %3, align 4
  %4 = alloca ptr, align 8
  store ptr %3, ptr %4, align 8
  %5 = alloca [6 x i8], align 1
  store [6 x i8] c"\22\80\DE\A4\B2\EF", ptr %5, align 1
  %6 = load ptr, ptr %4, align 8
  %7 = alloca i8, align 1
  store i8 2, ptr %7, align 1
  br label %8

8:                                                ; preds = %59, %2
  %9 = load i8, ptr %7, align 1
  %10 = icmp sge i8 %9, 0
  br i1 %10, label %11, label %61

11:                                               ; preds = %8
  %12 = alloca i64, align 8
  store i64 0, ptr %12, align 8
  br label %13

13:                                               ; preds = %56, %11
  %14 = load i64, ptr %12, align 8
  %15 = icmp ult i64 %14, %1
  br i1 %15, label %16, label %58

16:                                               ; preds = %13
  %17 = getelementptr inbounds i8, ptr %0, i64 %14
  %18 = load i8, ptr %17, align 1
  %19 = zext i8 %18 to i32
  %20 = load i32, ptr %3, align 4
  %21 = urem i64 %14, 4
  %22 = mul i64 %21, 8
  %23 = trunc i64 %22 to i32
  %24 = lshr i32 %20, %23
  %25 = trunc i32 %24 to i8
  %26 = zext i8 %25 to i32
  %27 = xor i32 %19, %26
  %28 = trunc i32 %27 to i8
  store i8 %28, ptr %17, align 1
  %29 = getelementptr inbounds i8, ptr %0, i64 %14
  %30 = load i8, ptr %29, align 1
  %31 = zext i8 %30 to i32
  %32 = ashr i32 %31, 1
  %33 = shl i32 %31, 7
  %34 = or i32 %32, %33
  %35 = trunc i32 %34 to i8
  store i8 %35, ptr %29, align 1
  %36 = getelementptr inbounds i8, ptr %0, i64 %14
  %37 = load i8, ptr %36, align 1
  %38 = zext i8 %37 to i32
  %39 = urem i64 %14, 6
  %40 = getelementptr inbounds [6 x i8], ptr %5, i64 0, i64 %39
  %41 = load i8, ptr %40, align 1
  %42 = zext i8 %41 to i32
  %43 = xor i32 %38, %42
  %44 = trunc i32 %43 to i8
  store i8 %44, ptr %36, align 1
  %45 = getelementptr inbounds i8, ptr %0, i64 %14
  %46 = load i8, ptr %45, align 1
  %47 = zext i8 %46 to i32
  %48 = sext i8 %9 to i64
  %49 = add i64 %14, %48
  %50 = urem i64 %49, 4
  %51 = getelementptr inbounds i8, ptr %6, i64 %50
  %52 = load i8, ptr %51, align 1
  %53 = zext i8 %52 to i32
  %54 = xor i32 %47, %53
  %55 = trunc i32 %54 to i8
  store i8 %55, ptr %45, align 1
  br label %56

56:                                               ; preds = %16
  %57 = add i64 %14, 1
  store i64 %57, ptr %12, align 8
  br label %13

58:                                               ; preds = %13
  br label %59

59:                                               ; preds = %58
  %60 = add nsw i8 %9, -1
  store i8 %60, ptr %7, align 1
  br label %8

61:                                               ; preds = %8
  %62 = alloca i64, align 8
  store i64 0, ptr %62, align 8
  br label %63

63:                                               ; preds = %114, %61
  %64 = load i64, ptr %62, align 8
  %65 = udiv i64 %1, 2
  %66 = icmp ult i64 %64, %65
  br i1 %66, label %67, label %116

67:                                               ; preds = %63
  %68 = alloca i64, align 8
  %69 = sub i64 %1, %64
  %70 = sub i64 %69, 1
  store i64 %70, ptr %68, align 8
  %71 = getelementptr inbounds i8, ptr %0, i64 %64
  %72 = load i8, ptr %71, align 1
  %73 = zext i8 %72 to i32
  %74 = urem i64 %64, 4
  %75 = getelementptr inbounds i8, ptr %6, i64 %74
  %76 = load i8, ptr %75, align 1
  %77 = zext i8 %76 to i32
  %78 = xor i32 %73, %77
  %79 = trunc i32 %78 to i8
  store i8 %79, ptr %71, align 1
  %80 = load i64, ptr %68, align 8
  %81 = getelementptr inbounds i8, ptr %0, i64 %80
  %82 = load i8, ptr %81, align 1
  %83 = zext i8 %82 to i32
  %84 = urem i64 %80, 4
  %85 = getelementptr inbounds i8, ptr %6, i64 %84
  %86 = load i8, ptr %85, align 1
  %87 = zext i8 %86 to i32
  %88 = xor i32 %83, %87
  %89 = trunc i32 %88 to i8
  store i8 %89, ptr %81, align 1
  %90 = getelementptr inbounds i8, ptr %0, i64 %64
  %91 = load i8, ptr %90, align 1
  %92 = zext i8 %91 to i32
  %93 = getelementptr inbounds i8, ptr %0, i64 %80
  %94 = load i8, ptr %93, align 1
  %95 = zext i8 %94 to i32
  %96 = xor i32 %92, %95
  %97 = trunc i32 %96 to i8
  store i8 %97, ptr %90, align 1
  %98 = getelementptr inbounds i8, ptr %0, i64 %80
  %99 = load i8, ptr %98, align 1
  %100 = zext i8 %99 to i32
  %101 = getelementptr inbounds i8, ptr %0, i64 %64
  %102 = load i8, ptr %101, align 1
  %103 = zext i8 %102 to i32
  %104 = xor i32 %100, %103
  %105 = trunc i32 %104 to i8
  store i8 %105, ptr %98, align 1
  %106 = getelementptr inbounds i8, ptr %0, i64 %64
  %107 = load i8, ptr %106, align 1
  %108 = zext i8 %107 to i32
  %109 = getelementptr inbounds i8, ptr %0, i64 %80
  %110 = load i8, ptr %109, align 1
  %111 = zext i8 %110 to i32
  %112 = xor i32 %108, %111
  %113 = trunc i32 %112 to i8
  store i8 %113, ptr %106, align 1
  br label %114

114:                                              ; preds = %67
  %115 = add i64 %64, 1
  store i64 %115, ptr %62, align 8
  br label %63

116:                                              ; preds = %63
  %117 = urem i64 %1, 2
  %118 = icmp ne i64 %117, 0
  br i1 %118, label %119, label %132

119:                                              ; preds = %116
  %120 = alloca i64, align 8
  %121 = udiv i64 %1, 2
  store i64 %121, ptr %120, align 8
  %122 = load i64, ptr %120, align 8
  %123 = getelementptr inbounds i8, ptr %0, i64 %122
  %124 = load i8, ptr %123, align 1
  %125 = zext i8 %124 to i32
  %126 = urem i64 %122, 4
  %127 = getelementptr inbounds i8, ptr %6, i64 %126
  %128 = load i8, ptr %127, align 1
  %129 = zext i8 %128 to i32
  %130 = xor i32 %125, %129
  %131 = trunc i32 %130 to i8
  store i8 %131, ptr %123, align 1
  br label %132

132:                                              ; preds = %119, %116
  ret void
}

attributes #0 = { mustprogress noinline optnone ssp uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cmov,+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "tune-cpu"="generic" }
attributes #1 = { "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cmov,+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "tune-cpu"="generic" }
attributes #2 = { mustprogress noinline norecurse optnone ssp uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cmov,+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "tune-cpu"="generic" }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!llvm.ident = !{!5}

!0 = !{i32 2, !"SDK Version", [2 x i32] [i32 15, i32 1]}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 8, !"PIC Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{i32 7, !"frame-pointer", i32 2}
!5 = !{!"clang version 20.1.2"}
!6 = distinct !{!6, !7}
!7 = !{!"llvm.loop.mustprogress"}
