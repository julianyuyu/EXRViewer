
#include "stdafx.h"
#include "viewimage.h"

#include <halfFunction.h>
#include <ImfStandardAttributes.h>
#include <ImfMultiPartInputFile.h>

float Knee(double x, double f)
{
	return float(IMATH::Math<double>::log(x * f + 1) / f);
}

float FindKneeF(float x, float y)
{
	float f0 = 0;
	float f1 = 1;

	while (Knee(x, f1) > y)
	{
		f0 = f1;
		f1 = f1 * 2;
	}

	for (int i = 0; i < 30; ++i)
	{
		float f2 = (f0 + f1) / 2;
		float y2 = Knee(x, f2);

		if (y2 < y)
			f1 = f2;
		else
			f0 = f2;
	}

	return (f0 + f1) / 2;
}

IMF::Chromaticities DisplayChromaticities()
{
	//
	// Get the chromaticities of the display's primaries and
	// white point from an environment variable.  If this fails,
	// assume chromaticities according to Rec. ITU-R BT.709.
	//

	static const char chromaticitiesEnv[] = "CTL_DISPLAY_CHROMATICITIES";
	IMF::Chromaticities c;  // default-initialized according to Rec. 709

	if (const char *chromaticities = getenv(chromaticitiesEnv))
	{
		IMF::Chromaticities tmp;

		int n = sscanf(chromaticities,
			" red %f %f green %f %f blue %f %f white %f %f",
			&tmp.red.x, &tmp.red.y,
			&tmp.green.x, &tmp.green.y,
			&tmp.blue.x, &tmp.blue.y,
			&tmp.white.x, &tmp.white.y);

		if (n == 8)
			c = tmp;
		else
			printf("Cannot parse environment variable , using default value "
				"(chromaticities according to Rec. ITU-R BT.709).");
	}
	else
	{
		printf("Environment variable is not set; using default value (chromaticities according "
			"to Rec. ITU-R BT.709).");
	}

	return c;
}

void AdjustChromaticities(const IMF::Header &header,
	const IMF::Array<IMF::Rgba> &inPixels,
	int w, int h,
	IMF::Array<IMF::Rgba> &outPixels)
{
	IMF::Chromaticities fileChroma;  // default-initialized according to Rec. 709

	if (hasChromaticities(header))
		fileChroma = chromaticities(header);

	IMF::Chromaticities displayChroma = DisplayChromaticities();

	if (fileChroma.red == displayChroma.red &&
		fileChroma.green == displayChroma.green &&
		fileChroma.blue == displayChroma.blue &&
		fileChroma.white == displayChroma.white)
	{
		return;
	}

	IMATH::M44f M = RGBtoXYZ(fileChroma, 1) * XYZtoRGB(displayChroma, 1);

	size_t numPixels = w * h;

	for (size_t i = 0; i < numPixels; ++i)
	{
		const IMF::Rgba &in = inPixels[i];
		IMF::Rgba &out = outPixels[i];

		IMATH::V3f rgb = IMATH::V3f(in.r, in.g, in.b) * M;

		out.r = rgb[0];
		out.g = rgb[1];
		out.b = rgb[2];
	}
}

void GetSize(IMF::Header* hdr, int& width, int& height, int& disp_w, int& disp_h, int& dx, int& dy, float& ratio)
{
	IMATH_NAMESPACE::Box2i drect = hdr->dataWindow();
	IMATH_NAMESPACE::Box2i disp = hdr->displayWindow();
	width = drect.max.x - drect.min.x + 1;
	height = drect.max.y - drect.min.y + 1;

	ratio = hdr->pixelAspectRatio();
	dx = drect.min.x - disp.min.x;
	dy = drect.min.y - disp.min.y;
	disp_w = disp.max.x - disp.min.x + 1;
	disp_h = disp.max.y - disp.min.y + 1;
}

