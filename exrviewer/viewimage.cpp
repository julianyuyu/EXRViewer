#include "viewimage.h"
#include <windows.h>

#include <halfFunction.h>
#include <ImathFun.h>
#include "winsync.h"
#include "stdafx.h"

#if 0
char CurrentFileName[256] = {};

void UpdateImage(HWND hWnd);
EXR_IMAGE img = {};
float _gamma = 1.0 / 2.2;
float _exposure = -1;
float _defog = 0;
float _kneeLow = 0;
float _kneeHigh = 5.0;
float _fogR = 0;
float _fogG = 0;
float _fogB = 0;

void
computeFogColor(IMF::Rgba* rp, int _dw, int _dh)
{
	_fogR = 0;
	_fogG = 0;
	_fogB = 0;

	for (int j = 0; j < _dw * _dh; ++j)
	{
		//const IMF::Rgba &rp = _rawPixels[j];

		if (rp->r.isFinite())
			_fogR += rp->r;

		if (rp->g.isFinite())
			_fogG += rp->g;

		if (rp->b.isFinite())
			_fogB += rp->b;
	}

	_fogR /= _dw * _dh;
	_fogG /= _dw * _dh;
	_fogB /= _dw * _dh;
}

float
knee(double x, double f)
{
	return float(IMATH::Math<double>::log(x * f + 1) / f);
}


float
findKneeF(float x, float y)
{
	float f0 = 0;
	float f1 = 1;

	while (knee(x, f1) > y)
	{
		f0 = f1;
		f1 = f1 * 2;
	}

	for (int i = 0; i < 30; ++i)
	{
		float f2 = (f0 + f1) / 2;
		float y2 = knee(x, f2);

		if (y2 < y)
			f1 = f2;
		else
			f0 = f2;
	}

	return (f0 + f1) / 2;
}

struct Gamma
{
	float g, m, d, kl, f, s;

	Gamma(float gamma,
		float exposure,
		float defog,
		float kneeLow,
		float kneeHigh);

	float operator () (half h);
};


Gamma::Gamma
(float gamma,
	float exposure,
	float defog,
	float kneeLow,
	float kneeHigh)
	:
	g(gamma),
	m(IMATH::Math<float>::pow(2, exposure + 2.47393)),
	d(defog),
	kl(IMATH::Math<float>::pow(2, kneeLow)),
	f(findKneeF(IMATH::Math<float>::pow(2, kneeHigh) - kl,
		IMATH::Math<float>::pow(2, 3.5) - kl)),
	s(255.0 * IMATH::Math<float>::pow(2, -3.5 * g))
{}


float Gamma::operator () (half h)
{
	//
	// Defog
	//

	float x = max(0.f, (h - d));

	//
	// Exposure
	//

	x *= m;

	//
	// Knee
	//

	if (x > kl)
		x = kl + knee(x - kl, f);

	//
	// Gamma
	//

	x = IMATH::Math<float>::pow(x, g);

	//
	// Scale and clamp
	//

	return IMATH_NAMESPACE::clamp(x * s, 0.f, 255.f);
}


//
//  Dithering: Reducing the raw 16-bit pixel data to 8 bits for the
//  OpenGL frame buffer can sometimes lead to contouring in smooth
//  color ramps.  Dithering with a simple Bayer pattern eliminates
//  visible contouring.
//

unsigned char
dither(float v, int x, int y)
{
	static const float d[4][4] =
	{
		{ 0.f / 16,  8.f / 16,  2.f / 16, 10.f / 16 },
		{ 12.f / 16,  4.f / 16, 14.f / 16,  6.f / 16 },
		{ 3.f / 16, 11.f / 16,  1.f / 16,  9.f / 16 },
		{ 15.f / 16,  7.f / 16, 13.f / 16,  5.f / 16 }
	};

	return (unsigned char)(v + d[y & 3][x & 3]);
}

void OnDraw(HWND hWnd)
{
	if (!img.rgb)
		return;

	HDC hdc = GetDC(hWnd);
	HDC hMemDC = CreateCompatibleDC(hdc);
	HBITMAP hBmp = CreateCompatibleBitmap(hdc, img.width, img.height);
	SelectObject(hMemDC, (HGDIOBJ)hBmp);

	BITMAPINFO bmpInfo;
	bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmpInfo.bmiHeader.biWidth = img.width;
	bmpInfo.bmiHeader.biHeight = -img.height;
	bmpInfo.bmiHeader.biPlanes = 1;
	bmpInfo.bmiHeader.biBitCount = 24;
	bmpInfo.bmiHeader.biCompression = BI_RGB;

	SetDIBits(hMemDC, hBmp, 0, abs(bmpInfo.bmiHeader.biHeight), (BYTE *)img.rgb, &bmpInfo, DIB_RGB_COLORS);

	BitBlt(hdc, 0, 0, bmpInfo.bmiHeader.biWidth, abs(bmpInfo.bmiHeader.biHeight), hMemDC, 0, 0, SRCCOPY);
	//RECT rect;
	//GetClientRect(hWnd, &rect);
	//StretchBlt(hdc, 0, 0, rect.right - rect.left, rect.bottom - rect.top, hMemDC, 0, 0, bmpInfo.bmiHeader.biWidth, abs(bmpInfo.bmiHeader.biHeight), SRCCOPY);

	DeleteDC(hMemDC);
	DeleteObject(hBmp);
	ReleaseDC(hWnd, hdc);
}

