#pragma once

#include "stdafx.h"
#include <commctrl.h>
#include <windowsx.h>
#include "userwnd.h"

#define MAX_STRING	100

inline int IntClamp(int v, int _min, int _max)
{
	return (v < _min) ? _min : ((v > _max) ? _max : v);
}

typedef void(*SLIDER_CB)(HWND hSlider, ULONG_PTR arg);

void SetMenuItemState(HMENU hMenu, DWORD ItemId, bool bCheck);
void SetMainMenuItemState(HWND hWnd, DWORD ItemId, bool bCheck);

inline bool GetCheckBox(HWND hCheck)
{
	return (BST_CHECKED == Button_GetCheck(hCheck));
}

inline void SetCheckBox(HWND hCheck, bool bCheck)
{
	Button_SetCheck(hCheck, bCheck ? BM_SETCHECK : BST_UNCHECKED);

}

inline int GetIntFromString(LPCWSTR str)
{
	int result = 0;
	if (str)
	{
		if (wcsncmp(str, L"", MAX_STRING))
		{
			swscanf_s(str, L"%d", &result);
		}
	}
	return result;
}

inline int GetHexFromString(LPCTSTR str)
{
	int result = 0;
	if (str)
	{
		if (wcsncmp(str, L"", MAX_STRING))
		{
			if (swscanf_s(str, L"0x%x", &result) == 0)
			{
				swscanf_s(str, L"%x", &result);
			}
		}
	}
	return result;
}

inline int EditboxGetInt(HWND hEditbox)
{
	WCHAR input[MAX_STRING] = {};
	Edit_GetText(hEditbox, input, MAX_STRING);
	return GetIntFromString(input);
}

inline float EditboxGetFloat(HWND hEditbox)
{
	WCHAR input[MAX_STRING] = {};
	float result = 0.0f;
	Edit_GetText(hEditbox, input, MAX_STRING);
	if (wcsncmp(input, L"", MAX_STRING))
	{
		swscanf_s(input, L"%f", &result);
	}
	return result;
}

inline void EditboxSetInt(HWND hEditbox, int value)
{
	WCHAR output[MAX_STRING] = {};
	swprintf_s(output, MAX_STRING, L"%d", value);
	Edit_SetText(hEditbox, output);
}

inline void EditboxSetUINT(HWND hEditbox, UINT value)
{
	EditboxSetInt(hEditbox, (int)value);
}

inline DWORD EditboxGetHex(HWND hEditbox)
{
	WCHAR input[MAX_STRING] = {};
	DWORD result = 0;
	Edit_GetText(hEditbox, input, MAX_STRING);
	if (wcsncmp(input, L"", MAX_STRING))
	{
		_wcslwr_s(input, MAX_STRING);
		swscanf_s(input, L"%x", &result);
	}
	return result;
}

inline void EditboxSetHex(HWND hEditbox, DWORD value)
{
	WCHAR output[MAX_STRING] = {};
	swprintf_s(output, MAX_STRING, L"0x%X", value);
	Edit_SetText(hEditbox, output);
}

class MenuMan
{
public:
	MenuMan() {}
	MenuMan(HWND hwnd) { Init(hwnd); }
	~MenuMan() {}
	void Init(HWND hwnd)
	{
		m_hWnd = hwnd;
		m_hMainMenu = ::GetMenu(m_hWnd);
	}
	HMENU GetSubItem(int Pos)
	{
		return ::GetSubMenu(m_hMainMenu, Pos);
	}
	void CheckItem(HMENU hMenu, DWORD ItemId, bool bCheck);
	void CheckMainItem(DWORD ItemId, bool bCheck)
	{
		CheckItem(m_hMainMenu, ItemId, bCheck);
	}
	void AppendItem(HMENU hMenu, DWORD ItemId, PWSTR str)
	{
		::AppendMenuW(hMenu, MF_STRING, ItemId, str);
	}
	void RemoveItem(HMENU hMenu, DWORD ItemId)
	{
		::RemoveMenu(hMenu, ItemId, MF_BYCOMMAND);
	}

protected:
	HWND m_hWnd;
	HMENU m_hMainMenu;
};

class StandardScroll
{
public:
	StandardScroll() {}
	StandardScroll(HWND hwnd)
	{
		Init(hwnd);
	}
	~StandardScroll()
	{
	}
	void Init(HWND hwnd)
	{
		m_hWnd = hwnd;
		ZeroMemory(&m_si, sizeof(SCROLLINFO));
		m_si.cbSize = sizeof(SCROLLINFO);
		m_PageX = m_PageY = 0;
		m_MinX = m_MinY = 0;
		m_MaxX = m_MaxY = 0;
		m_bShowVert = false;
		m_bShowHorz = false;
	}