void LoadExrImage(
	const char fileName[],
	const char channel[],
	const char layer[],
	bool preview,
	int lx,
	int ly,
	int PartIndex,
	int &zsize,
	EXR_IMAGE* image)
{
	if (image)
	{
		loadImage(fileName, channel, layer, preview, lx, ly, PartIndex, zsize, image->hdr, image->pixels, image->dataZ, image->sampleCount, image->deepComp);
		GetSize(&image->hdr, image->width, image->height, image->disp_w, image->disp_h, image->dx, image->dy, image->ratio);
	}
}

void ImageViewer::LoadEXR(
	const char fileName[],
	const char channel[],
	const char layer[],
	bool preview, int lx, int ly)
{
	int PartIndex = 0;
	m_img.PartNum = GetPartNum(fileName);
	LoadExrImage(fileName, channel, layer, preview, lx, ly, PartIndex, m_zsize, &m_img);
	//GetSize(&(m_img.hdr), m_img.width, m_img.height, m_img.disp_w, m_img.disp_h, m_img.dx, m_img.dy, m_img.ratio);
}

void ImageViewer::OpenImage(const char * filename)
{
	if (!_stricmp(filename, m_szFileName))
		return;

	strcpy_s(m_szFileName, MAX_PATH, filename);

	if (m_img.pixels)
	{
		ClearImage();
		CloseImage();
	}

	LoadEXR(filename, 0, 0, false, -1, -1);

	m_img.rgb = new BYTE[m_img.width * m_img.height * 3];

	UpdateImage();
	DrawImage();
}

void ImageViewer::UpdateImage()
{
	if (!m_img.rgb)
		return;
	ComputeFogColor(m_img.pixels, m_img.width, m_img.height);
	ResumeAllThreads();
	WaitAllThreads();
}

bool ImageViewer::UpdateWndRect()
{
	bool updated = false;
	RECT rc;
	::GetClientRect(m_hWnd, &rc);
	int ww = rc.right - rc.left;
	int wh = rc.bottom - rc.top;

	if (ww != m_WndWidth)
	{
		m_WndWidth = ww;
		updated = true;
	}
	if (wh != m_WndHeight)
	{
		m_WndHeight = wh;
		updated = true;
	}
	if (updated)
	{
		if (m_bStretchDisplay)
		{
			float r_wnd = (float)m_WndWidth / (float)m_WndHeight;
			float r_img = (float)m_img.width / (float)m_img.height;
			if (r_wnd >= r_img)
			{
				m_StretchRect.h = m_WndHeight;
				m_StretchRect.w = m_WndHeight * r_img;
				m_StretchRect.x = (m_WndWidth - m_StretchRect.w) / 2;
				m_StretchRect.y = 0;
			}
			else
			{
				m_StretchRect.h = (int)((float)m_WndWidth / r_img);
				m_StretchRect.w = m_WndWidth;
				m_StretchRect.x = 0;
				m_StretchRect.y = (m_WndHeight - m_StretchRect.h) / 2;
			}
		}
		else
		{
			// scroll display
			m_ScrollRect.w = m_WndWidth;
			m_ScrollRect.h = m_WndHeight;
		}
	}
	return updated;
}

void ImageViewer::DrawImage()
{
	if (!m_img.rgb)
		return;

	HDC hdc = GetDC(m_hWnd);
	HDC hMemDC = CreateCompatibleDC(hdc);
	HBITMAP hBmp = CreateCompatibleBitmap(hdc, m_img.width, m_img.height);
	SelectObject(hMemDC, (HGDIOBJ)hBmp);

	BITMAPINFO bmpInfo;
	bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmpInfo.bmiHeader.biWidth = m_img.width;
	bmpInfo.bmiHeader.biHeight = -m_img.height;
	bmpInfo.bmiHeader.biPlanes = 1;
	bmpInfo.bmiHeader.biBitCount = 24;
	bmpInfo.bmiHeader.biCompression = BI_RGB;

	SetDIBits(hMemDC, hBmp, 0, abs(bmpInfo.bmiHeader.biHeight), (BYTE *)m_img.rgb, &bmpInfo, DIB_RGB_COLORS);

	UpdateWndRect();
	if (m_bStretchDisplay)
	{
		SetStretchBltMode(hdc, COLORONCOLOR);
		StretchBlt(hdc, m_StretchRect.x, m_StretchRect.y, m_StretchRect.w, m_StretchRect.h,
			hMemDC, 0, 0, bmpInfo.bmiHeader.biWidth, abs(bmpInfo.bmiHeader.biHeight), SRCCOPY);
	}
	else
	{
		BitBlt(hdc, 0, 0, bmpInfo.bmiHeader.biWidth, abs(bmpInfo.bmiHeader.biHeight),
			hMemDC, m_ScrollRect.x, m_ScrollRect.y, SRCCOPY);
	}
	DeleteDC(hMemDC);
	DeleteObject(hBmp);
	ReleaseDC(m_hWnd, hdc);
}