void OnClear(HWND hWnd)
{
	// clear();
	HDC hdc = GetDC(hWnd);
	HBRUSH brush = CreateSolidBrush(RGB(255, 255, 255));
	RECT rc;
	GetClientRect(hWnd, &rc);
	FillRect(hdc, &rc, brush);
	DeleteObject(brush);
	ReleaseDC(hWnd, hdc);
}

void OnClose(HWND hWnd)
{
	delete img.rgb;
}


static bool bthread_inited = false;
#define THREAD_NUM	16
struct THREAD_CTXT
{
	int index;
};

WinThread* threads;
HANDLE hevents[THREAD_NUM];
HANDLE handles[THREAD_NUM];
THREAD_CTXT ctxt[THREAD_NUM] = {};

DWORD WINAPI ThreadProc(LPVOID param)
{
	halfFunction<float>
		rGamma(Gamma(_gamma,
			_exposure,
			_defog * _fogR,
			_kneeLow,
			_kneeHigh),
			-HALF_MAX, HALF_MAX,
			0.f, 255.f, 0.f, 0.f);

	halfFunction<float>
		gGamma(Gamma(_gamma,
			_exposure,
			_defog * _fogG,
			_kneeLow,
			_kneeHigh),
			-HALF_MAX, HALF_MAX,
			0.f, 255.f, 0.f, 0.f);

	halfFunction<float>
		bGamma(Gamma(_gamma,
			_exposure,
			_defog * _fogB,
			_kneeLow,
			_kneeHigh),
			-HALF_MAX, HALF_MAX,
			0.f, 255.f, 0.f, 0.f);

	THREAD_CTXT* ctxt = (THREAD_CTXT *)param;
	int index = ctxt->index;
#if 0
	int x_index = index & 0x1;
	int y_index = (index >> 1) & 0x1;
	int ww = img.width / 2;
	int hh = img.height / 2;
	int x_start = x_index * ww;
	int y_start = y_index * hh;
	int x_end = x_start + ww;
	int y_end = y_start + hh;
#else
	int x_index = 0;
	int y_index = (index);
	int ww = img.width;
	int hh = img.height / THREAD_NUM;
	int x_start = x_index * ww;
	int y_start = y_index * hh;
	int x_end = x_start + ww;
	int y_end = y_start + hh;
#endif
	Imf::Rgba *pixel, *pixel0;

	BYTE *pRgbData00 = (BYTE*)img.rgb;
	BYTE *pRgbData0 = pRgbData00;
	BYTE *pRgbData = pRgbData0;

	for (int y = y_start; y < y_end && y<img.height; y++)
	{
		pixel0 = &(img.pixels[img.width * y]);
		pRgbData0 = pRgbData00+ img.width * y * 3;
		for (int x = x_start; x < x_end; x++)
		{
			pixel = pixel0 + x;
			pRgbData = pRgbData0 + x * 3;
			*pRgbData++ = (unsigned char)(dither(bGamma(pixel->b), x, y) /** 255.f*/);
			*pRgbData++ = (unsigned char)(dither(gGamma(pixel->g), x, y) /** 255.f*/);
			*pRgbData++ = (unsigned char)(dither(rGamma(pixel->r), x, y) /** 255.f*/);
		}
	}
	SetEvent(hevents[index]);
	return 0;
}

void CreateThreads()
{
	//if (bthread_inited)
	//	return;
	threads = new WinThread[THREAD_NUM];
	for (int i = 0; i < THREAD_NUM; i++)
	{
		hevents[i] = CreateEvent(
			NULL,   // default security attributes
			TRUE,  // auto-reset event object
			FALSE,  // initial state is nonsignaled
			NULL);  // unnamed object
		handles[i] = threads[i].GetHandle();
		ctxt[i].index = i;

		threads[i].Create(ThreadProc, &(ctxt[i]));
	}
	bthread_inited = true;
}

void DestroyThreads()
{
	bthread_inited = false;
	SAFEDELETEARRAY(threads);
	for (int i = 0; i < THREAD_NUM; i++)
	{
//		CloseHandle(hevents[i]);
	}
}

