#pragma once

#include "Rect.h"
#include "Win32.h"

#include <cmath>

namespace Jig
{
	CRect Convert(const Rect& r)
	{
		return CRect((long)std::floor(r.m_p0.x), (long)std::floor(r.m_p0.y), (long)std::floor(r.m_p1.x), (long)std::floor(r.m_p1.y));
	}

	CPoint Convert(const Vec2& p)
	{
		return CPoint((long)std::floor(p.x), (long)std::floor(p.y));
	}

	Vec2 Convert(const CPoint& p)
	{
		return Vec2(p.x, p.y);
	}
}