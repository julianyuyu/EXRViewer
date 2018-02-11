#include "stdafx.h"
#include "userwnd.h"

#pragma comment (lib, "comctl32.lib")

// Include the v6 common controls in the manifest
#pragma comment(linker, \
	"\"/manifestdependency:type='Win32' "\
	"name='Microsoft.Windows.Common-Controls' "\
	"version='6.0.0.0' "\
	"processorArchitecture='*' "\
	"publicKeyToken='6595b64144ccf1df' "\
	"language='*'\"")

HINSTANCE UserWnd::m_hAppInst = nullptr;
int UserWnd::m_FontRefCount = 0;
HFONT UserWnd::m_hFont = nullptr;

bool BasicWnd::m_ClassReged = false;
//HINSTANCE BasicWnd::m_hInst = nullptr;
//WNDPROC BasicWnd::m_OrigProc = nullptr;

HCURSOR SplitterWnd::m_hCursor = nullptr;


SplitterWnd::SplitterWnd(bool vertical, int pos, HWND hPrev, HWND hNext, HWND hParent, HINSTANCE hInst)
//	: BasicWnd()
	: m_hParent(hParent)
{
	//SplitterWnd();
	if (m_hCursor)
		m_hCursor = (HCURSOR)LoadCursor(nullptr, IDC_CROSS);
		//m_hCursor = (HCURSOR)LoadImageW(hInst, IDC_CROSS, IMAGE_CURSOR, LR_DEFAULTSIZE, LR_DEFAULTSIZE, LR_SHARED);

	//RECT rc;
	//GetClientRect(hParent, &rc);
	//int ww = rc.right - rc.left;
	//int hh = rc.bottom - rc.top;
#if 0
	int ww, hh;
	GetClientSize(hParent, ww, hh);

	m_bVertical = vertical;
	int x, y, w, h;
	if (m_bVertical)
	{
		x = 0;
		y = pos;
		w = ww;
		h = g_SplitterSize;
	}
	else
	{
		x = pos;
		y = 0;
		w = g_SplitterSize;
		h = hh;
	}

	CreateWnd(L"BUTTON", nullptr, x, y, w, h, hParent);
#else

	CreateWnd(L"BUTTON", nullptr, 0,0,0,0, hParent);
	Move(pos);
#endif
	//Create(x, y, w, h, hParent);
	SetUserProc_Method2(m_hWnd);
	SetCapture(m_hWnd);
//	SetWindowLongPtr(m_hWnd, GWLP_USERDATA, (LONG_PTR)this);
	SetWindowLongPtr(m_hWnd, GWL_EXSTYLE, WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE);
//	SetWindowLongPtr(m_hWnd, GWL_STYLE, WS_BORDER | WS_CHILD | WS_VISIBLE);
//	SetClassLongPtr(m_hWnd, GCLP_HCURSOR, (LONG_PTR)m_hCursor);
	//TRACKMOUSEEVENT tme = {sizeof(TRACKMOUSEEVENT), TME_HOVER, m_hWnd, HOVER_DEFAULT};
	//TrackMouseEvent(&tme);
	m_hPrevWnd = hPrev;
	m_hNextWnd = hNext;
}

void SplitterWnd::Move(int pos, bool bRepaint)
{
	int x, y, w, h;
	int ww, hh;
	GetClientSize(m_hParent, ww, hh);

	if (m_bVertical)
	{
		x = 0;
		y = pos;
		w = ww;
		h = g_SplitterSize;
	}
	else
	{
		x = pos;
		y = 0;
		w = g_SplitterSize;
		h = hh;
	}

	MoveWindow(m_hWnd, x, y, w, h, bRepaint ? TRUE : FALSE);
	m_Width = w;
	m_Height = h;
}

int SplitterWnd::OnMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		/*
		case WM_ERASEBKGND:
		//case WM_PAINT:
		{
		//HBRUSH hbrush = CreateSolidBrush(RGB(0, 255, 0));
		HBRUSH hbrush = (HBRUSH)::GetStockObject(BLACK_BRUSH);
		RECT rect;
		GetClientRect(hWnd, &rect);
		HDC dc = GetDC(hWnd);

		FillRect(dc, &rect, hbrush);
		ReleaseDC(hWnd, dc);
		//return 1;
		break;
		}*/

	case WM_MOUSEMOVE:
		//SetCursor(m_hCursor);
		break;
	case WM_MOUSEHOVER:
		//SetCursor(m_hCursor);
		break;
	case WM_LBUTTONDOWN:
		//SetCursor(m_hCursor);
		m_bSplitting = true;
		break;
	case WM_LBUTTONUP:
		m_bSplitting = false;
		break;
		//case WM_MOVE:
		//	break;
		//case WM_DESTROY:
		//	ReleaseCapture();
		//	break;
	default:
		return 0;
	}
	return 0;
}