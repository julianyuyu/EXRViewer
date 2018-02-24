
#include "stdafx.h"
#include "panel.h"

int ImageWnd::OnMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_MOUSEMOVE:
		if (m_pViewer)
			m_pViewer->MouseScroll((wParam == MK_LBUTTON), GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		break;
	case WM_HSCROLL:
	case WM_VSCROLL:
		if (m_pViewer)
			m_pViewer->Scroll((message == WM_HSCROLL), LOWORD(wParam), HIWORD(wParam));
		break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		if (m_pViewer)
		{
			m_pViewer->UpdateDisplayRect();
			m_pViewer->DrawImage();
		}
	}
	break;
	}
	return 0;
}

#if 0
void CalcCtrlsSize(HWND hParent, int x, int y, int& w, int& h,
	int& x1, int& x2, int& x3, int& x4, 
	int& y1, int& y2, int& y3, int& y4,
	int& w1, int& w2, int& w3, int& w4)
#endif
void CtlPanel::CalcCtrlsSize(int x, int y, int& w, int& h,
		DispRect& r1, DispRect& r2, DispRect& r3, DispRect& r4,/* 4 sliders*/
		DispRect& r5, DispRect& r6 /*2 editboxs*/)
{
	int offset = 4;
	int offset_y = offset * 2 + SLIDER_HEIGHT;
	int sep_x = offset * 8;
	int x1 = offset;
	int y1 = offset;
	int x2, x3, x4, y2, y3, y4, w1, w2, w3, w4;

	RECT rc;
	GetClientRect(m_hParent, &rc);
	int w_all = rc.right - rc.left;
	int h_all = rc.bottom - rc.top;

	if (w == 0)
		w = w_all;

	const int w_edt = EDITBOX_WIDTH;
	const int h_edt = EDITBOX_HEIGHT;
	int w_sld_all=w-w_edt-offset;
	r5.w = w_edt, r6.w = w_edt;
	r5.h = h_edt, r6.h = h_edt;
	r5.x = x1;
	r6.x = x1;
	x1 += w_edt + offset;

	if (w > TWOLINE_WIDTH_THRESHOLD)
	{
		int w_2 = (w_sld_all - sep_x )/ 2 - offset * 2 ;
		w4 = w3 = w2 = w1 = w_2;
		x3 = x1;
		x4 = x2 = x1 + w_2 + sep_x + offset * 2;
		y2 = y1;
		y4 = y3 = y1 + offset_y;

		// calc h
		h = offset_y * 2;

		// calc edit box size
		r5.y = y1 + offset;
		r6.y = y3 + offset;
	}
	else
	{
		w2 = w3 = w4 = w1 = w_sld_all - offset * 2;
		x2 = x3 = x4 = x1;
		y2 = y1 + offset_y;
		y3 = y2 + offset_y;
		y4 = y3 + offset_y;

		// calc h
		h = offset_y * 4;

		// calc edit box size
		r5.y = y1 + offset;
		r6.y = y2 + offset;
	}

	y = h_all - h;

	r1.x = x1, r2.x = x2, r3.x = x3, r4.x = x4;
	r1.y = y1, r2.y = y2, r3.y = y3, r4.y = y4;
	r1.w = w1, r2.w = w2, r3.w = w3, r4.w = w4;
}

