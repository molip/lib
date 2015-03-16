#pragma once

#include "Types.h"
#include <cassert>

namespace Jig
{

class Line2
{
public:
	Line2();
		
	static Line2 MakeFinite(const Vec2& p0, const Vec2& p1);
	static Line2 MakeInfinite(const Vec2& p0, const Vec2& p1);
	static Line2 MakeVertical(double x);
	static Line2 MakeHorizontal(double y);

	const Line2& operator=(const Line2& rhs);

	void SetFinite(bool val);
	bool IsFinite() const { return m_finite; }

	void SwapPoints();

	void SetP0(const Vec2& p0);
	void SetP1(const Vec2& p1);

	const Vec2& GetP0() const;
	const Vec2& GetP1() const;

	double Length() const;
	bool Intersect(const Line2& that, Vec2* pPoint = nullptr) const;
	Line2 GetPerpBisector() const;
	Line2 GetPerpThroughtPoint(const Vec2& point) const;
	double PerpDistanceTo(const Vec2& point, bool* intersects) const;

	bool IsVertical() const { Validate();  return m_vert; }
	bool IsHorizontal() const;

private:
	Line2(const Vec2& p0, const Vec2& p1, bool finite);
	Line2(const Vec2& p0, double m);
	void Validate() const;
	bool IsPointWithinFiniteRange(const Vec2& point) const;

	Vec2 m_p0, m_p1;
	bool m_finite;
	mutable double m_m, m_c;
	mutable bool m_vert, m_valid;
};

}
