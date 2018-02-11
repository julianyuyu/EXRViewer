#pragma once

#include <commctrl.h>
#include <windows.h>
#include <functional>

/*
	if ENABLE_2_CB, no need to define derived class, just SetMessageCb to the object.
	else, we should define a derived class, which implement the OnMessage function. eg: BackFrameWnd
*/
#define ENABLE_2_CB	0

const int g_SplitterSize = 20;
const PCWSTR szBasicWindowClass = L"BasicWindowClassName";

inline void GetClientSize(HWND hWnd, int& w, int& h)
{
	RECT rect;
	GetClientRect(hWnd, &rect);
	w = rect.right - rect.left;
	h = rect.bottom - rect.top;
}

class UserProcWnd
{
	static LRESULT CALLBACK UserDefinedProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		UserProcWnd *thisPtr = (UserProcWnd*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
		if (thisPtr)
			thisPtr->OnMessage_Stage1(hWnd, message, wParam, lParam);
		return CallWindowProc(thisPtr->m_pOrigProc, hWnd, message, wParam, lParam);
	}
	static LRESULT CALLBACK UserDefinedProc_Method2(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR  uIdSubclass, DWORD_PTR dwRefData)
	{
		UserProcWnd *thisPtr = (UserProcWnd*)dwRefData;
		if (thisPtr)
			thisPtr->OnMessage_Stage1(hWnd, message, wParam, lParam);
		return DefSubclassProc(hWnd, message, wParam, lParam);
	}
public:
	UserProcWnd() : /*m_hWnd(nullptr), */m_pOrigProc(nullptr)
#if ENABLE_2_CB
		, m_MessageCb(nullptr)
#endif
	{
	}
	UserProcWnd(PCWSTR wndCls, PCWSTR wndName = nullptr, int x = 0, int y = 0, int w = 0, int h = 0,
		HWND hParent = nullptr, HINSTANCE inst = nullptr, DWORD style = 0, DWORD styleEx = 0, ULONG_PTR hMenu = 0) :
		/*m_hWnd(nullptr), */m_pOrigProc(nullptr)
#if ENABLE_2_CB
		, m_MessageCb(nullptr)
#endif
	{

	}
	virtual ~UserProcWnd() {}

	// PROC definition type1 (there are 2 methods for type1): for class which defined own OnMessage routine
	virtual void SetOwnProc(HWND hWnd)
	{
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)this);
		m_pOrigProc = (WNDPROC)SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)UserDefinedProc);
	}
	virtual void SetUserProc_Method2(HWND hWnd)
	{
		SetWindowSubclass(hWnd, UserDefinedProc_Method2, 0, (ULONG_PTR)this);
	}
#if ENABLE_2_CB
	// PROC definition type2: for object which has not defined OnMessage, and want to set arbitrary STATIC func as CB
	virtual void SetMessageCb(HWND hWnd, WNDPROC cb)
	{
		m_MessageCb = cb;
		SetOwnProc(hWnd);
	}
#endif
protected:
#if ENABLE_2_CB
	virtual int OnMessage_Stage1(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		if (m_MessageCb)
			m_MessageCb(hWnd, message, wParam, lParam);
		OnMessage(hWnd, message, wParam, lParam);
		return 0;
	}
#else
	inline int OnMessage_Stage1(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{ return OnMessage(hWnd, message, wParam, lParam); }
#endif
	virtual int OnMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		// to be implemented in derived class.
		return 0;
	}
	//HWND m_hWnd;
	WNDPROC m_pOrigProc;
#if ENABLE_2_CB
	WNDPROC m_MessageCb;
#endif
};

