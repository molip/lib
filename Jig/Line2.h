#pragma once

#include "Types.h"
#include <cassert>

namespace Jig
{

class Line2
{
public:
	Line2();
	Line2(const Vec2& p0, const Vec2& p1);
		
	static Line2 MakeVertical(double x);
	static Line2 MakeHorizontal(double y);

	const Line2& operator=(const Line2& rhs);

	void SetP0(const Vec2& p0);
	void SetP1(const Vec2& p1);
	
	double Length() const; // finite
	bool Intersect(const Line2& that, Vec2& point) const; // infinite
	Line2 GetPerpBisector() const; // finite
	double DistanceTo(const Vec2& point, bool finite) const;

	bool IsVertical() const { return m_vert; }
	bool IsHorizontal() const;
private:
	Line2(const Vec2& p0, double m);
	void Validate() const;

	Vec2 m_p0, m_p1;
	mutable double m_m, m_c;
	mutable bool m_vert, m_valid;
};

}