void ImageViewer::ClearImage()
{
	HDC hdc = GetDC(m_hWnd);
	RECT rc;
	GetClientRect(m_hWnd, &rc);
	FillRect(hdc, &rc, m_Brush);
	ReleaseDC(m_hWnd, hdc);
}

void ImageViewer::RunThread(int index)
{
	if (!m_img.rgb)
		return;

	halfFunction<float> rGamma(Gamma(m_gamma, m_exposure, m_defog * m_fogR, m_kneeLow, m_kneeHigh),
		-HALF_MAX, HALF_MAX, 0.f, 255.f, 0.f, 0.f);

	halfFunction<float> gGamma(Gamma(m_gamma, m_exposure, m_defog * m_fogG, m_kneeLow, m_kneeHigh),
		-HALF_MAX, HALF_MAX, 0.f, 255.f, 0.f, 0.f);

	halfFunction<float> bGamma(Gamma(m_gamma, m_exposure, m_defog * m_fogB, m_kneeLow, m_kneeHigh),
		-HALF_MAX, HALF_MAX, 0.f, 255.f, 0.f, 0.f);

	int x_index = 0;
	int y_index = (index);
	int ww = m_img.width;
	int hh = m_img.height / THREAD_NUM;
	int x_start = x_index * ww;
	int y_start = y_index * hh;
	int x_end = x_start + ww;
	int y_end = y_start + hh;
	Imf::Rgba *pixel, *pixel0;
	BYTE *pRgbData00 = (BYTE*)m_img.rgb;
	BYTE *pRgbData0 = pRgbData00;
	BYTE *pRgbData = pRgbData0;

	for (int y = y_start; y < y_end && y < m_img.height; y++)
	{
		pixel0 = &(m_img.pixels[m_img.width * y]);
		pRgbData0 = pRgbData00 + m_img.width * y * 3;
		for (int x = x_start; x < x_end; x++)
		{
			pixel = pixel0 + x;
			pRgbData = pRgbData0 + x * 3;
			*pRgbData++ = (unsigned char)(dither(bGamma(pixel->b), x, y) /** 255.f*/);
			*pRgbData++ = (unsigned char)(dither(gGamma(pixel->g), x, y) /** 255.f*/);
			*pRgbData++ = (unsigned char)(dither(rGamma(pixel->r), x, y) /** 255.f*/);
		}
	}
}

int ImageViewer::GetPartNum(const char* filename)
{
	int numparts = 0;

	try
	{
		IMF::MultiPartInputFile *infile = new IMF::MultiPartInputFile(filename);
		numparts = infile->parts();
		delete infile;
	}
	catch (IEX_NAMESPACE::BaseExc &e)
	{
		printf("error: %s", e.what());
		return 0;
	}
	return numparts;
}

void ImageViewer::CreateThreads()
{
	for (int i = 0; i < THREAD_NUM; i++)
	{
		m_Threads[i] = new ThreadRunner(this, i);
	}
}

void ImageViewer::DestroyThreads()
{
	for (int i = 0; i < THREAD_NUM; i++)
	{
		SAFEDELETE(m_Threads[i]);
	}
}

void ImageViewer::ResumeAllThreads()
{
	for (int i = 0; i < THREAD_NUM; i++)
	{
		m_Threads[i]->Resume();
	}
}

void ImageViewer::WaitAllThreads()
{
	for (int i = 0; i < THREAD_NUM; i++)
	{
		m_Threads[i]->WaitSync();
	}
}
