/*
* Tests for Simd Library (http://simd.sourceforge.net).
*
* Copyright (c) 2011-2015 Yermalayeu Ihar.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/
#include "Test/TestUtils.h"
#include "Test/TestPerformance.h"
#include "Test/TestData.h"

namespace Test
{
	namespace
	{
		struct Func1
		{
			typedef void (*FuncPtr)(const uint8_t *src, size_t stride, size_t width, size_t height, 
				 uint8_t * min, uint8_t * max, uint8_t * average);

			FuncPtr func;
			std::string description;

			Func1(const FuncPtr & f, const std::string & d) : func(f), description(d) {}

			void Call(const View & src, uint8_t * min, uint8_t * max, uint8_t * average) const
			{
				TEST_PERFORMANCE_TEST(description);
				func(src.data, src.stride, src.width, src.height, min, max, average);
			}
		};
	}

#define FUNC1(function) Func1(function, #function)

	bool GetStatisticAutoTest(int width, int height, const Func1 & f1, const Func1 & f2)
	{
		bool result = true;

		TEST_LOG_SS(Info, "Test " << f1.description << " & " << f2.description << " [" << width << ", " << height << "].");

		View src(width, height, View::Gray8, NULL, TEST_ALIGN(width));
		FillRandom(src);

		 uint8_t min1, max1, average1;
		 uint8_t min2, max2, average2;

		TEST_EXECUTE_AT_LEAST_MIN_TIME(f1.Call(src, &min1, &max1, &average1));

		TEST_EXECUTE_AT_LEAST_MIN_TIME(f2.Call(src, &min2, &max2, &average2));

        TEST_CHECK_VALUE(min);
        TEST_CHECK_VALUE(max);
        TEST_CHECK_VALUE(average);

		return result;
	}

    bool GetStatisticAutoTest(const Func1 & f1, const Func1 & f2)
    {
        bool result = true;

        result = result && GetStatisticAutoTest(W, H, f1, f2);
        result = result && GetStatisticAutoTest(W + O, H - O, f1, f2);
        result = result && GetStatisticAutoTest(W - O, H + O, f1, f2);

        return result;
    }

	bool GetStatisticAutoTest()
	{
		bool result = true;

		result = result && GetStatisticAutoTest(FUNC1(Simd::Base::GetStatistic), FUNC1(SimdGetStatistic));

#ifdef SIMD_SSE2_ENABLE
        if(Simd::Sse2::Enable)
            result = result && GetStatisticAutoTest(FUNC1(Simd::Sse2::GetStatistic), FUNC1(SimdGetStatistic));
#endif 

#ifdef SIMD_AVX2_ENABLE
        if(Simd::Avx2::Enable)
            result = result && GetStatisticAutoTest(FUNC1(Simd::Avx2::GetStatistic), FUNC1(SimdGetStatistic));
#endif 

#ifdef SIMD_VMX_ENABLE
        if(Simd::Vmx::Enable)
            result = result && GetStatisticAutoTest(FUNC1(Simd::Vmx::GetStatistic), FUNC1(SimdGetStatistic));
#endif 

		return result;
	}

    namespace
    {
        struct FuncM
        {
            typedef void (*FuncPtr)(const uint8_t * mask, size_t stride, size_t width, size_t height, uint8_t index, 
                uint64_t * area, uint64_t * x, uint64_t * y, uint64_t * xx, uint64_t * xy, uint64_t * yy);

            FuncPtr func;
            std::string description;

            FuncM(const FuncPtr & f, const std::string & d) : func(f), description(d) {}

            void Call(const View & mask, uint8_t index, uint64_t * area, uint64_t * x, uint64_t * y, uint64_t * xx, uint64_t * xy, uint64_t * yy) const
            {
                TEST_PERFORMANCE_TEST(description);
                func(mask.data, mask.stride, mask.width, mask.height, index, area, x, y, xx, xy, yy);
            }
        };
    }

#define FUNC_M(function) FuncM(function, #function)

#define ARGS_M(scale, function1, function2) \
    FuncM(function1.func, function1.description + ScaleDescription(scale)), \
    FuncM(function2.func, function2.description + ScaleDescription(scale)) 

    bool GetMomentsAutoTest(ptrdiff_t width, ptrdiff_t height, const FuncM & f1, const FuncM & f2)
    {
        bool result = true;

        TEST_LOG_SS(Info, "Test " << f1.description << " & " << f2.description << " [" << width << ", " << height << "].");

        const uint8_t index = 7;
        View mask(width, height, View::Gray8, NULL, TEST_ALIGN(width));
        FillRandomMask(mask, index);

        uint64_t area1, x1, y1, xx1, xy1, yy1;
        uint64_t area2, x2, y2, xx2, xy2, yy2;

        TEST_EXECUTE_AT_LEAST_MIN_TIME(f1.Call(mask, index, &area1, &x1, &y1, &xx1, &xy1, &yy1));

        TEST_EXECUTE_AT_LEAST_MIN_TIME(f2.Call(mask, index, &area2, &x2, &y2, &xx2, &xy2, &yy2));

        TEST_CHECK_VALUE(area);
        TEST_CHECK_VALUE(x);
        TEST_CHECK_VALUE(y);
        TEST_CHECK_VALUE(xx);
        TEST_CHECK_VALUE(xy);
        TEST_CHECK_VALUE(yy);

        return result;
    }

    bool GetMomentsAutoTest(const Point & scale, const FuncM & f1, const FuncM & f2)
    {
        bool result = true;

        result = result && GetMomentsAutoTest(W*scale.x, H*scale.y, ARGS_M(scale, f1, f2));
        result = result && GetMomentsAutoTest(W*scale.x + O, H*scale.y - O, ARGS_M(scale, f1, f2));
        result = result && GetMomentsAutoTest(W*scale.x - O, H*scale.y + O, ARGS_M(scale, f1, f2));

        return result;
    }

    bool GetMomentsAutoTest(const FuncM & f1, const FuncM & f2)
    {
        bool result = true;

        result = result && GetMomentsAutoTest(Point(1, 1), f1, f2);
#ifdef NDEBUG
        result = result && GetMomentsAutoTest(Point(5, 2), f1, f2);
#else
        result = result && GetMomentsAutoTest(Point(50, 20), f1, f2);
#endif

        return result;
    }

    bool GetMomentsAutoTest()
    {
        bool result = true;

        result = result && GetMomentsAutoTest(FUNC_M(Simd::Base::GetMoments), FUNC_M(SimdGetMoments));

#ifdef SIMD_SSE2_ENABLE
        if(Simd::Sse2::Enable)
            result = result && GetMomentsAutoTest(FUNC_M(Simd::Sse2::GetMoments), FUNC_M(SimdGetMoments));
#endif 

#ifdef SIMD_AVX2_ENABLE
        if(Simd::Avx2::Enable)
            result = result && GetMomentsAutoTest(FUNC_M(Simd::Avx2::GetMoments), FUNC_M(SimdGetMoments));
#endif 

#ifdef SIMD_VMX_ENABLE
        if(Simd::Vmx::Enable)
            result = result && GetMomentsAutoTest(FUNC_M(Simd::Vmx::GetMoments), FUNC_M(SimdGetMoments));
#endif 

        return result;
    }

    namespace
    {
        struct Func3
        {
            typedef void (*FuncPtr)(const uint8_t * src, size_t stride, size_t width, size_t height, uint32_t * sums);

            FuncPtr func;
            std::string description;

            Func3(const FuncPtr & f, const std::string & d) : func(f), description(d) {}

            void Call(const View & src, uint32_t * sums) const
            {
                TEST_PERFORMANCE_TEST(description);
                func(src.data, src.stride, src.width, src.height, sums);
            }
        };
    }

#define FUNC3(function) Func3(function, #function)

    bool GetSumsAutoTest(int width, int height, const Func3 & f1, const Func3 & f2, bool isRow)
    {
        bool result = true;

        TEST_LOG_SS(Info, "Test " << f1.description << " & " << f2.description << " [" << width << ", " << height << "].");

        View src(width, height, View::Gray8, NULL, TEST_ALIGN(width));
        FillRandom(src);

        size_t size = isRow ? height : width;
        Sums sums1(size, 0);
        Sums sums2(size, 0);

        TEST_EXECUTE_AT_LEAST_MIN_TIME(f1.Call(src, sums1.data()));

        TEST_EXECUTE_AT_LEAST_MIN_TIME(f2.Call(src, sums2.data()));

        result = result && Compare(sums1, sums2, 0, true, 32);

        return result;
    }

    bool GetSumsAutoTest(const Func3 & f1, const Func3 & f2, bool isRow)
    {
        bool result = true;

        result = result && GetSumsAutoTest(W, H, f1, f2, isRow);
        result = result && GetSumsAutoTest(W + O, H - O, f1, f2, isRow);
        result = result && GetSumsAutoTest(W - O, H + O, f1, f2, isRow);

        return result;
    }

    bool GetRowSumsAutoTest()
    {
        bool result = true;

        result = result && GetSumsAutoTest(FUNC3(Simd::Base::GetRowSums), FUNC3(SimdGetRowSums), true);

#ifdef SIMD_SSE2_ENABLE
        if(Simd::Sse2::Enable)
            result = result && GetSumsAutoTest(FUNC3(Simd::Sse2::GetRowSums), FUNC3(SimdGetRowSums), true);
#endif 

#ifdef SIMD_AVX2_ENABLE
        if(Simd::Avx2::Enable)
            result = result && GetSumsAutoTest(FUNC3(Simd::Avx2::GetRowSums), FUNC3(SimdGetRowSums), true);
#endif 

#ifdef SIMD_VMX_ENABLE
        if(Simd::Vmx::Enable)
            result = result && GetSumsAutoTest(FUNC3(Simd::Vmx::GetRowSums), FUNC3(SimdGetRowSums), true);
#endif 

        return result;
    }

    bool GetColSumsAutoTest()
    {
        bool result = true;

        result = result && GetSumsAutoTest(FUNC3(Simd::Base::GetColSums), FUNC3(SimdGetColSums), false);

#ifdef SIMD_SSE2_ENABLE
        if(Simd::Sse2::Enable)
            result = result && GetSumsAutoTest(FUNC3(Simd::Sse2::GetColSums), FUNC3(SimdGetColSums), false);
#endif 

#ifdef SIMD_AVX2_ENABLE
        if(Simd::Avx2::Enable)
            result = result && GetSumsAutoTest(FUNC3(Simd::Avx2::GetColSums), FUNC3(SimdGetColSums), false);
#endif 

#ifdef SIMD_VMX_ENABLE
        if(Simd::Vmx::Enable)
            result = result && GetSumsAutoTest(FUNC3(Simd::Vmx::GetColSums), FUNC3(SimdGetColSums), false);
#endif 

        return result;
    }

    bool GetAbsDyRowSumsAutoTest()
    {
        bool result = true;

        result = result && GetSumsAutoTest(FUNC3(Simd::Base::GetAbsDyRowSums), FUNC3(SimdGetAbsDyRowSums), true);

#ifdef SIMD_SSE2_ENABLE
        if(Simd::Sse2::Enable)
            result = result && GetSumsAutoTest(FUNC3(Simd::Sse2::GetAbsDyRowSums), FUNC3(SimdGetAbsDyRowSums), true);
#endif 

#ifdef SIMD_AVX2_ENABLE
        if(Simd::Avx2::Enable)
            result = result && GetSumsAutoTest(FUNC3(Simd::Avx2::GetAbsDyRowSums), FUNC3(SimdGetAbsDyRowSums), true);
#endif 

#ifdef SIMD_VMX_ENABLE
        if(Simd::Vmx::Enable)
            result = result && GetSumsAutoTest(FUNC3(Simd::Vmx::GetAbsDyRowSums), FUNC3(SimdGetAbsDyRowSums), true);
#endif 

        return result;
    }

    bool GetAbsDxColSumsAutoTest()
    {
        bool result = true;

        result = result && GetSumsAutoTest(FUNC3(Simd::Base::GetAbsDxColSums), FUNC3(SimdGetAbsDxColSums), false);

#ifdef SIMD_SSE2_ENABLE
        if(Simd::Sse2::Enable)
            result = result && GetSumsAutoTest(FUNC3(Simd::Sse2::GetAbsDxColSums), FUNC3(SimdGetAbsDxColSums), false);
#endif 

#ifdef SIMD_AVX2_ENABLE
        if(Simd::Avx2::Enable)
            result = result && GetSumsAutoTest(FUNC3(Simd::Avx2::GetAbsDxColSums), FUNC3(SimdGetAbsDxColSums), false);
#endif 

#ifdef SIMD_VMX_ENABLE
        if(Simd::Vmx::Enable)
            result = result && GetSumsAutoTest(FUNC3(Simd::Vmx::GetAbsDxColSums), FUNC3(SimdGetAbsDxColSums), false);
#endif 

        return result;
    }

    namespace
    {
        struct Func4
        {
            typedef void (*FuncPtr)(const uint8_t * src, size_t stride, size_t width, size_t height, uint64_t * sum);

            FuncPtr func;
            std::string description;

            Func4(const FuncPtr & f, const std::string & d) : func(f), description(d) {}

            void Call(const View & src, uint64_t * sum) const
            {
                TEST_PERFORMANCE_TEST(description);
                func(src.data, src.stride, src.width, src.height, sum);
            }
        };
    }

#define FUNC4(function) Func4(function, #function)

    bool SumAutoTest(int width, int height, const Func4 & f1, const Func4 & f2)
    {
        bool result = true;

        TEST_LOG_SS(Info, "Test " << f1.description << " & " << f2.description << " [" << width << ", " << height << "].");

        View src(width, height, View::Gray8, NULL, TEST_ALIGN(width));
        FillRandom(src);

        uint64_t sum1;
        uint64_t sum2;

        TEST_EXECUTE_AT_LEAST_MIN_TIME(f1.Call(src, &sum1));

        TEST_EXECUTE_AT_LEAST_MIN_TIME(f2.Call(src, &sum2));

        TEST_CHECK_VALUE(sum);

        return result;
    }

    bool SumAutoTest(const Func4 & f1, const Func4 & f2)
    {
        bool result = true;

        result = result && SumAutoTest(W, H, f1, f2);
        result = result && SumAutoTest(W + O, H - O, f1, f2);
        result = result && SumAutoTest(W - O, H + O, f1, f2);

        return result;
    }

    bool ValueSumAutoTest()
    {
        bool result = true;

        result = result && SumAutoTest(FUNC4(Simd::Base::ValueSum), FUNC4(SimdValueSum));

#ifdef SIMD_SSE2_ENABLE
        if(Simd::Sse2::Enable)
            result = result && SumAutoTest(FUNC4(Simd::Sse2::ValueSum), FUNC4(SimdValueSum));
#endif 

#ifdef SIMD_AVX2_ENABLE
        if(Simd::Avx2::Enable)
            result = result && SumAutoTest(FUNC4(Simd::Avx2::ValueSum), FUNC4(SimdValueSum));
#endif 

#ifdef SIMD_VMX_ENABLE
        if(Simd::Vmx::Enable)
            result = result && SumAutoTest(FUNC4(Simd::Vmx::ValueSum), FUNC4(SimdValueSum));
#endif 

        return result;
    }

    bool SquareSumAutoTest()
    {
        bool result = true;

        result = result && SumAutoTest(FUNC4(Simd::Base::SquareSum), FUNC4(SimdSquareSum));

#ifdef SIMD_SSE2_ENABLE
        if(Simd::Sse2::Enable)
            result = result && SumAutoTest(FUNC4(Simd::Sse2::SquareSum), FUNC4(SimdSquareSum));
#endif 

#ifdef SIMD_AVX2_ENABLE
        if(Simd::Avx2::Enable)
            result = result && SumAutoTest(FUNC4(Simd::Avx2::SquareSum), FUNC4(SimdSquareSum));
#endif 

#ifdef SIMD_VMX_ENABLE
        if(Simd::Vmx::Enable)
            result = result && SumAutoTest(FUNC4(Simd::Vmx::SquareSum), FUNC4(SimdSquareSum));
#endif

        return result;
    }

    bool SobelDxAbsSumAutoTest()
    {
        bool result = true;

        result = result && SumAutoTest(FUNC4(Simd::Base::SobelDxAbsSum), FUNC4(SimdSobelDxAbsSum));

#ifdef SIMD_SSSE3_ENABLE
        if(Simd::Ssse3::Enable)
            result = result && SumAutoTest(FUNC4(Simd::Ssse3::SobelDxAbsSum), FUNC4(SimdSobelDxAbsSum));
#endif 

#ifdef SIMD_AVX2_ENABLE
        if(Simd::Avx2::Enable)
            result = result && SumAutoTest(FUNC4(Simd::Avx2::SobelDxAbsSum), FUNC4(SimdSobelDxAbsSum));
#endif 

#ifdef SIMD_VMX_ENABLE
        if(Simd::Vmx::Enable)
            result = result && SumAutoTest(FUNC4(Simd::Vmx::SobelDxAbsSum), FUNC4(SimdSobelDxAbsSum));
#endif

        return result;
    }

    bool SobelDyAbsSumAutoTest()
    {
        bool result = true;

        result = result && SumAutoTest(FUNC4(Simd::Base::SobelDyAbsSum), FUNC4(SimdSobelDyAbsSum));

#ifdef SIMD_SSSE3_ENABLE
        if(Simd::Ssse3::Enable)
            result = result && SumAutoTest(FUNC4(Simd::Ssse3::SobelDyAbsSum), FUNC4(SimdSobelDyAbsSum));
#endif 

#ifdef SIMD_AVX2_ENABLE
        if(Simd::Avx2::Enable)
            result = result && SumAutoTest(FUNC4(Simd::Avx2::SobelDyAbsSum), FUNC4(SimdSobelDyAbsSum));
#endif 

#ifdef SIMD_VMX_ENABLE
        if(Simd::Vmx::Enable)
            result = result && SumAutoTest(FUNC4(Simd::Vmx::SobelDyAbsSum), FUNC4(SimdSobelDyAbsSum));
#endif

        return result;
    }

    bool LaplaceAbsSumAutoTest()
    {
        bool result = true;

        result = result && SumAutoTest(FUNC4(Simd::Base::LaplaceAbsSum), FUNC4(SimdLaplaceAbsSum));

#ifdef SIMD_SSSE3_ENABLE
        if(Simd::Ssse3::Enable)
            result = result && SumAutoTest(FUNC4(Simd::Ssse3::LaplaceAbsSum), FUNC4(SimdLaplaceAbsSum));
#endif 

#ifdef SIMD_AVX2_ENABLE
        if(Simd::Avx2::Enable)
            result = result && SumAutoTest(FUNC4(Simd::Avx2::LaplaceAbsSum), FUNC4(SimdLaplaceAbsSum));
#endif 

#ifdef SIMD_VMX_ENABLE
        if(Simd::Vmx::Enable)
            result = result && SumAutoTest(FUNC4(Simd::Vmx::LaplaceAbsSum), FUNC4(SimdLaplaceAbsSum));
#endif

        return result;
    }

    namespace
    {
        struct Func5
        {
            typedef void (*FuncPtr)(const uint8_t * a, size_t aStride, const uint8_t * b, size_t bStride, size_t width, size_t height, uint64_t * sum);

            FuncPtr func;
            std::string description;

            Func5(const FuncPtr & f, const std::string & d) : func(f), description(d) {}

            void Call(const View & a, const View & b, uint64_t * sum) const
            {
                TEST_PERFORMANCE_TEST(description);
                func(a.data, a.stride, b.data, b.stride, a.width, a.height, sum);
            }
        };
    }

#define FUNC5(function) Func5(function, #function)

    bool CorrelationSumAutoTest(int width, int height, const Func5 & f1, const Func5 & f2)
    {
        bool result = true;

        TEST_LOG_SS(Info, "Test " << f1.description << " & " << f2.description << " [" << width << ", " << height << "].");

        View a(width, height, View::Gray8, NULL, TEST_ALIGN(width));
        FillRandom(a);
        View b(width, height, View::Gray8, NULL, TEST_ALIGN(width));
        FillRandom(b);

        uint64_t sum1;
        uint64_t sum2;

        TEST_EXECUTE_AT_LEAST_MIN_TIME(f1.Call(a, b, &sum1));

        TEST_EXECUTE_AT_LEAST_MIN_TIME(f2.Call(a, b, &sum2));

        TEST_CHECK_VALUE(sum);

        return result;
    }

    bool CorrelationSumAutoTest(const Func5 & f1, const Func5 & f2)
    {
        bool result = true;

        result = result && CorrelationSumAutoTest(W, H, f1, f2);
        result = result && CorrelationSumAutoTest(W + O, H - O, f1, f2);
        result = result && CorrelationSumAutoTest(W - O, H + O, f1, f2);

        return result;
    }

    bool CorrelationSumAutoTest()
    {
        bool result = true;

        result = result && CorrelationSumAutoTest(FUNC5(Simd::Base::CorrelationSum), FUNC5(SimdCorrelationSum));

#ifdef SIMD_SSE2_ENABLE
        if(Simd::Sse2::Enable)
            result = result && CorrelationSumAutoTest(FUNC5(Simd::Sse2::CorrelationSum), FUNC5(SimdCorrelationSum));
#endif 

#ifdef SIMD_AVX2_ENABLE
        if(Simd::Avx2::Enable)
            result = result && CorrelationSumAutoTest(FUNC5(Simd::Avx2::CorrelationSum), FUNC5(SimdCorrelationSum));
#endif 

#ifdef SIMD_VMX_ENABLE
        if(Simd::Vmx::Enable)
            result = result && CorrelationSumAutoTest(FUNC5(Simd::Vmx::CorrelationSum), FUNC5(SimdCorrelationSum));
#endif 

        return result;
    }

    //-----------------------------------------------------------------------

    bool GetStatisticDataTest(bool create, int width, int height, const Func1 & f)
    {
        bool result = true;

        Data data(f.description);

        TEST_LOG_SS(Info, (create ? "Create" : "Verify") << " test " << f.description << " [" << width << ", " << height << "].");

        View src(width, height, View::Gray8, NULL, TEST_ALIGN(width));

        uint8_t min1, max1, average1;
        uint8_t min2, max2, average2;

        if(create)
        {
            FillRandom(src);

            TEST_SAVE(src);

            f.Call(src, &min1, &max1, &average1);

            TEST_SAVE(min1);
            TEST_SAVE(max1);
            TEST_SAVE(average1);
        }
        else
        {
            TEST_LOAD(src);

            TEST_LOAD(min1);
            TEST_LOAD(max1);
            TEST_LOAD(average1);

            f.Call(src, &min2, &max2, &average2);

            TEST_SAVE(min2);
            TEST_SAVE(max2);
            TEST_SAVE(average2);

            TEST_CHECK_VALUE(min);
            TEST_CHECK_VALUE(max);
            TEST_CHECK_VALUE(average);
        }

        return result;
    }

    bool GetStatisticDataTest(bool create)
    {
        bool result = true;

        result = result && GetStatisticDataTest(create, DW, DH, FUNC1(SimdGetStatistic));

        return result;
    }

    bool GetMomentsDataTest(bool create, int width, int height, const FuncM & f)
    {
        bool result = true;

        Data data(f.description);

        TEST_LOG_SS(Info, (create ? "Create" : "Verify") << " test " << f.description << " [" << width << ", " << height << "].");

        const uint8_t index = 7;
        View mask(width, height, View::Gray8, NULL, TEST_ALIGN(width));

        uint64_t area1, x1, y1, xx1, xy1, yy1;
        uint64_t area2, x2, y2, xx2, xy2, yy2;

        if(create)
        {
            FillRandomMask(mask, index);

            TEST_SAVE(mask);

            f.Call(mask, index, &area1, &x1, &y1, &xx1, &xy1, &yy1);

            TEST_SAVE(area1);
            TEST_SAVE(x1);
            TEST_SAVE(y1);
            TEST_SAVE(xx1);
            TEST_SAVE(xy1);
            TEST_SAVE(yy1);
        }
        else
        {
            TEST_LOAD(mask);

            TEST_LOAD(area1);
            TEST_LOAD(x1);
            TEST_LOAD(y1);
            TEST_LOAD(xx1);
            TEST_LOAD(xy1);
            TEST_LOAD(yy1);

            f.Call(mask, index, &area2, &x2, &y2, &xx2, &xy2, &yy2);

            TEST_SAVE(area2);
            TEST_SAVE(x2);
            TEST_SAVE(y2);
            TEST_SAVE(xx2);
            TEST_SAVE(xy2);
            TEST_SAVE(yy2);

            TEST_CHECK_VALUE(area);
            TEST_CHECK_VALUE(x);
            TEST_CHECK_VALUE(y);
            TEST_CHECK_VALUE(xx);
            TEST_CHECK_VALUE(xy);
            TEST_CHECK_VALUE(yy);
        }

        return result;
    }

    bool GetMomentsDataTest(bool create)
    {
        bool result = true;

        result = result && GetMomentsDataTest(create, DW, DH, FUNC_M(SimdGetMoments));

        return result;
    }

    bool GetSumsDataTest(bool create, int width, int height, const Func3 & f, bool isRow)
    {
        bool result = true;

        Data data(f.description);

        TEST_LOG_SS(Info, (create ? "Create" : "Verify") << " test " << f.description << " [" << width << ", " << height << "].");

        View src(width, height, View::Gray8, NULL, TEST_ALIGN(width));

        size_t size = isRow ? height : width;
        Sums sums1(size, 0);
        Sums sums2(size, 0);

        if(create)
        {
            FillRandom(src);

            TEST_SAVE(src);

            f.Call(src, sums1.data());

            TEST_SAVE(sums1);
        }
        else
        {
            TEST_LOAD(src);

            TEST_LOAD(sums1);

            f.Call(src, sums2.data());

            TEST_SAVE(sums2);

            result = result && Compare(sums1, sums2, 0, true, 32);
        }

        return result;
    }

    bool GetRowSumsDataTest(bool create)
    {
        bool result = true;

        result = result && GetSumsDataTest(create, DW, DH, FUNC3(SimdGetRowSums), true);

        return result;
    }

    bool GetColSumsDataTest(bool create)
    {
        bool result = true;

        result = result && GetSumsDataTest(create, DW, DH, FUNC3(SimdGetColSums), false);

        return result;
    }

    bool GetAbsDyRowSumsDataTest(bool create)
    {
        bool result = true;

        result = result && GetSumsDataTest(create, DW, DH, FUNC3(SimdGetAbsDyRowSums), true);

        return result;
    }

    bool GetAbsDxColSumsDataTest(bool create)
    {
        bool result = true;

        result = result && GetSumsDataTest(create, DW, DH, FUNC3(SimdGetAbsDxColSums), false);

        return result;
    }

    bool SumDataTest(bool create, int width, int height, const Func4 & f)
    {
        bool result = true;

        Data data(f.description);

        TEST_LOG_SS(Info, (create ? "Create" : "Verify") << " test " << f.description << " [" << width << ", " << height << "].");

        View src(width, height, View::Gray8, NULL, TEST_ALIGN(width));

        uint64_t sum1, sum2;

        if(create)
        {
            FillRandom(src);

            TEST_SAVE(src);

            f.Call(src, &sum1);

            TEST_SAVE(sum1);
        }
        else
        {
            TEST_LOAD(src);

            TEST_LOAD(sum1);

            f.Call(src, &sum2);

            TEST_SAVE(sum2);

            TEST_CHECK_VALUE(sum);
        }

        return result;
    }

    bool ValueSumDataTest(bool create)
    {
        bool result = true;

        result = result && SumDataTest(create, DW, DH, FUNC4(SimdValueSum));

        return result;
    }

    bool SquareSumDataTest(bool create)
    {
        bool result = true;

        result = result && SumDataTest(create, DW, DH, FUNC4(SimdSquareSum));

        return result;
    }

    bool SobelDxAbsSumDataTest(bool create)
    {
        bool result = true;

        result = result && SumDataTest(create, DW, DH, FUNC4(SimdSobelDxAbsSum));

        return result;
    }

    bool SobelDyAbsSumDataTest(bool create)
    {
        bool result = true;

        result = result && SumDataTest(create, DW, DH, FUNC4(SimdSobelDyAbsSum));

        return result;
    }

    bool LaplaceAbsSumDataTest(bool create)
    {
        bool result = true;

        result = result && SumDataTest(create, DW, DH, FUNC4(SimdLaplaceAbsSum));

        return result;
    }

    bool CorrelationSumDataTest(bool create, int width, int height, const Func5 & f)
    {
        bool result = true;

        Data data(f.description);

        TEST_LOG_SS(Info, (create ? "Create" : "Verify") << " test " << f.description << " [" << width << ", " << height << "].");

        View a(width, height, View::Gray8, NULL, TEST_ALIGN(width));
        View b(width, height, View::Gray8, NULL, TEST_ALIGN(width));

        uint64_t sum1, sum2;

        if(create)
        {
            FillRandom(a);
            FillRandom(b);

            TEST_SAVE(a);
            TEST_SAVE(b);

            f.Call(a, b, &sum1);

            TEST_SAVE(sum1);
        }
        else
        {
            TEST_LOAD(a);
            TEST_LOAD(b);

            TEST_LOAD(sum1);

            f.Call(a, b, &sum2);

            TEST_SAVE(sum2);

            TEST_CHECK_VALUE(sum);
        }

        return result;
    }

    bool CorrelationSumDataTest(bool create)
    {
        bool result = true;

        result = result && CorrelationSumDataTest(create, DW, DH, FUNC5(SimdCorrelationSum));

        return result;
    }
}
