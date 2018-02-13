#pragma once

#include "stdafx.h"
#include <commctrl.h>
#include <windowsx.h>
#include "userwnd.h"

#define MAX_STRING	100

typedef void(*SLIDER_CB)(HWND hSlider, ULONG_PTR arg);

inline bool SetMenuItemState(HMENU hMenu, DWORD ItemId, bool bCheck)
{
	if (hMenu)
	{
		if (bCheck)
		{
			return CheckMenuItem(hMenu, ItemId, MF_BYCOMMAND | MF_CHECKED);
		}
		else
		{
			return CheckMenuItem(hMenu, ItemId, MF_BYCOMMAND | MF_UNCHECKED);
		}
	}
	return false;
}

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

	virtual void CreateSlider(/*int x = 0, int y = 0, */int w = 0, LPWSTR caption = nullptr, int label_width = 0,
		HWND hParent = nullptr, HINSTANCE inst = nullptr, bool bCheckbox = false, bool bVertical = false, bool ticks = false)
	{
		w_label = caption ? label_width : 0;
		w_txt = cw_txt_default;
		w_chk = bCheckbox ? CHKBOX_WIDTH : 0;
		w_bar = w - w_label - w_txt - w_chk - 2 /*space*/ - 4 /*space between bar and edit*/-2 /*space*/;

		int y_label = (ch_bar - ch_label) / 2+2;
		if (caption)
		{
			m_pLabel = new UserCtrl(L"STATIC", caption, 0, y_label, w_label, ch_label, hParent, inst, false, WS_CHILD | WS_VISIBLE | SS_LEFT);
			SendMessage(m_hBar, TBM_SETBUDDY, (WPARAM)TRUE, (LPARAM)m_pLabel->GetHWND());
		}

		m_pBar = new UserCtrl(TRACKBAR_CLASS, nullptr, w_label + 2, 0, w_bar, ch_bar, hParent, inst, false,
			WS_CHILD | WS_VISIBLE | (bVertical? TBS_VERT : TBS_HORZ) | (ticks ? (TBS_BOTH): TBS_BOTH | TBS_NOTICKS) /* | TBS_AUTOTICKS | TBS_ENABLESELRANGE*/);
		m_hBar = m_pBar->GetHWND();

		//int y_txt = (ch_bar - ch_txt) / 2;
		m_pTxt = new UserCtrl(L"EDIT", L"", 0, 0, w_txt, ch_txt, hParent, inst, false, /*WS_BORDER |*/ WS_CHILD | WS_VISIBLE | ES_RIGHT | ES_READONLY);
		SendMessage(m_hBar, TBM_SETBUDDY, (WPARAM)FALSE, (LPARAM)m_pTxt->GetHWND());
		m_hTxt = m_pTxt->GetHWND();

		if (bCheckbox)
		{
			int x_chk = w - w_chk - 2;
			int y_chk = (ch_bar - CHKBOX_HEIGHT ) / 2 +2;
			m_pChk = new Checkbox(x_chk, y_chk, nullptr, 0, hParent, inst);
			m_hChk = m_pChk->GetHWND();
		}
		m_hTip = (HWND)SendMessage(m_hBar, TBM_GETTOOLTIPS, 0, 0);
	}

#if 1
	virtual void Move(int x = 0, int y = 0, int w = 0/*, int h = 0*/, bool bRepaint = false)
	{
		BasicWnd::Move(x, y, w, SLIDER_HEIGHT, bRepaint);
		int y_label = (ch_bar - ch_label) / 2;
		m_pLabel->Move(0, y_label, w_label, ch_label);
		int width = m_Width;
		w_bar = width - w_label - w_txt - w_chk - 2 /*space*/ - 4 /*space between bar and edit*/ - 2 /*space*/;
		m_pBar->Move(w_label + 2, 0, w_bar, ch_bar);
		if (m_pChk)
		{
			int x_chk = w - w_chk - 2;
			int y_chk = (ch_bar - CHKBOX_WIDTH) / 2;;
			m_pChk->Move(x_chk, y_chk);
		}
	}
#endif

	virtual HWND GetBarHWND() { return m_pBar ? m_pBar->GetHWND() : nullptr; }
	virtual HWND GetTxtHWND() { return m_pTxt ? m_pTxt->GetHWND() : nullptr; }

	virtual void InitValue(float minValue, float maxValue, float stepping=1.f, float curValue=0.f)
	{
		int MinPos, MaxPos, CurPos;
		m_MinValue = minValue;
		m_MaxValue = maxValue;
		m_Stepping = stepping;
		m_CurValue = curValue;
		MinPos = 0;
		MaxPos = (m_MaxValue - m_MinValue) / m_Stepping;
		CurPos = (m_CurValue - m_MinValue) / m_Stepping;
		SetRange(MinPos, MaxPos);
		SetPostion(CurPos);
		Display(m_CurValue);
	}

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

	virtual void SetTipText()
	{
		if (m_hTip)
		{
			TOOLINFO info = { 0 };
			info.cbSize = sizeof(TOOLINFO);
			info.uFlags = TTF_ABSOLUTE | TTF_TRACK | TTF_IDISHWND;
			info.hwnd = m_hBar;
			info.uId = (UINT_PTR)m_hBar;
			info.lpszText = m_szTmpTxt;
			SendMessage(m_hTip, TTM_UPDATETIPTEXT, 0, (LPARAM)&info);
		}
	}

	virtual int OnMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		switch (message)
		{
		case WM_HSCROLL:
			{
				LONG CurPos = 0;
				HWND hTrack = (HWND)(lParam);

				if ((LOWORD(wParam) == TB_THUMBTRACK) ||
					(LOWORD(wParam) == TB_THUMBPOSITION))
				{
					CurPos = (LONG)(HIWORD(wParam));
				}
				else
				{
					CurPos = (LONG)SendMessage(hTrack, TBM_GETPOS, 0, 0);
				}

				UpdateValue(CurPos);
				if (LOWORD(wParam) == TB_ENDTRACK)
				{
					if (m_pCb)
						m_pCb(m_hBar, m_pArg);
				}
			}

			return FALSE; //should return false for this message.
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
			default:
				break;
			}
		}
		return 0;
	}
};