void CtlPanel::CreateCtrls(int x, int y, int w, int h, HWND hParent, HINSTANCE hInst)
{
	int ww = w, hh = 0;
	int xx = x, yy = y;
	//int  x1, x2, x3, x4;
	//int  y1, y2, y3, y4;
	//int  w1, w2, w3, w4;
	//CalcCtrlsSize(m_hParent, xx, yy, ww, hh, x1, x2, x3, x4, y1, y2, y3, y4, w1, w2, w3, w4);
	DispRect r1, r2, r3, r4, r5, r6;
	CalcCtrlsSize(xx, yy, ww, hh, r1, r2, r3, r4, r5, r6);
	int x1 = r1.x, x2 = r2.x, x3 = r3.x, x4 = r4.x;
	int y1 = r1.y, y2 = r2.y, y3 = r3.y, y4 = r4.y;
	int w1 = r1.w, w2 = r2.w, w3 = r3.w, w4 = r4.w;


	m_pCtlWnd->Move(xx, yy, ww, hh, true);

	m_pSldExposure = new SliderSet(x1, y1, w1, L"Exposure", 50, m_hCtlWnd, hInst);
	m_pSldExposure->InitValue(-10, 10, 0.1, 0);
	m_pSldExposure->SetCallback(OnSliding, (ULONG_PTR)this);

	m_pSldDefog = new SliderSet(x2, y2, w2, L"Defog", 50, m_hCtlWnd, hInst);
	m_pSldDefog->InitValue(0, 0.01, 0.0001, 0);
	m_pSldDefog->SetCallback(OnSliding, (ULONG_PTR)this);

	m_pSldKneeLow = new SliderSet(x3, y3, w3, L"Knee Low", 50, m_hCtlWnd, hInst);
	m_pSldKneeLow->InitValue(-3, 3, 0.1, 0);
	m_pSldKneeLow->SetCallback(OnSliding, (ULONG_PTR)this);

	m_pSldKneeHigh = new SliderSet(x4, y4, w4, L"Knee High", 50, m_hCtlWnd, hInst);
	m_pSldKneeHigh->InitValue(3.5, 7.5, 0.1, 5);
	m_pSldKneeHigh->SetCallback(OnSliding, (ULONG_PTR)this);

	m_pEdtCoord = new Editbox(r5.x, r5.y, r5.w, r5.h, true, L"X:   , Y:", m_hCtlWnd);
	m_pEdtColor = new Editbox(r6.x, r6.y, r6.w, r6.h, true, L"RGB:", m_hCtlWnd);
}

void CtlPanel::Move(int x, int y, int w, int h, bool repaint/* = false*/)
{
	int ww = w, hh = 0;
	int xx = x, yy = y;
	//int  x1, x2, x3, x4;
	//int  y1, y2, y3, y4;
	//int  w1, w2, w3, w4;
	//CalcCtrlsSize(m_hParent, xx, yy, ww, hh, x1, x2, x3, x4, y1, y2, y3, y4, w1, w2, w3, w4);

	DispRect r1, r2, r3, r4, r5, r6;
	CalcCtrlsSize(xx, yy, ww, hh, r1, r2, r3, r4, r5, r6);
	int x1 = r1.x, x2 = r2.x, x3 = r3.x, x4 = r4.x;
	int y1 = r1.y, y2 = r2.y, y3 = r3.y, y4 = r4.y;
	int w1 = r1.w, w2 = r2.w, w3 = r3.w, w4 = r4.w;

	m_pSldExposure->Move(x1, y1, w1);
	m_pSldDefog->Move(x2, y2, w2);
	m_pSldKneeLow->Move(x3, y3, w3);
	m_pSldKneeHigh->Move(x4, y4, w4);
	m_pEdtCoord->Move(r5.x, r5.y, r5.w, r5.h);
	m_pEdtColor->Move(r6.x, r6.y, r6.w, r6.h);
	
	m_pCtlWnd->Move(xx, yy, ww, hh, repaint);
}

void CtlPanel::Sliding(HWND hSlider)
{
	bool update = false;
	if (hSlider == m_pSldExposure->GetBarHWND())
	{
		if (m_pViewer)
			m_pViewer->SetExposure(m_pSldExposure->CurValue());
		update = true;
	}
	else if (hSlider == m_pSldDefog->GetBarHWND())
	{
		if (m_pViewer)
			m_pViewer->SetDefog(m_pSldDefog->CurValue());
		update = true;
	}
	else if (hSlider == m_pSldKneeLow->GetBarHWND())
	{
		if (m_pViewer)
			m_pViewer->SetKneeLow(m_pSldKneeLow->CurValue());
		update = true;
	}
	else if (hSlider == m_pSldKneeHigh->GetBarHWND())
	{
		if (m_pViewer)
			m_pViewer->SetKneeHigh(m_pSldKneeHigh->CurValue());
		update = true;
	}

	if (update)
	{
		if (m_pViewer)
		{
			m_pViewer->UpdateImage();
			//m_pViewer->DrawImage();
			m_pViewer->InvalidateRect();
		}
	}
}