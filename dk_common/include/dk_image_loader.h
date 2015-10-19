#pragma once
#include <cmath>
#include <gdiplus.h>

#pragma comment(lib, "gdiplus.lib")

class dk_image_loader
{
public:
	Gdiplus::Bitmap * origin_bitmap;
	Gdiplus::Bitmap * bitmap;

	WORD bpp;
	WORD Bpp;
	BITMAPINFO * bmi;
	BITMAPINFOHEADER * bmih;
	HBITMAP dib; 
	BYTE * pixel_buffer;

public:
	dk_image_loader(void)
		: origin_bitmap(0)
		, bitmap(0)
		, bpp(32)
		, Bpp(bpp>>3)
		, bmi(0)
		, bmih(0)
		, pixel_buffer(0)
	{
		Gdiplus::GdiplusStartupInput gpsi;
		Gdiplus::GdiplusStartup(&_token, &gpsi, NULL);
	}

	dk_image_loader(LPCWSTR file)
		: origin_bitmap(0)
		, bitmap(0)
		, bpp(32)
		, Bpp(bpp >> 3)
		, bmi(0)
		, bmih(0)
		, pixel_buffer(0)
	{
		Gdiplus::GdiplusStartupInput gpsi;
		Gdiplus::GdiplusStartup(&_token, &gpsi, NULL);

		load(file);
	}

	virtual ~dk_image_loader(void)
	{
		empty();
		Gdiplus::GdiplusShutdown(_token);
	}

	void empty(void)
	{
		if (origin_bitmap)
		{
			delete origin_bitmap;
			origin_bitmap = 0;
		}
		if (bitmap)
		{
			delete bitmap;
			bitmap = 0;
		}
		if (bmi)
		{
			free(bmi);
			bmi = 0;
		}

	}

	bool load(LPCWSTR file)
	{
		empty();
		origin_bitmap = Gdiplus::Bitmap::FromFile(file);
		if (!origin_bitmap)
			return false;

		//bitmap = origin_bitmap;
		bitmap = origin_bitmap->Clone(0, 0, origin_bitmap->GetWidth(), origin_bitmap->GetHeight(), PixelFormat32bppARGB);
		if (bitmap->GetLastStatus() != Gdiplus::Ok)
			return false;

		int width = bitmap->GetWidth();
		int height =bitmap->GetHeight();
		int bpr/*BytePerRow*/ = (((width * (long)bpp + 31L) & (~31L)) / 8L);
		int size = height * bpr;
		bmi = (BITMAPINFO *)malloc(sizeof(BITMAPINFO) + size);
		if (bmi)
		{
			bmih = &bmi->bmiHeader;
			bmih->biSize = sizeof(BITMAPINFOHEADER);
			bmih->biWidth = width;
			bmih->biHeight = height;
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

			Gdiplus::BitmapData bitmap_data;
			Gdiplus::Rect rect(0, 0, width, height);

			//get the bitmap data
			if (Gdiplus::Ok == bitmap->LockBits(&rect, Gdiplus::ImageLockModeRead | Gdiplus::ImageLockModeWrite, bitmap->GetPixelFormat(), &bitmap_data))
			{
#if 1
				BYTE * src = (BYTE*)bitmap_data.Scan0;
				BYTE * dst = pixel_buffer + (bitmap_data.Height - 1)*std::abs(bitmap_data.Stride);
				for (unsigned int h = 0; h < bitmap_data.Height; h++)
				{
					memcpy(dst, src, std::abs(bitmap_data.Stride));
					src += std::abs(bitmap_data.Stride);
					dst -= std::abs(bitmap_data.Stride);
				}
#else
				//get the lenght of the bitmap data in bytes
				int length = bitmap_data.Height * std::abs(bitmap_data.Stride);
				memcpy(pixel_buffer, bitmap_data.Scan0, length);//copy it to an array of BYTEs
#endif
				bitmap->UnlockBits(&bitmap_data);
			}
		}
		return bitmap->GetLastStatus() == Gdiplus::Ok;
	}

	operator Gdiplus::Bitmap*() const			{ return bitmap; }

private:
	ULONG_PTR _token;
};