class UserWnd : public UserProcWnd
{
public:
	UserWnd() : UserProcWnd(), m_hWnd(nullptr), m_bShowing(false), m_Width(0), m_Height(0)
	{
		//if (m_hCursor)
		//	m_hCursor = (HCURSOR)LoadCursor(nullptr, IDC_ARROW);
		if (!m_hAppInst)
			m_hAppInst = GetModuleHandle(nullptr);
		if (!m_hFont)
			m_hFont = CreateFontW(-12, 0, 0, 0, FW_NORMAL, 0, 0, 0, 0x86, 0, 0, 0, 0, L"MS Shell Dlg");
	}
	UserWnd(PCWSTR wndCls, PCWSTR wndName = nullptr, int x = 0, int y = 0, int w = 0, int h = 0,
			HWND hParent = nullptr, HINSTANCE inst = nullptr, DWORD style = 0, DWORD styleEx = 0, ULONG_PTR hMenu = 0) :
		UserProcWnd(wndCls, wndName, x, y, w, h, hParent, inst, style, styleEx, hMenu),
		m_hWnd(nullptr), m_bShowing(false), m_Width(w), m_Height(h)
	{
		UserWnd();
		if (wndCls)
			CreateWnd(wndCls, wndName, x, y, w, h, hParent, inst, style, styleEx, hMenu);
	}
	virtual ~UserWnd()
	{
		if (m_hWnd)
			DestroyWindow(m_hWnd);
		m_FontRefCount--;
		if (m_FontRefCount == 0)
		{
			DeleteObject(m_hFont);
			m_hFont = nullptr;
		}
	}
	virtual HWND CreateWnd(PCWSTR wndCls, PCWSTR wndName = nullptr, int x = 0, int y = 0, int w = 0, int h = 0,
						   HWND hParent = nullptr, HINSTANCE inst = nullptr, DWORD style=0, DWORD styleEx=0, ULONG_PTR hMenu=0)
	{
		if (style == 0)
			style = WS_CHILD | WS_VISIBLE;
		if (!inst)
			inst = m_hAppInst;
		m_Width = w;
		m_Height = h;
		m_hWnd = CreateWindowExW(styleEx, wndCls, wndName, style, x, y, w, h, hParent, (HMENU)hMenu, inst, nullptr);
		SetFont();
		m_FontRefCount++;
		return m_hWnd;
	}
	virtual void GetSize(int& w, int& h)
	{
		//if (m_hWnd)
		//	GetClientSize(m_hWnd, w, h);
		w = m_Width;
		h = m_Height;
	}
	virtual void Move(int x = 0, int y = 0, int w = 0, int h = 0, bool bRepaint = false)
	{
		MoveWindow(m_hWnd, x, y, w, h, bRepaint ? TRUE : FALSE);
		m_Width = w;
		m_Height = h;
	}

	virtual HWND GetHWND() { return m_hWnd; }
	virtual bool IsShowing() { return m_bShowing; }
	virtual void Show(bool show)
	{
		m_bShowing = show;
		ShowWindow(m_hWnd, show ? SW_SHOW : SW_HIDE);
	}
	virtual void SetFont(HFONT font=nullptr)
	{
		if (!font)
			font = m_hFont;
		SendMessage(m_hWnd, WM_SETFONT, (WPARAM)font, TRUE);
	}
protected:
	int m_Width;
	int m_Height;
	HWND m_hWnd;
	bool m_bShowing;
//	static HCURSOR m_hCursor;
	static HINSTANCE m_hAppInst;
	static HFONT m_hFont;
	static int m_FontRefCount;
};

class BasicWnd : public UserWnd
{
	static LRESULT CALLBACK BasicWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		switch (message)
		{
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);
			EndPaint(hWnd, &ps);
		}
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		return 0;
	}
public:
	BasicWnd(bool bTempHidingWnd, HWND hParent = nullptr, HINSTANCE hInstance = nullptr) : UserWnd()
	{
		if (bTempHidingWnd)
		{
			// to create temp hiding window, for glew init using
			Create(0, 0, 10, 4, hParent, hInstance, WS_OVERLAPPEDWINDOW, 0, false);
		}
	}

	BasicWnd() : UserWnd() {}
	BasicWnd(int x, int y, int w, int h, HWND hParent = nullptr, HINSTANCE hInstance = nullptr, DWORD style = 0, DWORD styleEx = 0, bool bShow = true) : UserWnd()
	{
		Create(x, y, w, h, hParent, hInstance, style, styleEx, bShow);
	}
	~BasicWnd() {}

	virtual HWND Create(int x = 0, int y = 0, int w = 0, int h = 0, HWND hParent = nullptr, HINSTANCE hInst = nullptr, DWORD style=0, DWORD styleEx=0, bool bShow = true)
	{
		RegClass(hInst);
		m_hWnd = CreateWnd(szBasicWindowClass, nullptr, x, y, w, h, hParent, hInst, style, styleEx);

		Show(bShow);
		UpdateWindow(m_hWnd);
		return m_hWnd;
	}
	static void RegClass(HINSTANCE hInstance = nullptr)
	{
		if (!m_ClassReged)
		{
			if (!m_hAppInst)
				m_hAppInst = hInstance;

			WNDCLASSEXW wcex = {};
			wcex.cbSize = sizeof(WNDCLASSEX);
			wcex.style = CS_HREDRAW | CS_VREDRAW;
			wcex.lpfnWndProc = BasicWndProc;
			wcex.hbrBackground = (HBRUSH)COLOR_BACKGROUND+1; //COLOR_WINDOW;
			wcex.hInstance = hInstance;
			//wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_BRDFCONSOLE));
			wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
			wcex.lpszClassName = szBasicWindowClass;
			//wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
			//wcex.lpszMenuName = NULL;
			RegisterClassExW(&wcex);
			m_ClassReged = true;
		}
	}
