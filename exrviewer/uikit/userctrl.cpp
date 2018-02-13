
#include "stdafx.h"
#include "userctrl.h"
#include "userwnd.h"

void SetMenuItemState(HMENU hMenu, DWORD ItemId, bool bCheck)
{
	if (hMenu)
	{
		if (bCheck)
		{
			CheckMenuItem(hMenu, ItemId, MF_BYCOMMAND | MF_CHECKED);
		}
		else
		{
			CheckMenuItem(hMenu, ItemId, MF_BYCOMMAND | MF_UNCHECKED);
		}
	}
}

void SetMainMenuItemState(HWND hWnd, DWORD ItemId, bool bCheck)
{
	SetMenuItemState(GetMenu(hWnd), ItemId, bCheck);
}

int SliderSet::OnMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
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

void SliderSet::CreateSlider(int w/* = 0*/, LPWSTR caption/* = nullptr*/, int label_width/* = 0*/,
	HWND hParent/* = nullptr*/, HINSTANCE inst/* = nullptr*/, 
	bool bCheckbox/* = false*/, bool bVertical/* = false*/, bool ticks/* = false*/)
{
	w_label = caption ? label_width : 0;
	w_txt = cw_txt_default;
	w_chk = bCheckbox ? CHKBOX_WIDTH : 0;
	w_bar = w - w_label - w_txt - w_chk - 2 /*space*/ - 4 /*space between bar and edit*/ - 2 /*space*/;

	int y_label = (ch_bar - ch_label) / 2 + 2;
	if (caption)
	{
		m_pLabel = new UserCtrl(L"STATIC", caption, 0, y_label, w_label, ch_label, hParent, inst, false, WS_CHILD | WS_VISIBLE | SS_LEFT);
		SendMessage(m_hBar, TBM_SETBUDDY, (WPARAM)TRUE, (LPARAM)m_pLabel->GetHWND());
	}

	m_pBar = new UserCtrl(TRACKBAR_CLASS, nullptr, w_label + 2, 0, w_bar, ch_bar, hParent, inst, false,
		WS_CHILD | WS_VISIBLE | (bVertical ? TBS_VERT : TBS_HORZ) | (ticks ? (TBS_BOTH) : TBS_BOTH | TBS_NOTICKS) /* | TBS_AUTOTICKS | TBS_ENABLESELRANGE*/);
	m_hBar = m_pBar->GetHWND();

	//int y_txt = (ch_bar - ch_txt) / 2;
	m_pTxt = new UserCtrl(L"EDIT", L"", 0, 0, w_txt, ch_txt, hParent, inst, false, /*WS_BORDER |*/ WS_CHILD | WS_VISIBLE | ES_RIGHT | ES_READONLY);
	SendMessage(m_hBar, TBM_SETBUDDY, (WPARAM)FALSE, (LPARAM)m_pTxt->GetHWND());
	m_hTxt = m_pTxt->GetHWND();

	if (bCheckbox)
	{
		int x_chk = w - w_chk - 2;
		int y_chk = (ch_bar - CHKBOX_HEIGHT) / 2 + 2;
		m_pChk = new Checkbox(x_chk, y_chk, nullptr, 0, hParent, inst);
		m_hChk = m_pChk->GetHWND();
	}
	m_hTip = (HWND)SendMessage(m_hBar, TBM_GETTOOLTIPS, 0, 0);
}

void SliderSet::Move(int x /*= 0*/, int y/* = 0*/, int w/* = 0*/, bool bRepaint/* = false*/)
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

void SliderSet::InitValue(float minValue, float maxValue, float stepping/* = 1.f*/, float curValue/* = 0.f*/)
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

void SliderSet::SetTipText()
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