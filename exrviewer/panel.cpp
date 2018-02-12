

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
				//SetClassLong(hWnd, -12/*GCL_HCURSOR*/, LONG(NULL));
				//SetCursor(cur);
				bMouseMoving = true;
				/*uic->*/x_offset = xpos;
				/*uic->*/y_offset = ypos;
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
		// TODO: Add any drawing code that uses hdc here...
		EndPaint(hWnd, &ps);
		if (m_pViewer)
			m_pViewer->DrawImage();
	}
	break;
	}
	return 0;
}

void CtlPanel::CreateCtrls(int x, int y, int w, int h, HWND hParent, HINSTANCE hInst)
{
	int offset = 4;
	int offset_2 = 8 + SLIDER_HEIGHT;
	int x_sldr = offset, y_sldr = offset;
	int w_sldr = w - offset * 2;
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

	Move(x, y, w, h, true);
}

void CtlPanel::Move(int x, int y, int w, int h, bool repaint/* = false*/)
{
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