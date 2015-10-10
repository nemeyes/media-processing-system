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
#ifndef __TestUtils_h__
#define __TestUtils_h__

#include "Test/TestLog.h"

namespace Test
{
	SIMD_INLINE int Random(int range)
	{
		return ((::rand()&INT16_MAX)*range)/INT16_MAX;
	}

    SIMD_INLINE double Random()
    {
        return ((::rand()&INT16_MAX)*1.0)/INT16_MAX;
    }

    void FillRandom(View & view, uint8_t lo = 0, uint8_t hi = 255);


	void FillRandomMask(View & view, uint8_t index);

    void FillRhombMask(View & mask, const Rect & rect, uint8_t index);
    
    void FillRandom32f(View & view, float lo = 0, float hi = 4096.0f);

    bool Compare(const View & a, const View & b, 
		int differenceMax = 0, bool printError = false, int errorCountMax = 0, int valueCycle = 0, 
		const std::string & description = "");

	bool Compare(const Histogram a, const Histogram b, 
		int differenceMax = 0, bool printError = false, int errorCountMax = 0);

    bool Compare(const Sums & a, const Sums & b, 
        int differenceMax = 0, bool printError = false, int errorCountMax = 0);

    bool Compare(const Sums64 & a, const Sums64 & b, 
        int differenceMax = 0, bool printError = false, int errorCountMax = 0);

    bool Compare(const Rect & a, const Rect & b, bool printError = false);

    bool Compare(const Buffer32f & a, const Buffer32f & b, float relativeDifferenceMax = EPS, bool printError = false, int errorCountMax = 0);

    bool Compare(const float & a, const float & b, float relativeDifferenceMax = EPS, bool printError = false);

	std::string ColorDescription(View::Format format);

    std::string FormatDescription(View::Format format);

    std::string ScaleDescription(const Point & scale);

    std::string CompareTypeDescription(SimdCompareType type);

    std::string ExpandToLeft(const std::string & value, size_t count);
    std::string ExpandToRight(const std::string & value, size_t count);

    std::string ToString(double value, size_t iCount, size_t fCount);
}

#define TEST_CHECK_VALUE(name) \
    if(name##1 != name##2) \
    { \
        TEST_LOG_SS(Error, "Error " << #name << ": (" << name##1  << " != " << name##2 << ")! "); \
        return false; \
    } 

#endif//__TestUtils_h__