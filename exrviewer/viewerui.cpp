

#include "stdafx.h"
#include "viewimage.h"
#include "panel.h"


inline bool PtInRect(int x, int y, DispRect rect)
{
	return (x >= rect.x && x <= rect.x + rect.w &&
		y >= rect.y && y <= rect.y + rect.h);
}

void ImageViewer::UpdateScrollWnd()
{
	if (!m_Option.bActualImageSize)
	{
		m_Scroll.Show(SB_BOTH, false);
	}
	else
	{
		bool bHorz = (m_img.width > m_WndWidth);
		bool bVert = (m_img.height > m_WndHeight);

		m_Scroll.Show(SB_HORZ, bHorz);
		m_Scroll.Show(SB_VERT, bVert);

		RECT rc;
		::GetClientRect(m_hWnd, &rc);
		// update size with scroll bar show/unshow.
		//m_WndWidth = rc.right - rc.left;
		//m_WndHeight = rc.bottom - rc.top;
		if (bHorz)
		{
			m_Scroll.SetPage(SB_HORZ, rc.right - rc.left);
			m_Scroll.SetRange(SB_HORZ, 0, m_img.width);
		}

		if (bVert)
		{
			m_Scroll.SetPage(SB_VERT, rc.bottom - rc.top);
			m_Scroll.SetRange(SB_VERT, 0, m_img.height);
		}
	}
}

bool ImageViewer::UpdateDisplayRect()
{
	if (!m_bImageLoaded)
	{
		// must return, or it will update with error image size.
		return false;
	}
	bool updated = false;
	RECT rc;
	::GetClientRect(m_hWnd, &rc);
	int ww = rc.right - rc.left;
	int wh = rc.bottom - rc.top;

	if (ww != m_WndWidth)
	{
		m_WndWidth = ww;
		updated = true;
	}
	if (wh != m_WndHeight)
	{
		m_WndHeight = wh;
		updated = true;
	}

	UpdateScrollWnd();

	if (updated)
	{
		if (!m_Option.bActualImageSize)
		{
			float r_wnd = (float)m_WndWidth / (float)m_WndHeight;
			float r_img = (float)m_img.width / (float)m_img.height;
			if (r_wnd >= r_img)
			{
				m_StretchRect.h = m_WndHeight;
				m_StretchRect.w = (int)((float)m_WndHeight * r_img);
				m_StretchRect.x = (m_WndWidth - m_StretchRect.w) / 2;
				m_StretchRect.y = 0;
			}
			else
			{
				m_StretchRect.h = (int)((float)m_WndWidth / r_img);
				m_StretchRect.w = m_WndWidth;
				m_StretchRect.x = 0;
				m_StretchRect.y = (m_WndHeight - m_StretchRect.h) / 2;
			}
		}
		else
		{
			// scroll display
			m_ScrollRect.x = 0;
			m_ScrollRect.y = 0;
			m_ScrollRect.w = m_WndWidth;
			m_ScrollRect.h = m_WndHeight;

			if (!m_Scroll.IsHorzScroll())
			{
				m_ScrollRect.x = (m_WndWidth - m_img.width) / 2;
				m_ScrollRect.w = m_img.width;
			}
			if (!m_Scroll.IsVertScroll())
			{
				m_ScrollRect.y = (m_WndHeight - m_img.height) / 2;
				m_ScrollRect.h = m_img.height;
			}
			m_ScrollPosX = 0;
			m_ScrollPosY = 0;
		}
	}
	return updated;
}

void ImageViewer::MouseScroll(bool bLButton, int xpos, int ypos)
{
	if (!m_bImageLoaded)
	{
		ChangeCursor(CUR_ARROW);
		return;
	}
	static bool bMouseDragging = false;
	if (!bLButton)
	{
		if (bMouseDragging)
		{
			bMouseDragging = false;
		}
		if (( m_Option.bActualImageSize && PtInRect(xpos, ypos, m_ScrollRect))||
			(!m_Option.bActualImageSize && PtInRect(xpos, ypos, m_StretchRect)))
		{
			ChangeCursor(CUR_CROSS);
			ShowPixelInfo(xpos, ypos);
		}
		else
			ChangeCursor(CUR_ARROW);
	}
	else
	{
		static int xpos_last = 0;
		static int ypos_last = 0;
		if (!bMouseDragging)
		{
			bMouseDragging = true;
			ChangeCursor(CUR_HAND);
			xpos_last = xpos;
			ypos_last = ypos;
		}
		else /*if (bMouseMoving)*/
		{
			int delta_x = xpos_last - xpos;
			int delta_y = ypos_last - ypos;
			xpos_last = xpos;
			ypos_last = ypos;

			if (m_Scroll.IsHorzScroll())
			{
				m_ScrollPosX = m_Scroll.ClampXPosToRange(m_ScrollPosX + delta_x);
				m_Scroll.SetPos(SB_HORZ, m_ScrollPosX);
			}
			if (m_Scroll.IsVertScroll())
			{
				m_ScrollPosY = m_Scroll.ClampYPosToRange(m_ScrollPosY + delta_y);
				m_Scroll.SetPos(SB_VERT, m_ScrollPosY);
			}
			DrawImage();
		}
	}
}

