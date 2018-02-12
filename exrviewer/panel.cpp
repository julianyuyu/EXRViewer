

#include "stdafx.h"
#include "panel.h"

int ImageWnd::OnMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static bool bMouseMoving = false;
	switch (message)
	{
	case WM_LBUTTONUP:
		if (bMouseMoving)
		{
			bMouseMoving = false;

			HCURSOR cur = LoadCursor(nullptr, IDC_ARROW);
			SetClassLongPtrW(hWnd, GCLP_HCURSOR, LONG_PTR(cur));
			//SetCursor(cur);
		}
		break;
	case WM_MOUSEMOVE:
		if (wParam == MK_LBUTTON)
		{
			LONG xpos = GET_X_LPARAM(lParam);
			LONG ypos = GET_Y_LPARAM(lParam);
			static LONG x_offset = 0;
			static LONG y_offset = 0;
			if (!bMouseMoving)
			{
				HCURSOR cur = LoadCursor(nullptr, IDC_HAND);
				SetClassLongPtrW(hWnd, GCLP_HCURSOR, LONG_PTR(cur));
				//SetCursor(cur);
				bMouseMoving = true;
				x_offset = xpos;
				y_offset = ypos;
			}
			if (bMouseMoving)
			{
				DispRect *rc = m_pViewer->GetScrollRect();
				if (xpos < x_offset)
					rc->x = x_offset - xpos;
				if (ypos < y_offset)
					rc->y = y_offset - ypos;
				m_pViewer->DrawImage();
			}
		}
		break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		if (m_pViewer)
			m_pViewer->DrawImage();
	}
	break;
	}
	return 0;
}

void CalcCtrlsSize(HWND hParent, int x, int y, int& w, int& h,
	int& x1, int& x2, int& x3, int& x4, 
	int& y1, int& y2, int& y3, int& y4,
	int& w1, int& w2, int& w3, int& w4)
{
	int offset = 4;
	int offset_y = offset * 2 + SLIDER_HEIGHT;
	int sep_x = offset * 8;
	x1 = offset;
	y1 = offset;

	RECT rc;
	GetClientRect(hParent, &rc);
	int w_all = rc.right - rc.left;
	int h_all = rc.bottom - rc.top;

	if (w == 0)
		w = w_all;

	if (w > TWOLINE_WIDTH_THRESHOLD)
	{
		int w_2 = (w - sep_x )/ 2 - offset * 2 ;
		w4 = w3 = w2 = w1 = w_2;
		x3 = x1;
		x4 = x2 = x1 + w_2 + sep_x + offset * 2;
		y2 = y1;
		y4 = y3 = y1 + offset_y;

		// calc h
		h = offset_y * 2;
	}
	else
	{
		w2 = w3 = w4 = w1 = w - offset * 2;
		x2 = x3 = x4 = x1;
		y2 = y1 + offset_y;
		y3 = y2 + offset_y;
		y4 = y3 + offset_y;

		// calc h
		h = offset_y * 4;
	}

	y = h_all - h;
}

void CtlPanel::CreateCtrls(int x, int y, int w, int h, HWND hParent, HINSTANCE hInst)
{
	int ww = w, hh = 0;
	int xx = x, yy = y;
	int  x1, x2, x3, x4;
	int  y1, y2, y3, y4;
	int  w1, w2, w3, w4;
	CalcCtrlsSize(m_hParent, xx, yy, ww, hh, x1, x2, x3, x4, y1, y2, y3, y4, w1, w2, w3, w4);

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
}

void CtlPanel::Move(int x, int y, int w, int h, bool repaint/* = false*/)
{
	int ww = w, hh = 0;
	int xx = x, yy = y;
	int  x1, x2, x3, x4;
	int  y1, y2, y3, y4;
	int  w1, w2, w3, w4;

	CalcCtrlsSize(m_hParent, xx, yy, ww, hh, x1, x2, x3, x4, y1, y2, y3, y4, w1, w2, w3, w4);

	m_pSldExposure->Move(x1, y1, w1);
	m_pSldDefog->Move(x2, y2, w2);
	m_pSldKneeLow->Move(x3, y3, w3);
	m_pSldKneeHigh->Move(x4, y4, w4);
	
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