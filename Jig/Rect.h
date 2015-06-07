#pragma once

#include "Types.h"

namespace Jig
{
	class Rect
	{
	public:
		Rect();
		Rect(const Vec2& p);
		Rect(const Vec2& p0, const Vec2& p1);
		~Rect();

		double Width() const { return m_p1.x - m_p0.x; }
		double Height() const { return m_p1.y - m_p0.y; }
		Vec2 GetCentre() const { return Vec2(m_p0.x + (m_p1.x - m_p0.x) / 2, m_p0.y + (m_p1.y - m_p0.y) / 2); }
		bool Contains(const Vec2& point) const;
		bool IsEmpty() const { return m_p0 == m_p1; }

		void Normalise();
		void GrowTo(const Vec2& p);
		void GrowTo(const Rect& r);
		void Inflate(double x, double y);

		Vec2 m_p0, m_p1;
	};

	class RectGrower
	{
	public:
		RectGrower() : m_isNull(true) {}

		bool IsNull() const { return m_isNull; }
		const Rect& GetRect() const { return m_rect; }

		void Add(const Vec2& p);
		void Add(const Rect& r);
	
	private:
		Rect m_rect;
		bool m_isNull;
	};
}
