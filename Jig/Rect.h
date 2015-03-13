#pragma once

#include "Types.h"

namespace Jig
{
	class Rect
	{
	public:
		Rect();
		Rect(const Vec2& p0, const Vec2& p1);
		~Rect();

		float Width() const { return m_p1.x - m_p0.x; }
		float Height() const { return m_p1.y - m_p0.y; }

		void Normalise();

		Vec2 m_p0, m_p1;
	};
}