	bool IsVertScroll() { return m_bShowVert; }
	bool IsHorzScroll() { return m_bShowHorz; }
	bool Show(int barId, bool bShow)
	{
		//m_bShowVert = false;
		//m_bShowHorz = false;
		//if (bShow)
		{
			m_bShowVert = bShow && (barId == SB_BOTH || barId == SB_VERT);
			m_bShowHorz = bShow && (barId == SB_BOTH || barId == SB_HORZ);
		}
		return ::ShowScrollBar(m_hWnd, barId, bShow) ? true : false;
	}
	int GetPos(int barId)
	{
		m_si.fMask = SIF_POS;
		GetInfoWithSI(barId);
		return m_si.nPos;
	}
	int SetPos(int barId, int v, bool bRedraw = true)
	{
		return ::SetScrollPos(m_hWnd, barId, v, bRedraw);
	}
	bool SetRange(int barId, int _min, int _max)
	{
		if (barId == SB_HORZ)
		{
			m_MinX = _min;
			m_MaxX = _max;
		}
		else if (barId == SB_VERT)
		{
			m_MinY = _min;
			m_MaxY = _max;
		}

		return ::SetScrollRange(m_hWnd, barId, _min, _max, FALSE) ? true : false;
	}
	int SetPage(int barId, int v)
	{
		if (barId == SB_HORZ)
		{
			m_PageX = v;
		}
		else if (barId == SB_VERT)
		{
			m_PageY = v;
		}
		m_si.fMask = SIF_PAGE;
		m_si.nPage = v;
		return SetInfoWithSI(barId);
	}
	int PageX() { return m_PageX; }
	int PageY() { return m_PageY; }
	int MinX() { return m_MinX; }
	int MinY() { return m_MinY; }
	int MaxX() { return m_MaxX; }
	int MaxY() { return m_MaxY; }
	int ClampXPosToRange(int x)
	{
		return IntClamp(x, m_MinX, m_MaxX - m_PageX);
	}
	int ClampYPosToRange(int y)
	{
		return IntClamp(y, m_MinY, m_MaxY - m_PageY);
	}
protected:
	int GetInfoWithSI(int barId)
	{
		return ::GetScrollInfo(m_hWnd, barId, &m_si);
	}
	int SetInfoWithSI(int barId, bool redraw=true)
	{
		return ::SetScrollInfo(m_hWnd, barId, &m_si, redraw);
	}
	HWND m_hWnd;
	SCROLLINFO m_si;

	int m_PageX, m_PageY;
	int m_MinX, m_MinY;
	int m_MaxX, m_MaxY;
	bool m_bShowVert;
	bool m_bShowHorz;
};

class UserCtrl : public UserWnd
{
protected:

	//static LRESULT CALLBACK UserCtrlSubClassProc(HWND hControl, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
	//{
	//	UserCtrl* thisPtr = (UserCtrl*)dwRefData;
	//	thisPtr->OnMessage2(hControl, message, wParam, lParam);
	//	thisPtr->KillFocus();
	//	return DefSubclassProc(hControl, message, wParam, lParam);
	//}

	virtual int OnMessage(HWND hCtrl, UINT message, WPARAM wParam, LPARAM lParam)
	{
		//HWND hCtrl = HWND(lParam);
		switch (message)
		{
		case WM_LBUTTONUP:
			//if (hCtrl == m_hWnd)
			//{
				OnClick(message, wParam, lParam);
			//}
			break;
		}
		return 0;
	}
	virtual void OnClick(UINT message, WPARAM wParam, LPARAM lParam)
	{

	}
public:
	UserCtrl(PCWSTR wndCls, LPWSTR caption = nullptr, int x = 0, int y = 0, int w = 0, int h = 0, 
		HWND hParent = nullptr, HINSTANCE inst = nullptr, bool bSubProc = false, DWORD style = 0, DWORD styleEx = 0, ULONG_PTR hMenu = 0) :
		UserWnd(wndCls, caption, x, y, w, h, hParent, inst, style, styleEx, hMenu)
	{
		m_hParent = hParent;
		if (bSubProc)
		//	SetWindowSubclass(m_hWnd, UserCtrlSubClassProc, 2047, (DWORD_PTR)this);
			SetOwnProc(m_hWnd);
	}
protected:
	virtual void KillFocus() { SetFocus(m_hParent); }
	HWND m_hParent;
};

class Groupbox : public UserWnd
{
public:
	Groupbox(int x = 0, int y = 0, int w = 0, int h = 0, LPWSTR caption = nullptr, HWND hParent = nullptr, HINSTANCE inst = nullptr) :
		UserWnd(L"BUTTON", caption, x, y, w, h, hParent, inst, WS_CHILD | WS_VISIBLE | BS_GROUPBOX, 0, 0)
	{
	}
};

class Checkbox : public UserCtrl
{
#define CHKBOX_WIDTH	14
#define CHKBOX_HEIGHT	14

