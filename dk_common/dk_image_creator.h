#pragma once
#include <cmath>

class dk_image_creator
{
public:
	int width;
	int height;
	WORD bpp;
	WORD Bpp;
	BITMAPINFO * bmi;
	BITMAPINFOHEADER * bmih;
	HBITMAP dib;
	BYTE * pixel_buffer;

public:
	dk_image_creator(void)
		: bpp(32)
		, Bpp(bpp >> 3)
		, bmi(0)
		, bmih(0)
		, pixel_buffer(0)
	{
	}

	dk_image_creator(int width, int height)
		: width(width)
		, height(height)
		, bpp(32)
		, Bpp(bpp >> 3)
		, bmi(0)
		, bmih(0)
		, pixel_buffer(0)
	{
		load(width, height);
	}

	virtual ~dk_image_creator(void)
	{
		empty();
	}

	void empty(void)
	{
		if (bmi)
		{
			free(bmi);
			bmi = 0;
		}
	}

	bool load(int width, int height)
	{
		empty();

		int bpr/*BytePerRow*/ = (((width * (long)bpp + 31L) & (~31L)) / 8L);
		int size = height * bpr;
		bmi = (BITMAPINFO *)malloc(sizeof(BITMAPINFO) + size);
		if (bmi)
		{
			bmih = &bmi->bmiHeader;
			bmih->biSize = sizeof(BITMAPINFOHEADER);
			bmih->biWidth = width;
			bmih->biHeight = -height;
			bmih->biPlanes = 1;
			bmih->biBitCount = bpp;
			bmih->biCompression = BI_RGB;
			bmih->biSizeImage = size;
			bmih->biXPelsPerMeter = 0;
			bmih->biYPelsPerMeter = 0;
			bmih->biClrUsed = 0;
			bmih->biClrImportant = 0;
			bmih->biSizeImage = width * height * Bpp;

			dib = CreateDIBSection(NULL, bmi, DIB_RGB_COLORS, (void **)&pixel_buffer, NULL, 0);
		}
		return true;
	}

};