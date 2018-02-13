#pragma once

#include "userctrl.h"
#include "viewimage.h"

//
// Image display window class. 
//
class ImageWnd : public BasicWnd
{
public:
	ImageWnd(int x, int y, int w, int h, HWND hParent = nullptr, HINSTANCE hInstance = nullptr, DWORD style = 0, DWORD styleEx = 0, bool bShow = true)
		: BasicWnd(x, y, w, h, hParent, hInstance, style, styleEx, bShow), m_pViewer(nullptr)
	{
		SetOwnProc(m_hWnd);
	}
	virtual void SetViewer(ImageViewer* v) { m_pViewer = v; }
	virtual int OnMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
private:
	ImageViewer *m_pViewer;
};

//
// struct for manage image display window. 
//
class ImgPanel
{
public:
	ImgPanel(int x, int y, int w, int h, HWND hParent = nullptr, HINSTANCE hInst = nullptr) :
		m_pImgWnd(nullptr)
	{
		m_pImgWnd = new ImageWnd(x, y, w, h, hParent, hInst, 
			WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL/* | WS_BORDER*/);
		m_hImgWnd = m_pImgWnd->GetHWND();
	}
	virtual void SetViewer(ImageViewer* v) { m_pImgWnd->SetViewer(v); }
	virtual HWND GetHWND() { return m_hImgWnd; }
	virtual ~ImgPanel()
	{
		SAFEDELETE(m_pImgWnd);
	}
	virtual int GetHeight()
	{
		int x, y;
		m_pImgWnd->GetSize(x, y);
		return y;
	}
	virtual int GetWidth()
	{
		int x, y;
		m_pImgWnd->GetSize(x, y);
		return x;
	}
	virtual void Move(int x, int y, int w, int h, bool repaint = false)
	{
		int h_entire = h;
		m_pImgWnd->Move(x, y, w, h_entire, repaint);
	}
private:
	ImageWnd *m_pImgWnd;
	HWND m_hImgWnd;
};

//
// struct for manage control window and UI controls. 
//
class CtlPanel
{
#define TWOLINE_WIDTH_THRESHOLD		500
public:
	CtlPanel(int x, int y, int w, int h, HWND hParent = nullptr, HINSTANCE hInst = nullptr) :
		m_pCtlWnd(nullptr), m_pViewer(nullptr),
		m_pSldExposure(nullptr), m_pSldDefog(nullptr), m_pSldKneeLow(nullptr), m_pSldKneeHigh(nullptr)
	{
		m_hParent = hParent;
		m_pCtlWnd = new BasicWnd(x, y, w, h, hParent, hInst, WS_CHILD | WS_VISIBLE/* | WS_BORDER*/);
		m_hCtlWnd = m_pCtlWnd->GetHWND();
		CreateCtrls(x, y, w, h, hParent, hInst);
	}
#if 1
	virtual void ChangetoToolWindow()
	{
		SetWindowLong(m_hCtlWnd, GWL_STYLE, GetWindowLong(m_hCtlWnd, GWL_STYLE) | WS_CHILD);
		SetWindowLong(m_hCtlWnd, GWL_EXSTYLE, GetWindowLong(m_hCtlWnd, GWL_EXSTYLE) | WS_EX_LAYERED | WS_EX_TOOLWINDOW);

		COLORREF crKey = 0;
		BYTE     bAlpha = 120;
		SetLayeredWindowAttributes(m_hCtlWnd, crKey, bAlpha, LWA_ALPHA);
	}
#endif
	virtual void SetViewer(ImageViewer* v) { m_pViewer = v; }
	virtual HWND GetHWND() { return m_hCtlWnd; }
	virtual ~CtlPanel()
	{
		//SAFEDELETE(m_pSldGamma);
		SAFEDELETE(m_pSldExposure);
		SAFEDELETE(m_pSldDefog);
		SAFEDELETE(m_pSldKneeLow);
		SAFEDELETE(m_pSldKneeHigh);
		SAFEDELETE(m_pCtlWnd);
	}
	virtual int GetHeight()
	{
		int x, y;
		m_pCtlWnd->GetSize(x, y);
		return y;
	}
	virtual int GetWidth()
	{
		int x, y;
		m_pCtlWnd->GetSize(x, y);
		return x;
	}
	virtual void Move(int x, int y, int w, int h, bool repaint = false);
private:
	static void OnSliding(HWND hSlider, ULONG_PTR arg)
	{
		CtlPanel *thisPtr = (CtlPanel *)arg;
		thisPtr->Sliding(hSlider);
	}
public:
	virtual void Sliding(HWND hSlider);
private:
	virtual void CreateCtrls(int x, int y, int w, int h, HWND hParent, HINSTANCE hInst);
	BasicWnd *m_pCtlWnd;
	HWND m_hCtlWnd;
	HWND m_hParent;
	//SliderSet *m_pSldGamma;
	SliderSet *m_pSldExposure;
	SliderSet *m_pSldDefog;
	SliderSet *m_pSldKneeLow;
	SliderSet *m_pSldKneeHigh;

	ImageViewer *m_pViewer;
};