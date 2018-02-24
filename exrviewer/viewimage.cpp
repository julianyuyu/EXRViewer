
#include "stdafx.h"
#include "viewimage.h"
#include "panel.h"
#include "threadrunner.h"

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

bool LoadExrImage(
	const char fileName[],
	const char channel[],
	const char layer[],
	bool preview,
	int lx,
	int ly,
	int PartIndex,
	EXR_IMAGE* image)
{
	if (image && fileName)
	{
		if (_stricmp(fileName, ""))
		{
			loadImage(fileName, channel, layer, preview, lx, ly, PartIndex, image->zsize, image->hdr, image->pixels, image->dataZ, image->sampleCount, image->deepComp);
			GetSize(&image->hdr, image->width, image->height, image->disp_w, image->disp_h, image->dx, image->dy, image->ratio);
			return true;
		}
	}
	return false;
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

bool ImageViewer::LoadEXR(const char* fileName, bool preview, int lx, int ly)
{
	const char channel_A[2] = "A";
	const char channel_Z[2] = "Z";

	const char* channel = nullptr;
	const char* layer = nullptr;
	if (m_Option.Channel == ALPHA_CHANNEL)
		channel = channel_A;
	else if (m_Option.Channel == DEPTH_CHANNEL)
		channel = channel_Z;

	m_img.PartNum = GetPartNum(fileName);
	return LoadExrImage(fileName, channel, layer, preview, lx, ly, 0/*load 1st part this time*/, &m_img);
}

void ImageViewer::OpenImage(const wchar_t * filename)
{
	if (!_wcsicmp(filename, m_szFileName))
		return;

	wcscpy_s(m_szFileName, MAX_PATH, filename);

	char ansiname[MAX_PATH] = {};
	
	size_t i;
	wcstombs_s(&i, ansiname, MAX_PATH, filename, MAX_PATH);
	if (i != (wcslen(filename) + 1/* null terminated */))
	{
		return;
	}

	if (m_img.pixels)
	{
		ClearImage();
		CloseImage();
	}

	if (LoadEXR(ansiname, false, -1, -1))
	{
		m_bImageLoaded = true;
		SAFEDELETE(m_img.rgb);
		m_img.rgb = new BYTE[m_img.width * m_img.height * 3];

		AppendEXRPartIndexMenu();

		UpdateImage();
		UpdateDisplayRect();
		DrawImage();
	}
}

void ImageViewer::UpdateImage()
{
	if (!m_img.rgb)
		return;
	ComputeFogColor(m_img.pixels, m_img.width, m_img.height);
	ResumeAllThreads();
	WaitAllThreads();
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

	//UpdateDisplayRect();
	if (!m_Option.bActualImageSize)
	{
		SetStretchBltMode(hdc, COLORONCOLOR);
		StretchBlt(hdc, m_StretchRect.x, m_StretchRect.y, m_StretchRect.w, m_StretchRect.h,
			hMemDC, 0, 0, bmpInfo.bmiHeader.biWidth, abs(bmpInfo.bmiHeader.biHeight), SRCCOPY);
	}
	else
	{
		BitBlt(hdc, m_ScrollRect.x, m_ScrollRect.y, m_ScrollRect.w, m_ScrollRect.h, 
			hMemDC, m_ScrollPosX, m_ScrollPosY, SRCCOPY);
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

void ImageViewer::CloseImage()
{
	m_bImageLoaded = false;

	RemoveEXRPartIndexMenu();

	if (m_pCtlPanel)
		m_pCtlPanel->SetCoordAndColorInfo(0,0,0,0,0);

	m_szFileName[0] = L'\0';
	SAFEDELETE(m_img.rgb);
	m_Scroll.Show(SB_BOTH, false);
}

void ImageViewer::InitOption()
{
	ZeroMemory(&m_Option, sizeof(VIEWER_OPTION));
	m_Option.Channel = RGB_CHANNEL;
	m_Option.bActualImageSize = true;
}

int ImageViewer::SetOption(VIEWER_OPTION_TYPE type, int value/*=0*/)
{
	switch (type)
	{
	case OPT_ACTUALSIZE:
		m_Option.bActualImageSize = (bool)value;
		UpdateDisplayRect();
		DrawImage();
		break;
	default:
		break;
	}
	return 0;
}

int ImageViewer::GetOption(VIEWER_OPTION_TYPE type)
{
	switch (type)
	{
	case OPT_ACTUALSIZE:
		return m_Option.bActualImageSize ? 1 : 0;
		break;
	default:
		break;
	}
	return 0;
}