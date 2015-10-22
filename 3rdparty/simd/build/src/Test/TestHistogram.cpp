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
        struct FuncH
        {
            typedef void (*FuncPtr)(
                const uint8_t *src, size_t width, size_t height, size_t stride, uint32_t * histogram);

            FuncPtr func;
            std::string description;

            FuncH(const FuncPtr & f, const std::string & d) : func(f), description(d) {}

            void Call(const View & src, uint32_t * histogram) const
            {
                TEST_PERFORMANCE_TEST(description);
                func(src.data, src.width, src.height, src.stride, histogram);
            }
        };       
        
        struct FuncHM
        {
            typedef void (*FuncPtr)(const uint8_t * src, size_t srcStride, size_t width, size_t height, 
                const uint8_t * mask, size_t maskStride, uint8_t index, uint32_t * histogram);

            FuncPtr func;
            std::string description;

            FuncHM(const FuncPtr & f, const std::string & d) : func(f), description(d) {}

            void Call(const View & src, const View & mask, uint8_t index, uint32_t * histogram) const
            {
                TEST_PERFORMANCE_TEST(description);
                func(src.data, src.stride, src.width, src.height, mask.data, mask.stride, index, histogram);
            }
        };


		struct FuncASDH
		{
			typedef void (*FuncPtr)(
				const uint8_t *src, size_t width, size_t height, size_t stride,
				size_t step, size_t indent, uint32_t * histogram);

			FuncPtr func;
			std::string description;

			FuncASDH(const FuncPtr & f, const std::string & d) : func(f), description(d) {}

			void Call(const View & src, size_t step, size_t indent, uint32_t * histogram) const
			{
				TEST_PERFORMANCE_TEST(description);
				func(src.data, src.width, src.height, src.stride,
					step, indent, histogram);
			}
		};
	}

#define FUNC_H(function) FuncH(function, #function)

#define FUNC_HM(function) FuncHM(function, #function)

