#pragma once

#include "stdafx.h"
#include <ImfArray.h>
#include <ImfHeader.h>
#include <ImathFun.h>
#include <ImathLimits.h>
#include <ImfThreading.h>
#include <IlmThread.h>
#include <IlmThreadSemaphore.h>

#include "loadImage.h"
#include "scaleImage.h"
#include "namespaceAlias.h"

#define max(a,b)            (((a) > (b)) ? (a) : (b))

inline float Knee(double x, double f);
inline float FindKneeF(float x, float y);

struct DispRect
{
	int x, y, w, h;
};

struct EXR_IMAGE
{
	IMF::Header hdr;
	IMF::Array<IMF::Rgba>		pixels;
	IMF::Array<float*>			dataZ;
	IMF::Array<unsigned int>	sampleCount;
	bool deepComp;
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

struct Gamma
{
	float g, m, d, kl, f, s;

	Gamma(float gamma,
		float exposure,
		float defog,
		float kneeLow,
		float kneeHigh)	:
		g(gamma),
		m(IMATH::Math<float>::pow(2, exposure + 2.47393)),
		d(defog),
		kl(IMATH::Math<float>::pow(2, kneeLow)),
		f(FindKneeF(IMATH::Math<float>::pow(2, kneeHigh) - kl,
			IMATH::Math<float>::pow(2, 3.5) - kl)),
		s(255.0 * IMATH::Math<float>::pow(2, -3.5 * g))
	{}

	float operator () (half h)
	{
		// Defog
		float x = max(0.f, (h - d));

		// Exposure
		x *= m;

		// Knee
		if (x > kl)
			x = kl + Knee(x - kl, f);

		// Gamma
		x = IMATH::Math<float>::pow(x, g);

		// Scale and clamp
		return IMATH_NAMESPACE::clamp(x * s, 0.f, 255.f);
	}

};

class ThreadRunner;

class ImageViewer
{
#define THREAD_NUM	4
public:
	ImageViewer(HWND wnd) : m_hWnd(wnd), m_WndWidth(0), m_WndHeight(0),
		m_bStretchDisplay(false), m_gamma(1.0 / 2.2), m_exposure(-1), m_defog(0),
		m_kneeLow(0),m_kneeHigh(0),m_fogR(0),m_fogG(0),m_fogB(0)
	{
		//ZeroMemory(&m_img, sizeof(m_img));
		m_img.rgb = nullptr;
		m_Brush = CreateSolidBrush(RGB(255, 255, 255));
		ZeroMemory(&m_StretchRect, sizeof(DispRect));
		ZeroMemory(&m_ScrollRect, sizeof(DispRect));
		CreateThreads();
	}
	virtual ~ImageViewer()
	{
		CloseImage();
		DeleteObject(m_Brush);
		DestroyThreads();
	}

	virtual inline void SetExposure(float v) { m_exposure = v; }
	virtual inline void SetDefog(float v) { m_defog = v; }
	virtual inline void SetKneeLow(float v) { m_kneeLow = v; }
	virtual inline void SetKneeHigh(float v) { m_kneeHigh = v; }

	virtual void InvalidateRect()
	{
		::InvalidateRect(m_hWnd, nullptr, true);
	}
	virtual DispRect* GetScrollRect() { return &m_ScrollRect; }
	virtual bool UpdateWndRect();
	virtual void OpenImage(const char * filename);
	virtual void UpdateImage();
	virtual void DrawImage();
	virtual void ClearImage();
	virtual void CloseImage()
	{
		SAFEDELETE(m_img.rgb);
	}

	virtual void CreateThreads();
	virtual void DestroyThreads();
	virtual void ResumeAllThreads();
	virtual void WaitAllThreads();
	virtual void RunThread(int index);

protected:

	virtual int GetPartNum(const char* filename);
	virtual void LoadEXR(
		const char fileName[],
		const char channel[],
		const char layer[],
		bool preview,
		int lx,
		int ly);
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

	ThreadRunner *m_Threads[THREAD_NUM];
	char m_szFileName[MAX_PATH];
	int m_zsize;
	EXR_IMAGE m_img;
	HWND m_hWnd;
	int m_WndWidth;
	int m_WndHeight;
	bool m_bStretchDisplay;
	DispRect m_StretchRect;
	DispRect m_ScrollRect;
	HBRUSH m_Brush;

	float m_gamma;
	float m_exposure;
	float m_defog;
	float m_kneeLow;
	float m_kneeHigh;
	float m_fogR;
	float m_fogG;
	float m_fogB;
};


class ThreadRunner : public IlmThread::Thread
{
public:
	ThreadRunner() : IlmThread::Thread() {}
	ThreadRunner(ImageViewer *viewer, int index) :
		IlmThread::Thread()
	{
		Init(viewer, index);
	}

	virtual ~ThreadRunner()
	{
		Close();
		m_Semaphore.wait();
		CloseHandle(m_hComputeEvent);
		CloseHandle(m_hResumeEvent);
	}
	virtual void Init(ImageViewer *viewer, int index)
	{
		m_Worker = viewer;
		m_Index = index;
		m_hComputeEvent = CreateEventW(nullptr, false/*auto - reset*/, false, nullptr);
		m_hResumeEvent = CreateEventW(nullptr, false/*auto - reset*/, false, nullptr);
		m_bExit = false;
		start();
	}
	virtual void run()
	{
		m_Semaphore.post();
		while (!m_bExit)
		{
			WaitForSingleObject(m_hResumeEvent, INFINITE);
			m_Worker->RunThread(m_Index);
			SetEvent(m_hComputeEvent);
		}
	}

	virtual void Close()
	{
		Resume();
		m_bExit = true;
	}

	virtual void Resume()
	{
		SetEvent(m_hResumeEvent);
	}
	virtual void WaitSync()
	{
		WaitForSingleObject(m_hComputeEvent, INFINITE);
	}
private:
	int m_Index;
	ImageViewer *m_Worker;
	bool m_bExit;
	HANDLE m_hComputeEvent;
	HANDLE m_hResumeEvent;
	IlmThread::Semaphore m_Semaphore;
};