void OnLoad(HWND hWnd, const char * filename)
{
//	CreateThreads();
#if 0
	if (img.pixels)
		return;
#else
	if (!_stricmp(filename, CurrentFileName))
		return;

	if (img.pixels)
	{
		OnClear(hWnd);
		OnClose(hWnd);
	}
#endif
	int partIdx = 0;
	int zsize;
	//EXR_IMAGE img;
	LoadExrImage(filename, 0, 0, false, -1, -1, partIdx, zsize, &img);
	zsize = 1;

	img.rgb = new BYTE[img.width * img.height * 3];

#if 0
	HDC hdc = GetDC(hWnd);
	COLORREF cc;
	unsigned char r, g, b;
	Imf::Rgba *pixel, *pixel0;
	for (int y = 0; y < img.height; y++)
	{
		pixel0 = &(img.pixels[img.width * y]);
		for (int x = 0; x < img.width; x++)
		{
			pixel = pixel0 + x;
			//r = dither((pixel->r), x, y);
			//g = dither((pixel->g), x, y);
			//b = dither((pixel->b), x, y);
			//r = (pixel->r.bits())>>4;
			//g = (pixel->g.bits()) >> 4;
			//b = (pixel->b.bits()) >> 4;
			r = (unsigned char)((pixel->r) * 255.f);
			g = (unsigned char)((pixel->g) * 255.f);
			b = (unsigned char)((pixel->b) * 255.f);
			cc = RGB(r, g, b);
			//cc = RGB(pixel->r.bits, pixel->g, pixel->b);
			::SetPixel(hdc, x, y, cc);
		}
	}
	ReleaseDC(hWnd, hdc);
#else
	UpdateImage(hWnd);
#endif
}

void UpdateImage(HWND hWnd)
{
	halfFunction<float>
		rGamma(Gamma(_gamma,
			_exposure,
			_defog * _fogR,
			_kneeLow,
			_kneeHigh),
			-HALF_MAX, HALF_MAX,
			0.f, 255.f, 0.f, 0.f);

	halfFunction<float>
		gGamma(Gamma(_gamma,
			_exposure,
			_defog * _fogG,
			_kneeLow,
			_kneeHigh),
			-HALF_MAX, HALF_MAX,
			0.f, 255.f, 0.f, 0.f);

	halfFunction<float>
		bGamma(Gamma(_gamma,
			_exposure,
			_defog * _fogB,
			_kneeLow,
			_kneeHigh),
			-HALF_MAX, HALF_MAX,
			0.f, 255.f, 0.f, 0.f);


	if (!img.rgb)
		return;
	computeFogColor(img.pixels, img.width, img.height);
#if 0
	Imf::Rgba *pixel, *pixel0;

	BYTE *pRgbData = (BYTE*)img.rgb;

	for (int y = 0; y < img.height; y++)
	{
		pixel0 = &(img.pixels[img.width * y]);
		for (int x = 0; x < img.width; x++)
		{
			pixel = pixel0 + x;
#if 0
#if 0
			// simple test
			* pRgbData++ = (unsigned char)((pixel->b)* 255.f);
			*pRgbData++ = (unsigned char)((pixel->g)* 255.f);
			*pRgbData++ = (unsigned char)((pixel->r)* 255.f);
#else
			// dither only
			*pRgbData++ = (unsigned char)(dither((pixel->b), x, y) * 255.f);
			*pRgbData++ = (unsigned char)(dither((pixel->g), x, y) * 255.f);
			*pRgbData++ = (unsigned char)(dither((pixel->r), x, y) * 255.f);
#endif
#else
			*pRgbData++ = (unsigned char)(dither(bGamma(pixel->b), x, y) /** 255.f*/);
			*pRgbData++ = (unsigned char)(dither(gGamma(pixel->g), x, y) /** 255.f*/);
			*pRgbData++ = (unsigned char)(dither(rGamma(pixel->r), x, y) /** 255.f*/);
#endif
		}
	}
#else
	// multi-thread
	//for (int i=0;i<THREAD_NUM; i++)
	{
		//for (int y = 0; y < img.height; y++)
		//{
		//	pixel0 = &(img.pixels[img.width * y]);
		//	for (int x = 0; x < img.width; x++)
		//	{
		//		pixel = pixel0 + x;
		//		*pRgbData++ = (unsigned char)(dither(bGamma(pixel->b), x, y) /** 255.f*/);
		//		*pRgbData++ = (unsigned char)(dither(gGamma(pixel->g), x, y) /** 255.f*/);
		//		*pRgbData++ = (unsigned char)(dither(rGamma(pixel->r), x, y) /** 255.f*/);
		//	}
		//}
		CreateThreads();
		WaitForMultipleObjects(THREAD_NUM, /*handles*/hevents, TRUE, INFINITE);
		DestroyThreads();
	}
#endif

}

#endif
