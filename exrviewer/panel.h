#pragma once


#include "stdafx.h"
#include "userctrl.h"
#include "viewimage.h"
//void OnDraw(HWND hWnd);
//extern float _exposure;
//extern float _defog;
//extern float _kneeLow;
//extern float _kneeHigh;

//void UpdateImage(HWND hWnd);
//extern HWND hImageWnd;

class ImageWnd : public BasicWnd
{
public:
	ImageWnd(int x, int y, int w, int h, HWND hParent = nullptr, HINSTANCE hInstance = nullptr, DWORD style = 0, DWORD styleEx = 0, bool bShow = true)
		: BasicWnd(x, y, w, h, hParent, hInstance, style, styleEx, bShow), m_pViewer(nullptr)
	{
		SetOwnProc(m_hWnd);
	}
	virtual void SetViewer(ImageViewer* v) { m_pViewer = v; }
	virtual int OnMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		switch (message)
		{
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);
			// TODO: Add any drawing code that uses hdc here...
			EndPaint(hWnd, &ps);
			if (m_pViewer)
				m_pViewer->DrawImage();
		}
			break;
		}
		return 0;
	}
private:
	ImageViewer *m_pViewer;
};

class ImgPanel
{
public:
	ImgPanel(int x, int y, int w, int h, /*WNDPROC proc = nullptr, */HWND hParent = nullptr, HINSTANCE hInst = nullptr) :
		m_pImgWnd(nullptr)
	{
		m_pImgWnd = new ImageWnd(x, y, w, h, hParent, hInst, WS_CHILD | WS_VISIBLE/* | WS_BORDER*/);
		//if (proc)
		//	return; // not needed ext proc.
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

class CtlPanel
{
public:
	CtlPanel(int x, int y, int w, int h, /*WNDPROC proc = nullptr, */HWND hParent = nullptr, HINSTANCE hInst = nullptr) :
		m_pCtlWnd(nullptr), m_pViewer(nullptr)
	{
		m_pCtlWnd = new BasicWnd(x, y, w, h, hParent, hInst, WS_CHILD | WS_VISIBLE/* | WS_BORDER*/);
		//if (proc)
		//	return; // not needed ext proc.
		m_hCtlWnd = m_pCtlWnd->GetHWND();
		int ww, hh;
		m_pCtlWnd->GetSize(ww, hh);
		CreateCtrls(x, y, ww, hh, hParent, hInst);
	}
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
	virtual void Move(int x, int y, int w, int h, bool repaint = false)
	{
		//m_pColorPanel->Move(x, y, w, h, repaint);

		//int x_list = 3;
		//int y_list = 3;
		//int w_list = 80;
		//int h_list = 72;
		//m_pColorList->Move(x_list, y_list, w_list, h_list, false);

		//int x_chk = x_list + w_list + 10;
		//int y_chk = y_list + 2;
		//m_pChk1->Move(x_chk, y_chk, false);
		//m_pChk2->Move(x_chk, y_chk2, false);

		//int x_grp = 4, y_grp = 84;
		//int w_grp = w - offset_2;
		//m_pGrp->Move(x_grp, y_grp, w_grp, h_grp, false);
		//int w_sldr = w_grp - offset_2 * 2;
//

		int offset = 4;
		int offset_2 = 8 + SLIDER_HEIGHT;
		int x_sldr = offset, y_sldr = offset;
		int w_sldr = w - offset * 2;
		m_pSldExposure->Move(x_sldr, y_sldr, w_sldr);
		y_sldr += offset_2;
		m_pSldDefog->Move(x_sldr, y_sldr, w_sldr);
		y_sldr += offset_2;
		m_pSldKneeLow->Move(x_sldr, y_sldr, w_sldr);
		y_sldr += offset_2;
		m_pSldKneeHigh->Move(x_sldr, y_sldr, w_sldr);

		int h_entire = h;
		m_pCtlWnd->Move(x, y, w, h_entire, repaint);
	}
private:
	static void OnSliding(HWND hSlider, ULONG_PTR arg)
	{
		CtlPanel *thisPtr = (CtlPanel *)arg;
		thisPtr->Sliding(hSlider);
	}
public:
	virtual void Sliding(HWND hSlider)
	{
		bool update = false;
		if (hSlider == m_pSldExposure->GetBarHWND())
		{
			//_exposure = m_pSldExposure->CurValue();
			if (m_pViewer)
			m_pViewer->SetExposure(m_pSldExposure->CurValue());
			update = true;
		}
		else if (hSlider == m_pSldDefog->GetBarHWND())
		{
			//_defog = m_pSldDefog->CurValue();
			if (m_pViewer)
			m_pViewer->SetDefog(m_pSldDefog->CurValue());
			update = true;
		}
		else if (hSlider == m_pSldKneeLow->GetBarHWND())
		{
			//_kneeLow = m_pSldKneeLow->CurValue();
			if (m_pViewer)
			m_pViewer->SetKneeLow(m_pSldKneeLow->CurValue());
			update = true;
		}
		else if (hSlider == m_pSldKneeHigh->GetBarHWND())
		{
			//_kneeHigh = m_pSldKneeHigh->CurValue();
			if (m_pViewer)
			m_pViewer->SetKneeHigh(m_pSldKneeHigh->CurValue());
			update = true;
		}

		if (update)
		{
			//UpdateImage(hImageWnd);
			//OnDraw(hImageWnd);
			if (m_pViewer)
			{
				m_pViewer->UpdateImage();
				//m_pViewer->DrawImage();
				m_pViewer->InvalidateRect();
			}
		}
	}
private:
	virtual void CreateCtrls(int x, int y, int w, int h, HWND hParent, HINSTANCE hInst)
	{
		//m_pTrack = new UserWnd(TRACKBAR_CLASS, L"Image Slice2", 2, 180, 160, 30, m_hColorWnd, hInst, WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS | TBS_ENABLESELRANGE, 0, 100);
		//m_pTrack = new SliderSet(2, 180, 160, 30, m_hColorWnd, hInst);

		int offset = 4;
		int offset_2 = 8 + SLIDER_HEIGHT;
		int x_sldr = offset, y_sldr = offset;
		int w_sldr = w - offset* 2;
		m_pSldExposure = new SliderSet(x_sldr, y_sldr, w_sldr,/* 0, */L"Exposure", 60, m_hCtlWnd, hInst);
		m_pSldExposure->InitValue(-10, 10, 0.1, -2);
		m_pSldExposure->SetCallback(OnSliding, (ULONG_PTR)this);
		y_sldr += offset_2;
		m_pSldDefog = new SliderSet(x_sldr, y_sldr, w_sldr,/* 0, */L"Defog", 60, m_hCtlWnd, hInst);
		m_pSldDefog->InitValue(0, 0.01, 0.0001, 0);
		m_pSldDefog->SetCallback(OnSliding, (ULONG_PTR)this);
		y_sldr += offset_2;
		m_pSldKneeLow = new SliderSet(x_sldr, y_sldr, w_sldr,/* 0, */L"Knee Low", 60, m_hCtlWnd, hInst);
		m_pSldKneeLow->InitValue(-3, 3, 0.1, 0);
		m_pSldKneeLow->SetCallback(OnSliding, (ULONG_PTR)this);
		y_sldr += offset_2;
		m_pSldKneeHigh = new SliderSet(x_sldr, y_sldr, w_sldr,/* 0, */L"Knee High", 60, m_hCtlWnd, hInst);
		m_pSldKneeHigh->InitValue(3.5, 7.5, 0.1, 5);
		m_pSldKneeHigh->SetCallback(OnSliding, (ULONG_PTR)this);

		//HWND hwndTrack = m_pTrack->GetBarHWND();
		Move(x, y, w, h, true);
	}
	BasicWnd *m_pCtlWnd;
	HWND m_hCtlWnd;
	//SliderSet *m_pSldGamma;
	SliderSet *m_pSldExposure;
	SliderSet *m_pSldDefog;
	SliderSet *m_pSldKneeLow;
	SliderSet *m_pSldKneeHigh;

	ImageViewer *m_pViewer;
};