#define FUNC_ASDH(function) FuncASDH(function, #function)

    bool HistogramAutoTest(int width, int height, const FuncH & f1, const FuncH & f2)
    {
        bool result = true;

        TEST_LOG_SS(Info, "Test " << f1.description << " & " << f2.description << " [" << width << ", " << height << "].");

        View s(int(width), int(height), View::Gray8, NULL, TEST_ALIGN(width));
        FillRandom(s);

        Histogram h1 = {0}, h2 = {0};

        TEST_EXECUTE_AT_LEAST_MIN_TIME(f1.Call(s, h1));

        TEST_EXECUTE_AT_LEAST_MIN_TIME(f2.Call(s, h2));

        result = result && Compare(h1, h2, 0, true, 32);

        return result;
    }

    bool HistogramAutoTest(const FuncH & f1, const FuncH & f2)
    {
        bool result = true;

        result = result && HistogramAutoTest(W, H, f1, f2);
        result = result && HistogramAutoTest(W + O, H - O, f1, f2);
        result = result && HistogramAutoTest(W - O, H + O, f1, f2);

        return result;
    }

    bool HistogramAutoTest()
    {
        bool result = true;

        result = result && HistogramAutoTest(FUNC_H(Simd::Base::Histogram), FUNC_H(SimdHistogram));

        return result;
    }

    bool HistogramMaskedAutoTest(int width, int height, const FuncHM & f1, const FuncHM & f2)
    {
        bool result = true;

        TEST_LOG_SS(Info, "Test " << f1.description << " & " << f2.description << " [" << width << ", " << height << "].");

        View s(int(width), int(height), View::Gray8, NULL, TEST_ALIGN(width));
        View m(int(width), int(height), View::Gray8, NULL, TEST_ALIGN(width));

        const uint8_t index = 77;
        FillRandom(s);
        FillRandomMask(m, index);

        Histogram h1 = {0}, h2 = {0};

        TEST_EXECUTE_AT_LEAST_MIN_TIME(f1.Call(s, m, index, h1));

        TEST_EXECUTE_AT_LEAST_MIN_TIME(f2.Call(s, m, index, h2));

        result = result && Compare(h1, h2, 0, true, 32);

        return result;
    }

    bool HistogramMaskedAutoTest(const FuncHM & f1, const FuncHM & f2)
    {
        bool result = true;

        result = result && HistogramMaskedAutoTest(W, H, f1, f2);
        result = result && HistogramMaskedAutoTest(W + O, H - O, f1, f2);
        result = result && HistogramMaskedAutoTest(W - O, H + O, f1, f2);

        return result;
    }

    bool HistogramMaskedAutoTest()
    {
        bool result = true;

        result = result && HistogramMaskedAutoTest(FUNC_HM(Simd::Base::HistogramMasked), FUNC_HM(SimdHistogramMasked));

#ifdef SIMD_SSE2_ENABLE
        if(Simd::Sse2::Enable)
            result = result && HistogramMaskedAutoTest(FUNC_HM(Simd::Sse2::HistogramMasked), FUNC_HM(SimdHistogramMasked));
#endif 

#ifdef SIMD_AVX2_ENABLE
        if(Simd::Avx2::Enable)
            result = result && HistogramMaskedAutoTest(FUNC_HM(Simd::Avx2::HistogramMasked), FUNC_HM(SimdHistogramMasked));
#endif 

#ifdef SIMD_VMX_ENABLE
        if(Simd::Vmx::Enable)
            result = result && HistogramMaskedAutoTest(FUNC_HM(Simd::Vmx::HistogramMasked), FUNC_HM(SimdHistogramMasked));
#endif 

        return result;
    }

	bool AbsSecondDerivativeHistogramAutoTest(int width, int height, int step, int indent, const FuncASDH & f1, const FuncASDH & f2)
	{
		bool result = true;

		TEST_LOG_SS(Info, "Test " << f1.description << " & " << f2.description << " [" << width << ", " << height << "] (" << step << ", " << indent << ").");

		View s(int(width), int(height), View::Gray8, NULL, TEST_ALIGN(width));
		FillRandom(s);

		Histogram h1 = {0}, h2 = {0};

		TEST_EXECUTE_AT_LEAST_MIN_TIME(f1.Call(s, step, indent, h1));

		TEST_EXECUTE_AT_LEAST_MIN_TIME(f2.Call(s, step, indent, h2));

		result = result && Compare(h1, h2, 0, true, 32);

		return result;
	}

    bool AbsSecondDerivativeHistogramAutoTest(const FuncASDH & f1, const FuncASDH & f2)
    {
        bool result = true;

        result = result && AbsSecondDerivativeHistogramAutoTest(W, H, 1, 16, f1, f2);
        result = result && AbsSecondDerivativeHistogramAutoTest(W + O, H - O, 2, 16, f1, f2);
        result = result && AbsSecondDerivativeHistogramAutoTest(W, H, 3, 8, f1, f2);
        result = result && AbsSecondDerivativeHistogramAutoTest(W - O, H + O, 4, 8, f1, f2);

        return result;
    }

	bool AbsSecondDerivativeHistogramAutoTest()
	{
		bool result = true;

		result = result && AbsSecondDerivativeHistogramAutoTest(FUNC_ASDH(Simd::Base::AbsSecondDerivativeHistogram), FUNC_ASDH(SimdAbsSecondDerivativeHistogram));

#ifdef SIMD_SSE2_ENABLE
        if(Simd::Sse2::Enable)
            result = result && AbsSecondDerivativeHistogramAutoTest(FUNC_ASDH(Simd::Sse2::AbsSecondDerivativeHistogram), FUNC_ASDH(SimdAbsSecondDerivativeHistogram));
#endif 

#ifdef SIMD_AVX2_ENABLE
        if(Simd::Avx2::Enable)
            result = result && AbsSecondDerivativeHistogramAutoTest(FUNC_ASDH(Simd::Avx2::AbsSecondDerivativeHistogram), FUNC_ASDH(SimdAbsSecondDerivativeHistogram));
#endif 

#ifdef SIMD_VMX_ENABLE
        if(Simd::Vmx::Enable)
            result = result && AbsSecondDerivativeHistogramAutoTest(FUNC_ASDH(Simd::Vmx::AbsSecondDerivativeHistogram), FUNC_ASDH(SimdAbsSecondDerivativeHistogram));
#endif 

		return result;
	}

    //-----------------------------------------------------------------------

    bool HistogramDataTest(bool create, int width, int height, const FuncH & f)
    {
        bool result = true;

        Data data(f.description);

        TEST_LOG_SS(Info, (create ? "Create" : "Verify") << " test " << f.description << " [" << width << ", " << height << "].");

        View src(width, height, View::Gray8, NULL, TEST_ALIGN(width));

        Histogram h1, h2;

        if(create)
        {
            FillRandom(src);

            TEST_SAVE(src);

            f.Call(src, h1);

            TEST_SAVE(h1);
        }
        else
        {
            TEST_LOAD(src);

            TEST_LOAD(h1);

            f.Call(src, h2);

            TEST_SAVE(h2);

            result = result && Compare(h1, h2, 0, true, 32);
        }

        return result;
    }

    bool HistogramDataTest(bool create)
    {
        bool result = true;

        result = result && HistogramDataTest(create, DW, DH, FUNC_H(SimdHistogram));

        return result;
    }

    bool HistogramMaskedDataTest(bool create, int width, int height, const FuncHM & f)
    {
        bool result = true;

        Data data(f.description);

        TEST_LOG_SS(Info, (create ? "Create" : "Verify") << " test " << f.description << " [" << width << ", " << height << "].");

        View src(width, height, View::Gray8, NULL, TEST_ALIGN(width));
        View mask(width, height, View::Gray8, NULL, TEST_ALIGN(width));

        const uint8_t index = 77;
        Histogram h1, h2;

        if(create)
        {
            FillRandom(src);
            FillRandomMask(mask, index);

            TEST_SAVE(src);
            TEST_SAVE(mask);

            f.Call(src, mask, index, h1);

            TEST_SAVE(h1);
        }
        else
        {
            TEST_LOAD(src);
            TEST_LOAD(mask);

            TEST_LOAD(h1);

            f.Call(src, mask, index, h2);

            TEST_SAVE(h2);

            result = result && Compare(h1, h2, 0, true, 32);
        }

        return result;
    }

    bool HistogramMaskedDataTest(bool create)
    {
        bool result = true;

        result = result && HistogramMaskedDataTest(create, DW, DH, FUNC_HM(SimdHistogramMasked));

        return result;
    }

    bool AbsSecondDerivativeHistogramDataTest(bool create, int width, int height, const FuncASDH & f)
    {
        bool result = true;

        Data data(f.description);

        TEST_LOG_SS(Info, (create ? "Create" : "Verify") << " test " << f.description << " [" << width << ", " << height << "].");

        View src(width, height, View::Gray8, NULL, TEST_ALIGN(width));

        size_t step = 1, indent = 16;
        Histogram h1, h2;

        if(create)
        {
            FillRandom(src);

            TEST_SAVE(src);

            f.Call(src, step, indent, h1);

            TEST_SAVE(h1);
        }
        else
        {
            TEST_LOAD(src);

            TEST_LOAD(h1);

            f.Call(src, step, indent, h2);

            TEST_SAVE(h2);

            result = result && Compare(h1, h2, 0, true, 32);
        }

        return result;
    }

    bool AbsSecondDerivativeHistogramDataTest(bool create)
    {
        bool result = true;

        result = result && AbsSecondDerivativeHistogramDataTest(create, DW, DH, FUNC_ASDH(SimdAbsSecondDerivativeHistogram));

        return result;
    }
}
