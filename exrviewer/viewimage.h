#pragma once

#include "stdafx.h"
#include <ImfArray.h>
#include <ImfHeader.h>
#include <ImathFun.h>
#include <ImathLimits.h>
#include "namespaceAlias.h"

#include "loadImage.h"
#include "scaleImage.h"
#include "userctrl.h"

struct DispRect
{
	int x, y, w, h;
};

enum IMAGE_CHANNEL_TYPE
{
	RGB_CHANNEL = 1,
	ALPHA_CHANNEL,
	DEPTH_CHANNEL,
};

enum VIEWER_CURSOR_TYPE
{
	CUR_ARROW = 0,
	CUR_HAND,
	CUR_CROSS,
};

enum VIEWER_OPTION_TYPE
{
	OPT_ACTUALSIZE = 1,
	OPT_IMAGECHANNEL,
};

struct VIEWER_OPTION
{
	bool bActualImageSize;
	IMAGE_CHANNEL_TYPE Channel;
};

struct EXR_IMAGE
{
	IMF::Header hdr;
	IMF::Array<IMF::Rgba>		pixels;
	IMF::Array<float*>			dataZ;
	IMF::Array<unsigned int>	sampleCount;
	bool deepComp;
	int zsize;
	int PartNum;
	int width;
	int height;
	int disp_w;
	int disp_h;
	int dx;
	int dy;
	float ratio;
	void *rgb;

	int Scale()
	{
		if (ratio > 1)
			scaleX(ratio, disp_w, disp_h, width, height, dx, dy, pixels);
		else
			scaleY(1 / ratio, disp_w, disp_h, width, height, dx, dy, pixels);
	}

	int Normalize()
	{
		normalizePixels(width, height, pixels);
	}

	int Mirror()
	{
		swapPixels(width, height, pixels);
	}
};

class ThreadRunner;
class CtlPanel;

class ImageViewer
{
#define THREAD_NUM	4
public:
	ImageViewer(HWND wnd) : m_hWnd(wnd), m_WndWidth(0), m_WndHeight(0), m_bImageLoaded(false),
		m_CurType(CUR_ARROW), m_gamma(1.0 / 2.2), m_exposure(-1), m_defog(0),
		m_kneeLow(0),m_kneeHigh(0),m_fogR(0),m_fogG(0),m_fogB(0)
	{
		//ZeroMemory(&m_img, sizeof(m_img));
		m_img.rgb = nullptr;
		m_Brush = CreateSolidBrush(RGB(255, 255, 255));
		ZeroMemory(&m_StretchRect, sizeof(DispRect));
		ZeroMemory(&m_ScrollRect, sizeof(DispRect));
		m_ScrollPosX = 0;
		m_ScrollPosY = 0;
		CreateThreads();
		m_Scroll.Init(m_hWnd);
		m_Scroll.Show(SB_BOTH, false);

		m_hHandCursor = LoadCursor(nullptr, IDC_HAND);
		m_hCrossCursor = LoadCursor(nullptr, IDC_CROSS);
		m_hArrowCursor = LoadCursor(nullptr, IDC_ARROW);

		InitOption();
	}
	virtual ~ImageViewer()
	{
		CloseImage();
		DeleteObject(m_Brush);
		DestroyThreads();
	}
	virtual void SetPanel(CtlPanel* panel) { m_pCtlPanel = panel; }

	virtual void InitOption();

	virtual inline void SetExposure(float v) { m_exposure = v; }
	virtual inline void SetDefog(float v) { m_defog = v; }
	virtual inline void SetKneeLow(float v) { m_kneeLow = v; }
	virtual inline void SetKneeHigh(float v) { m_kneeHigh = v; }
	virtual int SetOption(VIEWER_OPTION_TYPE type, int value = 0);
	virtual int GetOption(VIEWER_OPTION_TYPE type);
	virtual void SetMenuMan(MenuMan* m) { m_pMenuMan = m; }
	virtual void InvalidateRect(bool redraw = true)
	{
		::InvalidateRect(m_hWnd, nullptr, redraw);
	}
	virtual void ChangeCursor(VIEWER_CURSOR_TYPE cur)
	{
		if (m_CurType != cur)
		{
			m_CurType = cur;
			HCURSOR cc = (cur == CUR_HAND) ? m_hHandCursor : ((cur == CUR_CROSS) ? m_hCrossCursor : m_hArrowCursor);
			SetClassLongPtrW(m_hWnd, GCLP_HCURSOR, LONG_PTR(cc));
		}
	}
	virtual void Scroll(bool bHorz, int request, int pos);
	virtual void MouseScroll(bool bLButton, int x, int y);
	virtual bool UpdateDisplayRect();
	virtual void ShowPixelInfo(int xpos, int ypos);

	virtual void OpenImage(const wchar_t * filename);
	virtual void UpdateImage();
	virtual void DrawImage();
	virtual void ClearImage();
	virtual void CloseImage();
	virtual void CreateThreads();
	virtual void DestroyThreads();
	virtual void ResumeAllThreads();
	virtual void WaitAllThreads();
	virtual void RunThread(int index);

protected:
	virtual void AppendEXRPartIndexMenu();
	virtual void RemoveEXRPartIndexMenu();
	
	virtual int GetPartNum(const char* filename);
	virtual bool LoadEXR(const char* fileName, bool preview, int lx, int ly);
	//
	//  Dithering: Reducing the raw 16-bit pixel data to 8 bits for the
	//  OpenGL frame buffer can sometimes lead to contouring in smooth
	//  color ramps.  Dithering with a simple Bayer pattern eliminates
	//  visible contouring.
	//
	virtual unsigned char dither(float v, int x, int y)
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

	virtual void ComputeFogColor(IMF::Rgba* rp, int _dw, int _dh)
	{
		m_fogR = 0;
		m_fogG = 0;
		m_fogB = 0;

		for (int j = 0; j < _dw * _dh; ++j)
		{
			//const IMF::Rgba &rp = _rawPixels[j];

			if (rp->r.isFinite())
				m_fogR += rp->r;

			if (rp->g.isFinite())
				m_fogG += rp->g;

			if (rp->b.isFinite())
				m_fogB += rp->b;
		}

		m_fogR /= _dw * _dh;
		m_fogG /= _dw * _dh;
		m_fogB /= _dw * _dh;
	}
	virtual void UpdateScrollWnd();

	float m_GammaLut[THREAD_NUM][3/*rgb channel*/][1 << 16];
	ThreadRunner *m_Threads[THREAD_NUM];
	wchar_t m_szFileName[MAX_PATH];
	bool m_bImageLoaded;
	EXR_IMAGE m_img;
	HWND m_hWnd;
	int m_WndWidth;
	int m_WndHeight;
	MenuMan* m_pMenuMan;
	CtlPanel *m_pCtlPanel;

	DispRect m_StretchRect;
	DispRect m_ScrollRect;
	int m_ScrollPosX;
	int m_ScrollPosY;
	StandardScrollBar m_Scroll;

	HBRUSH m_Brush;
	HCURSOR m_hHandCursor;
	HCURSOR m_hArrowCursor;
	HCURSOR m_hCrossCursor;
	VIEWER_CURSOR_TYPE m_CurType;
	VIEWER_OPTION m_Option;

	float m_gamma;
	float m_exposure;
	float m_defog;
	float m_kneeLow;
	float m_kneeHigh;
	float m_fogR;
	float m_fogG;
	float m_fogB;
};