	virtual void OnClick(UINT message, WPARAM wParam, LPARAM lParam)
	{
		bool state = Get();
		Set(!state);
	}
public:
	Checkbox(int x = 0, int y = 0, LPWSTR caption=nullptr, int label_width= 0, HWND hParent = nullptr, HINSTANCE inst = nullptr, bool bSubProc=false) :
		UserCtrl(L"BUTTON", caption, x, y, (caption ? label_width + CHKBOX_WIDTH : CHKBOX_WIDTH), CHKBOX_HEIGHT, hParent, inst, bSubProc, WS_CHILD | WS_VISIBLE | BS_CHECKBOX, 0, 0)
	{
		m_Width = (caption ? label_width + CHKBOX_WIDTH : CHKBOX_WIDTH);
		m_Height = CHKBOX_HEIGHT;
		if (bSubProc)
			//	SetWindowSubclass(m_hWnd, CheckboxSubClassProc, 0, (DWORD_PTR)this);
			SetOwnProc(m_hWnd);
	}
	virtual void Move(int x = 0, int y = 0, bool bRepaint = false)
	{
		UserWnd::Move(x, y, m_Width, m_Height, bRepaint);
	}
	virtual bool Get()
	{
		//return (Button_GetCheck(m_hWnd) == BST_CHECKED) ? true : false;
		return GetCheckBox(m_hWnd) ? true: false;
	}
	virtual void Set(bool value)
	{
		//Button_SetCheck(m_hWnd, (value ? BST_CHECKED : BST_UNCHECKED));
		SetCheckBox(m_hWnd, value);
	}
};

class Listbox : public UserWnd
{
public:
	Listbox(int x = 0, int y = 0, int w = 0, int h = 0, HWND hParent = nullptr, HINSTANCE inst = nullptr, 
		DWORD style = 0, DWORD styleEx = 0, ULONG_PTR hMenu = 0) : 
		UserWnd(L"LISTBOX", nullptr, x, y, w, h, hParent, inst, WS_CHILD | WS_BORDER | WS_VISIBLE), m_Count(0)
	{
	}

	virtual int InsertStringAndData(LPWSTR str, DWORD data)
	{
		int index = ListBox_AddString(m_hWnd, str);
		int ret = ListBox_SetItemData(m_hWnd, index, (LPARAM)data);
		m_Count++;
		return ret;
	}

	virtual DWORD GetCurSelData()
	{
		int index = ListBox_GetCurSel(m_hWnd);
		if (index == LB_ERR)
		{
			return index;
		}
		return (DWORD)(ListBox_GetItemData(m_hWnd, index));
	}

	virtual int SetCurSelByData(DWORD data)
	{
		/*	int index = ListBox_SelectItemData(hList, -1, (LPARAM)data);
		if (index == LB_ERR)
		{
		return index;
		}*/
		int count = ListBox_GetCount(m_hWnd);
		for (int index = 0; index < count; index++)
		{
			if ((DWORD)(ListBox_GetItemData(m_hWnd, index)) == data)
			{
				return ListBox_SetCurSel(m_hWnd, index);
			}
		}
		return LB_ERR;
	}

protected:
	//UserWnd *m_pList;
	//HWND m_hList;
	int m_Count;
};

class SliderSet : public BasicWnd
{
#define SLIDER_HEIGHT	22
	const int cw_label_default = 40;
	const int ch_label = 14;
	const int ch_bar = SLIDER_HEIGHT;
	const int cw_txt_default = 40;
	const int ch_txt = 18;
public:
	SliderSet(int x = 0, int y = 0, int w = 0/*, int h = 0*/, LPWSTR caption= nullptr, int caption_width = 0, HWND hParent = nullptr, HINSTANCE inst = nullptr, 
		bool bCheckbox = false, bool bVertical = false, bool ticks = false) : 
		BasicWnd(x, y, w, (/*h > SLIDER_HEIGHT ? h : */SLIDER_HEIGHT), hParent, inst), m_pBar(nullptr), m_pTxt(nullptr), m_pChk(nullptr),
		m_hBar(nullptr), m_hTxt(nullptr), m_hChk(nullptr), m_hTip(nullptr), m_bVertical(bVertical),
		m_pCb(nullptr), m_pArg(0)
	{
		//int width = w - 4;
		CreateSlider(/*x, y, */w, caption, ((caption_width == 0)? cw_label_default: caption_width), m_hWnd, inst, bCheckbox, bVertical, ticks);
		SetOwnProc(m_hWnd);
	}
	virtual void SetCallback(SLIDER_CB cb, ULONG_PTR arg)
	{
		m_pCb = cb;
		m_pArg = arg;
	}
	virtual ~SliderSet()
	{
		SAFEDELETE(m_pLabel);
		SAFEDELETE(m_pBar);
		SAFEDELETE(m_pTxt);
		SAFEDELETE(m_pChk);
	}

