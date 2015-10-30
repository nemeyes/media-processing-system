#pragma once
#include <cmath>
#include <gdiplus.h>

#pragma comment(lib, "gdiplus.lib")

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
		Gdiplus::GdiplusStartupInput gpsi;
		Gdiplus::GdiplusStartup(&_token, &gpsi, NULL);
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
		Gdiplus::GdiplusStartupInput gpsi;
		Gdiplus::GdiplusStartup(&_token, &gpsi, NULL);
		load(width, height);
	}

	virtual ~dk_image_creator(void)
	{
		empty();
		Gdiplus::GdiplusShutdown(_token);
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

	bool save(LPTSTR filename)
	{
		Gdiplus::Bitmap *image = new Gdiplus::Bitmap(dib, NULL);

		CLSID clsid;
		int retVal = get_encoder_clsid(L"image/bmp", &clsid);
		image->Save(filename, &clsid, NULL);
		delete image;

		return true;
	}

private:
	int get_encoder_clsid(const WCHAR * format, CLSID * clsid)
	{
		UINT  num = 0;          // number of image encoders
		UINT  size = 0;         // size of the image encoder array in bytes

		Gdiplus::ImageCodecInfo * image_codec_info = NULL;

		Gdiplus::GetImageEncodersSize(&num, &size);
		if (size == 0)
			return -1;  // Failure

		image_codec_info = (Gdiplus::ImageCodecInfo*)(malloc(size));
		if (image_codec_info == NULL)
			return -1;  // Failure

		Gdiplus::GetImageEncoders(num, size, image_codec_info);

		for (UINT j = 0; j < num; ++j)
		{
			if (wcscmp(image_codec_info[j].MimeType, format) == 0)
			{
				*clsid = image_codec_info[j].Clsid;
				free(image_codec_info);
				return j;  // Success
			}
		}
		free(image_codec_info);
		return -1;  // Failure
	}

private:
	ULONG_PTR _token;

};