void ImageViewer::Scroll(bool bHorz, int request, int pos)
{
	if (request == SB_THUMBTRACK || request == SB_THUMBPOSITION)
	{
		if (bHorz)
			m_ScrollPosX = pos;
		else
			m_ScrollPosY = pos;
		SetScrollPos(m_hWnd, bHorz ? SB_HORZ : SB_VERT, pos, TRUE);
	}
	else if (request == SB_PAGELEFT || request == SB_PAGERIGHT)
	{
		int sign = (request == SB_PAGELEFT) ? -1 : 1;
		int v = 0; // old pos
		if (bHorz)
		{
			v = m_Scroll.GetPos(SB_HORZ) + sign * m_Scroll.PageX();
			//v = IntClamp(v, m_Scroll.MinX(), m_Scroll.MaxX()- m_Scroll.PageX());
			v = m_Scroll.ClampXPosToRange(v);
			m_ScrollPosX = v;
		}
		else
		{
			v = m_Scroll.GetPos(SB_VERT) + sign * m_Scroll.PageY();
			//v = IntClamp(v, m_Scroll.MinY(), m_Scroll.MaxY() - m_Scroll.PageY());
			v = m_Scroll.ClampYPosToRange(v);
			m_ScrollPosY = v;
		}
		m_Scroll.SetPos(bHorz ? SB_HORZ : SB_VERT, v);
	}
	else if (request == SB_LINELEFT || request == SB_LINERIGHT)
	{
		const int scroll_step = 10;
		int sign = (request == SB_LINELEFT) ? -1 : 1;
		int v = 0; // new pos
		if (bHorz)
		{
			v = m_Scroll.GetPos(SB_HORZ) + sign * scroll_step;
			//v = IntClamp(v, m_Scroll.MinX(), m_Scroll.MaxX() - m_Scroll.PageX());
			v = m_Scroll.ClampXPosToRange(v);
			m_ScrollPosX = v;
		}
		else
		{
			v = m_Scroll.GetPos(SB_VERT) + sign * scroll_step;
			//v = IntClamp(v, m_Scroll.MinY(), m_Scroll.MaxY() - m_Scroll.PageY());
			v = m_Scroll.ClampYPosToRange(v);
			m_ScrollPosY = v;
		}
		m_Scroll.SetPos(bHorz ? SB_HORZ : SB_VERT, v);
	}

	DrawImage();
}

void ImageViewer::ShowPixelInfo(int xpos, int ypos)
{
	if (m_bImageLoaded)
	{
		int x, y;
		if (m_Option.bActualImageSize)
		{
			x = xpos + m_ScrollPosX - m_ScrollRect.x;
			y = ypos + m_ScrollPosY - m_ScrollRect.y;
			x = IntClamp(x, 0, m_img.width);
			y = IntClamp(y, 0, m_img.height);
		}
		else
		{
			int _x = xpos - m_StretchRect.x;
			int _y = ypos - m_StretchRect.y;
			_x = IntClamp(_x, 0, m_StretchRect.w);
			_y = IntClamp(_y, 0, m_StretchRect.h);
			x = _x * m_img.width / m_StretchRect.w;
			y = _y * m_img.height / m_StretchRect.h;
		}
		BYTE* rgb = (BYTE *)m_img.rgb + (y * m_img.width + x) * 3;
		int b = (int)(*rgb++);
		int g = (int)(*rgb++);
		int r = (int)(*rgb++);

		if (m_pCtlPanel)
			m_pCtlPanel->SetCoordAndColorInfo(x, y, r, g, b);
	}
}

void ImageViewer::RemoveEXRPartIndexMenu()
{
	const int IDM_PARTIDX_0 = 64000;
	if (m_img.PartNum >= 1)
	{
		HMENU hImgMenu = m_pMenuMan->GetSubMenuByName(L"&Image");
		for (int i = 0; i < m_img.PartNum; ++i)
		{
			m_pMenuMan->RemoveItem(hImgMenu, IDM_PARTIDX_0 + i);
		}
		m_pMenuMan->RemoveLastItem(hImgMenu); // remove seperator
	}
}

void ImageViewer::AppendEXRPartIndexMenu()
{
	const int IDM_PARTIDX_0 = 64000;

	if (m_img.PartNum >= 1)
	{
		HMENU hImgMenu = m_pMenuMan->GetSubMenuByName(L"&Image");
		m_pMenuMan->AppendSeparator(hImgMenu); // append separator
		WCHAR ch[20] = {};
		for (int i = 0; i < m_img.PartNum; ++i)
		{
			swprintf_s(ch, 20, L"Part Index %d", i);
			m_pMenuMan->AppendItem(hImgMenu, IDM_PARTIDX_0 + i, ch);
		}
	}
}