	virtual void CreateSlider(int w = 0, LPWSTR caption = nullptr, int label_width = 0,
		HWND hParent = nullptr, HINSTANCE inst = nullptr,
		bool bCheckbox = false, bool bVertical = false, bool ticks = false);

	virtual void Move(int x = 0, int y = 0, int w = 0/*, int h = 0*/, bool bRepaint = false);
	virtual HWND GetBarHWND() { return m_pBar ? m_pBar->GetHWND() : nullptr; }
	virtual HWND GetTxtHWND() { return m_pTxt ? m_pTxt->GetHWND() : nullptr; }

	virtual void InitValue(float minValue, float maxValue, float stepping = 1.f, float curValue = 0.f);
	virtual void Enable(BOOL bEnable)
	{
		EnableWindow(m_hBar, bEnable);
		EnableWindow(m_hTxt, bEnable);
	}

	//void ManuallyEnable(BOOL bEnable)
	//{
	//	Enable(bEnable);
	//	if (m_hChk) SetCheckBox(m_hChk, bEnable);
	//}

	//BOOL IsManualMode() { if (m_hChk) { return GetCheckBox(m_hChk); } else { return TRUE; } }

	virtual void UpdateValue(LONG NewPos)
	{
		m_CurValue = (float)NewPos * m_Stepping + m_MinValue;
		Display(m_CurValue);
	}

	virtual float CurValue() { return m_CurValue; }
	//LONG GetPostion() { return SendMessage(m_hBar, TBM_GETPOS, 0, 0); }
	/*LONG GetPostion(WPARAM wParam, LPARAM lParam)
	{
	HWND hTrack = (HWND)(lParam);
	if (hTrack != m_hBar)
	{
	return -1;
	}
	if((LOWORD(wParam) == TB_THUMBTRACK) ||
	(LOWORD(wParam) == TB_THUMBPOSITION))
	{
	return (LONG)(HIWORD(wParam));
	}
	else
	{
	return (LONG)SendMessage(m_hBar, TBM_GETPOS, 0, 0);
	}
	}*/
	virtual void Display(LPTSTR str) { if (m_hTxt) { SetWindowTextW(m_hTxt, str); } wcscpy_s(m_szTmpTxt, MAX_STRING, str); SetTipText(); }
	virtual void Display(LONG curValue) { swprintf_s(m_szTmpTxt, MAX_STRING, L"%d", curValue); Display(m_szTmpTxt); }
	virtual void Display(float curValue) { swprintf_s(m_szTmpTxt, MAX_STRING, L"%.4f", curValue); Display(m_szTmpTxt);}
private:
	// cb on slider moving
	SLIDER_CB m_pCb;
	ULONG_PTR m_pArg;
	// dimensions
	int w_label;
	int w_txt;
	int w_chk;
	int w_bar;


	bool m_bVertical;
	UserCtrl *m_pLabel;
	//UserWnd *m_pBar;
	UserCtrl *m_pBar;
	UserCtrl *m_pTxt;
	Checkbox *m_pChk;

	HWND m_hBar;
	HWND m_hTxt;
	HWND m_hChk;
	HWND m_hTip;
	//LONG m_MinValue;
	//LONG m_MaxValue;
	//LONG m_Stepping;
	//LONG m_CurValue;
	float m_MinValue;
	float m_MaxValue;
	float m_Stepping;
	float m_CurValue;
	WCHAR m_szTmpTxt[MAX_STRING];

	void SetPostion(int curPos) { SendMessage(m_hBar, TBM_SETPOS, TRUE, (LPARAM)curPos); }

	virtual void SetRange(int minPos, int maxPos)
	{
		SendMessage(m_hBar, TBM_SETRANGEMIN, TRUE, (LPARAM)minPos);
		SendMessage(m_hBar, TBM_SETRANGEMAX, TRUE, (LPARAM)maxPos);
	}
	void temp()
	{
		int iMin, iMax, iSelMin, iSelMax;
		SendMessage(GetBarHWND(), TBM_SETRANGE,
			(WPARAM)TRUE,                   // redraw flag 
			(LPARAM)MAKELONG(iMin, iMax));  // min. & max. positions

		SendMessage(GetBarHWND(), TBM_SETPAGESIZE,
			0, (LPARAM)4);                  // new page size 

		SendMessage(GetBarHWND(), TBM_SETSEL,
			(WPARAM)FALSE,                  // redraw flag 
			(LPARAM)MAKELONG(iSelMin, iSelMax));

		SendMessage(GetBarHWND(), TBM_SETPOS,
			(WPARAM)TRUE,                   // redraw flag 
			(LPARAM)iSelMin);
		SetFocus(GetBarHWND());
	}

	virtual void SetTipText();
	virtual int OnMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};