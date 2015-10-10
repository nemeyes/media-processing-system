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
		struct Func
		{
			typedef void (*FuncPtr)(
				const uint8_t *src, size_t srcWidth, size_t srcHeight, size_t srcStride,
				 uint8_t *dst, size_t dstWidth, size_t dstHeight, size_t dstStride, size_t channelCount);

			FuncPtr func;
			std::string description;

			Func(const FuncPtr & f, const std::string & d) : func(f), description(d) {}

			void Call(const View & src, View & dst) const
			{
				TEST_PERFORMANCE_TEST(description);
				func(src.data, src.width, src.height, src.stride,
					dst.data, dst.width, dst.height, dst.stride, View::PixelSize(src.format));
			}
		};
	}

#define ARGS(format, width, height, k, function1, function2) \
    format, width, height, k, \
    Func(function1.func, function1.description + ColorDescription(format)), \
    Func(function2.func, function2.description + ColorDescription(format))

#define FUNC(function) \
    Func(function, std::string(#function))

	bool ResizeAutoTest(View::Format format, int width, int height, double k, const Func & f1, const Func & f2)
	{
		bool result = true;

		TEST_LOG_SS(Info, "Test " << f1.description << " & " << f2.description
			<< " [" << size_t(width*k) << ", " << size_t(height*k) << "] -> [" << width << ", " << height << "].");

		View s(size_t(width*k), size_t(height*k), format, NULL, TEST_ALIGN(size_t(k*width)));
		FillRandom(s);

		View d1(width, height, format, NULL, TEST_ALIGN(width));
		View d2(width, height, format, NULL, TEST_ALIGN(width));

		TEST_EXECUTE_AT_LEAST_MIN_TIME(f1.Call(s, d1));

		TEST_EXECUTE_AT_LEAST_MIN_TIME(f2.Call(s, d2));

		result = result && Compare(d1, d2, 0, true, 64);

		return result;
	}

    bool ResizeBilinearAutoTest(const Func & f1, const Func & f2)
    {
        bool result = true;

        for(View::Format format = View::Gray8; format <= View::Bgra32; format = View::Format(format + 1))
        {
            result = result && ResizeAutoTest(ARGS(format, W, H, 0.9, f1, f2));
            result = result && ResizeAutoTest(ARGS(format, W + O, H - O, 1.3, f1, f2));
            result = result && ResizeAutoTest(ARGS(format, W - O, H + O, 0.7, f1, f2));
        }

        return result;
    }

    bool ResizeBilinearAutoTest()
    {
        bool result = true;

        result = result && ResizeBilinearAutoTest(FUNC(Simd::Base::ResizeBilinear), FUNC(SimdResizeBilinear));

#ifdef SIMD_SSE2_ENABLE
        if(Simd::Sse2::Enable)
            result = result && ResizeBilinearAutoTest(FUNC(Simd::Sse2::ResizeBilinear), FUNC(SimdResizeBilinear));
#endif 

#ifdef SIMD_SSSE3_ENABLE
        if(Simd::Ssse3::Enable)
            result = result && ResizeBilinearAutoTest(FUNC(Simd::Ssse3::ResizeBilinear), FUNC(SimdResizeBilinear));
#endif 

#ifdef SIMD_AVX2_ENABLE
        if(Simd::Avx2::Enable)
            result = result && ResizeBilinearAutoTest(FUNC(Simd::Avx2::ResizeBilinear), FUNC(SimdResizeBilinear));
#endif 

#ifdef SIMD_VMX_ENABLE
        if(Simd::Vmx::Enable)
            result = result && ResizeBilinearAutoTest(FUNC(Simd::Vmx::ResizeBilinear), FUNC(SimdResizeBilinear));
#endif 

        return result;
    }

    //-----------------------------------------------------------------------

    bool ResizeBilinearDataTest(bool create, int width, int height, View::Format format, const Func & f)
    {
        bool result = true;

        Data data(f.description);

        TEST_LOG_SS(Info, (create ? "Create" : "Verify") << " test " << f.description << " [" << width << ", " << height << "].");


        const double k = 0.7;

        View s(size_t(width*k), size_t(height*k), format, NULL, TEST_ALIGN(size_t(k*width)));

        View d1(width, height, format, NULL, TEST_ALIGN(width));
        View d2(width, height, format, NULL, TEST_ALIGN(width));

        if(create)
        {
            FillRandom(s);
            TEST_SAVE(s);

            f.Call(s, d1);

            TEST_SAVE(d1);
        }
        else
        {
            TEST_LOAD(s);

            TEST_LOAD(d1);

            f.Call(s, d2);

            TEST_SAVE(d2);

            result = result && Compare(d1, d2, 0, true, 64);
        }

        return result;
    }

    bool ResizeBilinearDataTest(bool create)
    {
        bool result = true;

        Func f = FUNC(SimdResizeBilinear);
        for(View::Format format = View::Gray8; format <= View::Bgra32; format = View::Format(format + 1))
        {
            result = result && ResizeBilinearDataTest(create, DW, DH, format, Func(f.func, f.description + Data::Description(format)));
        }

        return result;
    }
}
