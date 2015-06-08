#pragma once

#include "Win32.h"

namespace Jig
{
	class MemoryDC : public CDC
	{
	public:
		MemoryDC(CWnd& wnd);
		~MemoryDC();

		const CRect& GetRect() const { return m_rect; }

	private:
		CPaintDC m_paintDC;
		CBitmap m_bitmap;
		CBitmap* m_oldBitmap;
		CRect m_rect;
	};
}