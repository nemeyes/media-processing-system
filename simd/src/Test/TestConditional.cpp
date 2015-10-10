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
        struct FuncC8U
        {
            typedef void (*FuncPtr)(const uint8_t * src, size_t stride, size_t width, size_t height, 
                uint8_t value, SimdCompareType compareType, uint32_t * count);

            FuncPtr func;
            std::string description;

            FuncC8U(const FuncPtr & f, const std::string & d) : func(f), description(d) {}

            void Call(const View & src, uint8_t value, SimdCompareType compareType, uint32_t & count) const
            {
                TEST_PERFORMANCE_TEST(description);
                func(src.data, src.stride, src.width, src.height, value, compareType, &count);
            }
        };
    }

#define ARGS_C8U(width, height, type, function1, function2) \
    width, height, type, \
    FuncC8U(function1.func, function1.description + CompareTypeDescription(type)), \
    FuncC8U(function2.func, function2.description + CompareTypeDescription(type))

#define FUNC_C8U(function) \
    FuncC8U(function, std::string(#function))

    bool ConditionalCount8uAutoTest(int width, int height, SimdCompareType type, const FuncC8U & f1, const FuncC8U & f2)
    {
        bool result = true;

        TEST_LOG_SS(Info, "Test " << f1.description << " & " << f2.description << " [" << width << ", " << height << "].");

        View src(width, height, View::Gray8, NULL, TEST_ALIGN(width));
        FillRandom(src);

        uint8_t value = 127;
        uint32_t c1, c2;

        TEST_EXECUTE_AT_LEAST_MIN_TIME(f1.Call(src, value, type, c1));

        TEST_EXECUTE_AT_LEAST_MIN_TIME(f2.Call(src, value, type, c2));

        TEST_CHECK_VALUE(c);

        return result;
    }

    bool ConditionalCount8uAutoTest(const FuncC8U & f1, const FuncC8U & f2)
    {
        bool result = true;

        for(SimdCompareType type = SimdCompareEqual; type <= SimdCompareLesserOrEqual && result; type = SimdCompareType(type + 1))
        {
            result = result && ConditionalCount8uAutoTest(ARGS_C8U(W, H, type, f1, f2));
            result = result && ConditionalCount8uAutoTest(ARGS_C8U(W + O, H - O, type, f1, f2));
            result = result && ConditionalCount8uAutoTest(ARGS_C8U(W - O, H + O, type, f1, f2));
        }

        return result;
    }

    bool ConditionalCount8uAutoTest()
    {
        bool result = true;

        result = result && ConditionalCount8uAutoTest(FUNC_C8U(Simd::Base::ConditionalCount8u), FUNC_C8U(SimdConditionalCount8u));

#ifdef SIMD_SSE2_ENABLE
        if(Simd::Sse2::Enable)
            result = result && ConditionalCount8uAutoTest(FUNC_C8U(Simd::Sse2::ConditionalCount8u), FUNC_C8U(SimdConditionalCount8u));
#endif 

#ifdef SIMD_AVX2_ENABLE
        if(Simd::Avx2::Enable)
            result = result && ConditionalCount8uAutoTest(FUNC_C8U(Simd::Avx2::ConditionalCount8u), FUNC_C8U(SimdConditionalCount8u));
#endif 

#ifdef SIMD_VMX_ENABLE
        if(Simd::Vmx::Enable)
            result = result && ConditionalCount8uAutoTest(FUNC_C8U(Simd::Vmx::ConditionalCount8u), FUNC_C8U(SimdConditionalCount8u));
#endif 

        return result;
    }

    namespace
    {
        struct FuncC16I
        {
            typedef void (*FuncPtr)(const uint8_t * src, size_t stride, size_t width, size_t height, 
                int16_t value, SimdCompareType compareType, uint32_t * count);

            FuncPtr func;
            std::string description;

            FuncC16I(const FuncPtr & f, const std::string & d) : func(f), description(d) {}

            void Call(const View & src, int16_t value, SimdCompareType compareType, uint32_t & count) const
            {
                TEST_PERFORMANCE_TEST(description);
                func(src.data, src.stride, src.width, src.height, value, compareType, &count);
            }
        };
    }

#define ARGS_C16I(width, height, type, function1, function2) \
    width, height, type, \
    FuncC16I(function1.func, function1.description + CompareTypeDescription(type)), \
    FuncC16I(function2.func, function2.description + CompareTypeDescription(type))

#define FUNC_C16I(function) \
    FuncC16I(function, std::string(#function))

    bool ConditionalCount16iAutoTest(int width, int height, SimdCompareType type, const FuncC16I & f1, const FuncC16I & f2)
    {
        bool result = true;

        TEST_LOG_SS(Info, "Test " << f1.description << " & " << f2.description << " [" << width << ", " << height << "].");

        View src(width, height, View::Int16, NULL, TEST_ALIGN(width));
        FillRandom(src);

        int16_t value = 1111;
        uint32_t c1, c2;

        TEST_EXECUTE_AT_LEAST_MIN_TIME(f1.Call(src, value, type, c1));

        TEST_EXECUTE_AT_LEAST_MIN_TIME(f2.Call(src, value, type, c2));

        TEST_CHECK_VALUE(c);

        return result;
    }

    bool ConditionalCount16iAutoTest(const FuncC16I & f1, const FuncC16I & f2)
    {
        bool result = true;

        for(SimdCompareType type = SimdCompareEqual; type <= SimdCompareLesserOrEqual && result; type = SimdCompareType(type + 1))
        {
            result = result && ConditionalCount16iAutoTest(ARGS_C16I(W, H, type, f1, f2));
            result = result && ConditionalCount16iAutoTest(ARGS_C16I(W + O, H - O, type, f1, f2));
            result = result && ConditionalCount16iAutoTest(ARGS_C16I(W - O, H + O, type, f1, f2));
        }

        return result;
    }

    bool ConditionalCount16iAutoTest()
    {
        bool result = true;

        result = result && ConditionalCount16iAutoTest(FUNC_C16I(Simd::Base::ConditionalCount16i), FUNC_C16I(SimdConditionalCount16i));

#ifdef SIMD_SSE2_ENABLE
        if(Simd::Sse2::Enable)
            result = result && ConditionalCount16iAutoTest(FUNC_C16I(Simd::Sse2::ConditionalCount16i), FUNC_C16I(SimdConditionalCount16i));
#endif 

#ifdef SIMD_AVX2_ENABLE
        if(Simd::Avx2::Enable)
            result = result && ConditionalCount16iAutoTest(FUNC_C16I(Simd::Avx2::ConditionalCount16i), FUNC_C16I(SimdConditionalCount16i));
#endif 

#ifdef SIMD_VMX_ENABLE
        if(Simd::Vmx::Enable)
            result = result && ConditionalCount16iAutoTest(FUNC_C16I(Simd::Vmx::ConditionalCount16i), FUNC_C16I(SimdConditionalCount16i));
#endif 

        return result;
    }

    namespace 
    {
        struct FuncS
        {
            typedef void (*FuncPtr)(const uint8_t * src, size_t srcStride, size_t width, size_t height, 
                const uint8_t * mask, size_t maskStride, uint8_t value, SimdCompareType compareType, uint64_t * sum);

            FuncPtr func;
            std::string description;

            FuncS(const FuncPtr & f, const std::string & d) : func(f), description(d) {}

            void Call(const View & src, const View & mask, uint8_t value, SimdCompareType compareType, uint64_t & sum) const
            {
                TEST_PERFORMANCE_TEST(description);
                func(src.data, src.stride, src.width, src.height, mask.data, mask.stride, value, compareType, &sum);
            }
        };
    }

#define ARGS_S(width, height, type, function1, function2) \
    width, height, type, \
    FuncS(function1.func, function1.description + CompareTypeDescription(type)), \
    FuncS(function2.func, function2.description + CompareTypeDescription(type))

#define FUNC_S(function) \
    FuncS(function, std::string(#function))

    bool ConditionalSumAutoTest(int width, int height, SimdCompareType type, const FuncS & f1, const FuncS & f2)
    {
        bool result = true;

        TEST_LOG_SS(Info, "Test " << f1.description << " & " << f2.description << " [" << width << ", " << height << "].");

        View src(width, height, View::Gray8, NULL, TEST_ALIGN(width));
        FillRandom(src);
        View mask(width, height, View::Gray8, NULL, TEST_ALIGN(width));
        FillRandom(mask);

        uint8_t value = 127;
        uint64_t s1, s2;

        TEST_EXECUTE_AT_LEAST_MIN_TIME(f1.Call(src, mask, value, type, s1));

        TEST_EXECUTE_AT_LEAST_MIN_TIME(f2.Call(src, mask, value, type, s2));

        TEST_CHECK_VALUE(s);

        return result;
    }

    bool ConditionalSumAutoTest(const FuncS & f1, const FuncS & f2)
    {
        bool result = true;

        for(SimdCompareType type = SimdCompareEqual; type <= SimdCompareLesserOrEqual && result; type = SimdCompareType(type + 1))
        {
            result = result && ConditionalSumAutoTest(ARGS_S(W, H, type, f1, f2));
            result = result && ConditionalSumAutoTest(ARGS_S(W + O, H - O, type, f1, f2));
            result = result && ConditionalSumAutoTest(ARGS_S(W - O, H + O, type, f1, f2));
        }

        return result;
    }

    bool ConditionalSumAutoTest()
    {
        bool result = true;

        result = result && ConditionalSumAutoTest(FUNC_S(Simd::Base::ConditionalSum), FUNC_S(SimdConditionalSum));

#ifdef SIMD_SSE2_ENABLE
        if(Simd::Sse2::Enable)
            result = result && ConditionalSumAutoTest(FUNC_S(Simd::Sse2::ConditionalSum), FUNC_S(SimdConditionalSum));
#endif 

#ifdef SIMD_AVX2_ENABLE
        if(Simd::Avx2::Enable)
            result = result && ConditionalSumAutoTest(FUNC_S(Simd::Avx2::ConditionalSum), FUNC_S(SimdConditionalSum));
#endif 

#ifdef SIMD_VMX_ENABLE
        if(Simd::Vmx::Enable)
            result = result && ConditionalSumAutoTest(FUNC_S(Simd::Vmx::ConditionalSum), FUNC_S(SimdConditionalSum));
#endif 

        return result;
    }

    bool ConditionalSquareSumAutoTest()
    {
        bool result = true;

        result = result && ConditionalSumAutoTest(FUNC_S(Simd::Base::ConditionalSquareSum), FUNC_S(SimdConditionalSquareSum));

#ifdef SIMD_SSE2_ENABLE
        if(Simd::Sse2::Enable)
            result = result && ConditionalSumAutoTest(FUNC_S(Simd::Sse2::ConditionalSquareSum), FUNC_S(SimdConditionalSquareSum));
#endif 

#ifdef SIMD_AVX2_ENABLE
        if(Simd::Avx2::Enable)
            result = result && ConditionalSumAutoTest(FUNC_S(Simd::Avx2::ConditionalSquareSum), FUNC_S(SimdConditionalSquareSum));
#endif 

#ifdef SIMD_VMX_ENABLE
        if(Simd::Vmx::Enable)
            result = result && ConditionalSumAutoTest(FUNC_S(Simd::Vmx::ConditionalSquareSum), FUNC_S(SimdConditionalSquareSum));
#endif 

        return result;
    }

    bool ConditionalSquareGradientSumAutoTest()
    {
        bool result = true;

        result = result && ConditionalSumAutoTest(FUNC_S(Simd::Base::ConditionalSquareGradientSum), FUNC_S(SimdConditionalSquareGradientSum));

#ifdef SIMD_SSE2_ENABLE
        if(Simd::Sse2::Enable)
            result = result && ConditionalSumAutoTest(FUNC_S(Simd::Sse2::ConditionalSquareGradientSum), FUNC_S(SimdConditionalSquareGradientSum));
#endif 

#ifdef SIMD_AVX2_ENABLE
        if(Simd::Avx2::Enable)
            result = result && ConditionalSumAutoTest(FUNC_S(Simd::Avx2::ConditionalSquareGradientSum), FUNC_S(SimdConditionalSquareGradientSum));
#endif 

#ifdef SIMD_VMX_ENABLE
        if(Simd::Vmx::Enable)
            result = result && ConditionalSumAutoTest(FUNC_S(Simd::Vmx::ConditionalSquareGradientSum), FUNC_S(SimdConditionalSquareGradientSum));
#endif 

        return result;
    }

    //-----------------------------------------------------------------------

    bool ConditionalCount8uDataTest(bool create, int width, int height, SimdCompareType type, const FuncC8U & f)
    {
        bool result = true;

        Data data(f.description);

        TEST_LOG_SS(Info, (create ? "Create" : "Verify") << " test " << f.description << " [" << width << ", " << height << "].");

        View src(width, height, View::Gray8, NULL, TEST_ALIGN(width));
        uint8_t value = 127;
        uint32_t c1, c2;

        if(create)
        {
            FillRandom(src);

            TEST_SAVE(src);

            f.Call(src, value, type, c1);

            TEST_SAVE(c1);
        }
        else
        {
            TEST_LOAD(src);

            TEST_LOAD(c1);

            f.Call(src, value, type, c2);

            TEST_SAVE(c2);

            TEST_CHECK_VALUE(c);
        }

        return result;
    }

    bool ConditionalCount8uDataTest(bool create)
    {
        bool result = true;

        FuncC8U f = FUNC_C8U(SimdConditionalCount8u);
        for(SimdCompareType type = SimdCompareEqual; type <= SimdCompareLesserOrEqual && result; type = SimdCompareType(type + 1))
        {
            result = result && ConditionalCount8uDataTest(create, DW, DH, type, FuncC8U(f.func, f.description + Data::Description(type)));
        }

        return result;
    }

    bool ConditionalCount16iDataTest(bool create, int width, int height, SimdCompareType type, const FuncC16I & f)
    {
        bool result = true;

        Data data(f.description);

        TEST_LOG_SS(Info, (create ? "Create" : "Verify") << " test " << f.description << " [" << width << ", " << height << "].");

        View src(width, height, View::Int16, NULL, TEST_ALIGN(width));
        int16_t value = 1111;
        uint32_t c1, c2;

        if(create)
        {
            FillRandom(src);

            TEST_SAVE(src);

            f.Call(src, value, type, c1);

            TEST_SAVE(c1);
        }
        else
        {
            TEST_LOAD(src);

            TEST_LOAD(c1);

            f.Call(src, value, type, c2);

            TEST_SAVE(c2);

            TEST_CHECK_VALUE(c);
        }

        return result;
    }

    bool ConditionalCount16iDataTest(bool create)
    {
        bool result = true;

        FuncC16I f = FUNC_C16I(SimdConditionalCount16i);
        for(SimdCompareType type = SimdCompareEqual; type <= SimdCompareLesserOrEqual && result; type = SimdCompareType(type + 1))
        {
            result = result && ConditionalCount16iDataTest(create, DW, DH, type, FuncC16I(f.func, f.description + Data::Description(type)));
        }

        return result;
    }

    bool ConditionalSumDataTest(bool create, int width, int height, SimdCompareType type, const FuncS & f)
    {
        bool result = true;

        Data data(f.description);

        TEST_LOG_SS(Info, (create ? "Create" : "Verify") << " test " << f.description << " [" << width << ", " << height << "].");

        View src(width, height, View::Gray8, NULL, TEST_ALIGN(width));
        View mask(width, height, View::Gray8, NULL, TEST_ALIGN(width));
        uint8_t value = 127;
        uint64_t s1, s2;

        if(create)
        {
            FillRandom(src);
            FillRandom(mask);

            TEST_SAVE(src);
            TEST_SAVE(mask);

            f.Call(src, mask, value, type, s1);

            TEST_SAVE(s1);
        }
        else
        {
            TEST_LOAD(src);
            TEST_LOAD(mask);

            TEST_LOAD(s1);

            f.Call(src, mask, value, type, s2);

            TEST_SAVE(s2);

            TEST_CHECK_VALUE(s);
        }

        return result;
    }

    bool ConditionalSumDataTest(bool create)
    {
        bool result = true;

        FuncS f = FUNC_S(SimdConditionalSum);
        for(SimdCompareType type = SimdCompareEqual; type <= SimdCompareLesserOrEqual && result; type = SimdCompareType(type + 1))
        {
            result = result && ConditionalSumDataTest(create, DW, DH, type, FuncS(f.func, f.description + Data::Description(type)));
        }

        return result;
    }

    bool ConditionalSquareSumDataTest(bool create)
    {
        bool result = true;

        FuncS f = FUNC_S(SimdConditionalSquareSum);
        for(SimdCompareType type = SimdCompareEqual; type <= SimdCompareLesserOrEqual && result; type = SimdCompareType(type + 1))
        {
            result = result && ConditionalSumDataTest(create, DW, DH, type, FuncS(f.func, f.description + Data::Description(type)));
        }

        return result;
    }

    bool ConditionalSquareGradientSumDataTest(bool create)
    {
        bool result = true;

        FuncS f = FUNC_S(SimdConditionalSquareGradientSum);
        for(SimdCompareType type = SimdCompareEqual; type <= SimdCompareLesserOrEqual && result; type = SimdCompareType(type + 1))
        {
            result = result && ConditionalSumDataTest(create, DW, DH, type, FuncS(f.func, f.description + Data::Description(type)));
        }

        return result;
    }
}