private:
	static bool m_ClassReged;
//	static HINSTANCE m_hInst;
//	static WNDPROC m_OrigProc;
	//HWND m_hWnd;
};

class LayoutWnd : UserWnd
{
public:
	LayoutWnd(int x = 0, int y = 0, int w = 0, int h = 0,
		HWND hParent = nullptr, HINSTANCE inst = nullptr, bool bSubProc = false, DWORD style = 0, DWORD styleEx = 0, ULONG_PTR hMenu = 0) :
		UserWnd(L"STATIC", nullptr, x, y, w, h, hParent, inst, style, styleEx, hMenu)
	{
		//if (bSubProc)
			//	SetWindowSubclass(m_hWnd, UserCtrlSubClassProc, 2047, (DWORD_PTR)this);
		//	SetOwnProc(m_hWnd);
	}
};

class SplitterWnd : public UserWnd
{
public:
	//SplitterWnd()
	//{
	//	if (m_hCursor)
	//		m_hCursor = LoadCursor(NULL, MAKEINTRESOURCE(IDC_SIZEWE));
	//}
	
	SplitterWnd(bool vertical, int pos, HWND hPrev, HWND hNext, HWND hParent, HINSTANCE hInst);
	virtual void Move(int pos = 0, bool bRepaint = false);
	virtual int OnMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	//static LRESULT CALLBACK SplitterWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static int GetWidth() { return g_SplitterSize; }
protected:
	static HCURSOR m_hCursor;
	bool m_bVertical; // if false, then it's a horzontal splitter
	bool m_bSplitting; // in splitting mode, means move mouse with lbutton down.
	HWND m_hPrevWnd;
	HWND m_hNextWnd;
	HWND m_hParent;
};

class TabWnd : public UserWnd
{
public:
	TabWnd(int x, int y, int w, int h, HWND hParent, HINSTANCE hInst) : m_PageCount(0), UserWnd()
	{
		m_hWnd = CreateWnd(WC_TABCONTROLW, nullptr, x, y, w, h, hParent, hInst, WS_CHILD | WS_VISIBLE);
	}
	virtual ~TabWnd()
	{
		m_PageCount = 0;
	}
	virtual int InsertPage(PWSTR txt)
	{
		TCITEM tie;
		tie.mask = TCIF_TEXT;
		tie.pszText = txt;
		TabCtrl_InsertItem(m_hWnd, m_PageCount, &tie);
		m_PageCount++;
		return m_PageCount - 1; // return tab id
	}
	virtual int CurrPage()
	{
		return TabCtrl_GetCurSel(m_hWnd);
	}
	virtual PWSTR CurrPageName()
	{
		TCITEM tie = {};
		tie.mask = TCIF_TEXT;
		tie.pszText = m_PageNameString;
		tie.cchTextMax = MAX_PATH;
		TabCtrl_GetItem(m_hWnd, CurrPage(), &tie);
		return m_PageNameString;
	}
	virtual void GetPageRect(RECT* rect)
	{
		//TabCtrl_GetItemRect(m_hTab, 2, &rc);
		GetClientRect(m_hWnd, rect);
		//TabCtrl_AdjustRect(m_hTab, TRUE, &rc);
		TabCtrl_AdjustRect(m_hWnd, FALSE, rect);
	}
	virtual void GetPageSize(int& x, int& y, int& w, int& h)
	{
		RECT rc;
		GetPageRect(&rc);
		x = rc.left;
		y = rc.top;
		w = rc.right - rc.left;
		h = rc.bottom - rc.top;
	}
	virtual bool PageNameMatch(int page, PWSTR str)
	{
		return WSTR_MATCH(CurrPageName(), str) ? true : false;
	}
	virtual bool CurrPageNameMatch(PWSTR str)
	{
		return PageNameMatch(CurrPage(), str);
	}
	virtual bool IsTabChanged(LPNMHDR hdr)
	{
		return (hdr->hwndFrom == m_hWnd && hdr->code == TCN_SELCHANGE) ? true : false;
	}
private:
	WCHAR m_PageNameString[MAX_PATH];
	int m_PageCount;
};

