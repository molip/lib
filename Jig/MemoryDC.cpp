#include "MemoryDC.h"

using namespace Jig;
	
MemoryDC::MemoryDC(CWnd& wnd) : m_paintDC(&wnd)
{
	CreateCompatibleDC(&m_paintDC);

	m_paintDC.GetClipBox(m_rect);

	m_bitmap.CreateCompatibleBitmap(&m_paintDC, m_rect.Width(), m_rect.Height());
	m_oldBitmap = SelectObject(&m_bitmap);
	
	SetViewportOrg(-m_rect.TopLeft());
}

MemoryDC::~MemoryDC()
{
	m_paintDC.BitBlt(m_rect.left, m_rect.top, m_rect.Width(), m_rect.Height(), this, m_rect.left, m_rect.top, SRCCOPY);
	SelectObject(m_oldBitmap);
}
